# HEVC inverse quantization QP 74/75 regression inputs

These synthetic 128x64, 12-bit 4:4:4, single-frame streams were generated
with x265 4.2 from an LCG seed (8243). They contain no real photos or user data.
The `.yuv` files are the encoder's reconstructed output, copied row by row
from `x265_picture` returned by `encoder_encode`, not from an FFmpeg decode.
All files here are provided under LGPL-2.1-or-later, matching this repository.

With a local x265 build supporting 12-bit:

```sh
c++ -std=c++17 generate.cpp $(pkg-config --cflags --libs x265) -o generate
./generate qp74 47
./generate qp75 48
```

The command-line QP parameter is the encoder's rate-control setting, not
the inverse quantization table index. x265's intra and 4:4:4 chroma offsets
produce table indices 74 and 75 respectively. Temporary instrumentation
immediately before `scale = level_scale[rem6[qp]] << div6[qp]` confirmed:

- qp74: 8 luma transform blocks at QP 68; 16 chroma blocks at QP 74.
- qp75: 8 luma transform blocks at QP 69; 16 chroma blocks at QP 75.

That instrumentation is absent from the proposed patch.

```sh
ffmpeg -v error -i qp74.hevc -pix_fmt yuv444p12le -f rawvideo actual74.yuv
ffmpeg -v error -i qp75.hevc -pix_fmt yuv444p12le -f rawvideo actual75.yuv
cmp actual74.yuv qp74.yuv
cmp actual75.yuv qp75.yuv
```

Unmodified FFmpeg master 46f23be mismatches 16,377 and 16,379 samples,
respectively, out of 24,576 uint16 samples per image. With the two missing
entries appended to each table, both outputs match the encoder reconstruction
exactly. `results.json` records byte hashes and comparisons. The compressed
samples are 7,217 and 6,781 bytes. They are intended for the external FATE
sample directory `hevc/12bit-444-qp74.hevc` and `hevc/12bit-444-qp75.hevc`.
