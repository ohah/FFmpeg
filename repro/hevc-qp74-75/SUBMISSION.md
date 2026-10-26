# Proposed upstream submission

Title: avcodec/hevc: initialize inverse quantization tables through QP 75

The inverse-quantization `rem6` and `div6` arrays have 76 slots but
initialize only 74 of them. The two trailing implicit zeros produce
incorrect scaling for valid 12-bit HEVC QP indices 74 and 75. The first
patch appends the missing table values; the second adds two FATE tests.
No instrumentation, generated media or generator is added to the upstream
source diff. The samples belong in the external FATE sample suite.

Base: FFmpeg master `46f23be8b1976ed4c8ec85584daabcf0e57a7b9e`.
Patch branch: `ohah/FFmpeg:fix/hevc-12bit-qp-dequant`.
The two `git format-patch` files in `patches/` include Signed-off-by lines.

## Independent oracle

The sample generator obtains reconstructed pictures from x265 4.2's C API.
The expected outputs do not come from the decoder being fixed. Both sample
outputs match that reconstruction byte-for-byte after the fix. Temporary
QP tracing confirmed that the two inputs exercise indices 74 and 75,
respectively, and that tracing was removed before committing the fix.

| Test | Samples | Mismatch before | Mismatch after |
| --- | ---: | ---: | ---: |
| QP 74 | 24,576 | 16,377 | 0 |
| QP 75 | 24,576 | 16,379 | 0 |

## Validation

Build configuration (macOS arm64, Apple clang):

```sh
./configure --disable-everything --disable-autodetect --disable-doc \
  --disable-network --disable-programs --enable-ffmpeg \
  --enable-decoder=hevc --enable-parser=hevc \
  --enable-encoder=rawvideo,pcm_s16le --enable-demuxer=hevc \
  --enable-muxer=rawvideo,framecrc,framemd5 \
  --enable-protocol=file,pipe --enable-filter=null,format,scale \
  --enable-swscale
make -j8
```

Copy `qp74.hevc` and `qp75.hevc` to an external samples directory as
`hevc/12bit-444-qp74.hevc` and `hevc/12bit-444-qp75.hevc`.

```sh
make fate-hevc-12bit-qp74 fate-hevc-12bit-qp75 SAMPLES=/path/to/samples
make fate-hevc-12bit-qp74 fate-hevc-12bit-qp75 SAMPLES=/path/to/samples THREADS=4
make -j8 fate
```

- Both new tests fail on unmodified master with checksum differences.
- Both new tests pass after the fix, with default and four-thread decoding.
- `make -j8 fate` without SAMPLES passes 316 available tests in this minimal
  configuration. This does not represent a run of the entire external FATE
  sample suite or every enabled codec.
- `git diff --check` passes.
- `tools/patcheck` was run with GNU grep/findutils. Its non-static-symbol
  warnings are false positives on Makefile test commands, not C symbols.
  It also reports a missing changelog entry, which its message says to ignore
  for minor changes. No Changelog entry is added for this small decoder fix.

## Official submission route

Upstream `CONTRIBUTING.md` directs contributions to
https://code.ffmpeg.org/FFmpeg/FFmpeg/pulls or ffmpeg-devel, and says GitHub
pull requests are ignored. No GitHub PR was opened on the mirror.

Submitted as ohah to the official Forgejo:
https://code.ffmpeg.org/FFmpeg/FFmpeg/pulls/25467

The initial automated browser received Anubis Access Denied. Submission was
completed through the user's signed-in Aside Browser using `aside repl`.
The `git format-patch` files were applied with Forgejo's patch editor as two
commits (`97ff87d7d8` and `50972f5e67`). Commit metadata differs from the
GitHub branch, while the source changes remain the same. Both HEVC samples
are attached directly to the upstream PR with matching FATE filenames.
Upstream workflows require maintainer approval for this first contribution;
upstream review and full CI are pending.
