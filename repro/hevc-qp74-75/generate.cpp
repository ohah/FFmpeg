// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2026 ohah
// Standalone x265 reconstruction oracle for the HEVC QP 74/75 regression.
#include <x265.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    if (argc != 3) return 1;
    const x265_api *api = x265_api_get(12);
    if (!api) return 2;
    x265_param *param = api->param_alloc();
    api->param_default_preset(param, "medium", nullptr);
    param->sourceWidth = 128;
    param->sourceHeight = 64;
    param->fpsNum = 25;
    param->fpsDenom = 1;
    param->internalCsp = X265_CSP_I444;
    param->internalBitDepth = 12;
    param->totalFrames = 1;
    const char *keys[] = {"keyint", "qp", "aq-mode", "pools", "frame-threads", "wpp", "log-level"};
    const char *vals[] = {"1", argv[2], "0", "1", "1", "0", "error"};
    for (unsigned i = 0; i < sizeof(keys) / sizeof(*keys); i++)
        if (api->param_parse(param, keys[i], vals[i])) return 3;
    x265_encoder *encoder = api->encoder_open(param);
    if (!encoder) return 4;
    x265_picture *input = api->picture_alloc();
    x265_picture *output = api->picture_alloc();
    api->picture_init(param, input);
    api->picture_init(param, output);
    input->bitDepth = 12;
    input->colorSpace = X265_CSP_I444;
    std::vector<uint16_t> pixels(128 * 64 * 3);
    uint32_t seed = 8243;
    for (auto &pixel : pixels) {
        seed = seed * 1664525U + 1013904223U;
        pixel = (seed >> 16) & 4095;
    }
    for (int c = 0; c < 3; c++) {
        input->planes[c] = pixels.data() + c * 128 * 64;
        input->stride[c] = 128 * sizeof(uint16_t);
    }
    std::string prefix = argv[1];
    FILE *bitstream = fopen((prefix + ".hevc").c_str(), "wb");
    if (!bitstream) return 5;
    x265_nal *nals;
    uint32_t count;
    api->encoder_headers(encoder, &nals, &count);
    for (unsigned i = 0; i < count; i++)
        fwrite(nals[i].payload, 1, nals[i].sizeBytes, bitstream);
    int result = api->encoder_encode(encoder, &nals, &count, input, output);
    for (unsigned attempt = 0; attempt < 100; attempt++) {
        if (result < 0) return 6;
        for (unsigned i = 0; i < count; i++)
            fwrite(nals[i].payload, 1, nals[i].sizeBytes, bitstream);
        if (result > 0) {
            FILE *recon = fopen((prefix + ".yuv").c_str(), "wb");
            if (!recon) return 7;
            for (int c = 0; c < 3; c++)
                for (int y = 0; y < 64; y++)
                    fwrite(static_cast<char *>(output->planes[c]) + y * output->stride[c], 2, 128, recon);
            fclose(recon);
            break;
        }
        result = api->encoder_encode(encoder, &nals, &count, nullptr, output);
    }
    fclose(bitstream);
    api->encoder_close(encoder);
    api->picture_free(input);
    api->picture_free(output);
    api->param_free(param);
    return result > 0 ? 0 : 8;
}
