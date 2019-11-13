LIBGIF CODEC FOR SKIA
=====================

libgifcodec is based on a fork of libgif made by Chromium.  It was copied into
Skia with <https://codereview.chromium.org/2045293002>, as
<https://skia.googlesource.com/skia/+/19b91531e912283d237435d94516575b28713cba>.

The header file `SkGifCodec.h` exposes two functions:

  * `bool SkGifCodec::IsGif(const void*, size_t);`

  * `std::unique_ptr<SkCodec> SkGifCodec::MakeFromStream(std::unique_ptr<SkStream>, SkCodec::Result*);`

Which can be used by Skia's `SkCodec::MakeFromStream` to implement GIF Decoding.

See [`LICENSE`](LICENSE) for the license for `SkGifImageReader.cpp` and
`SkGifImageReader.h`.

See [`LICENSE_BSD_3_CLAUSE.md`](LICENSE_BSD_3_CLAUSE.md) for the license for
`SkLibGifCodec.h`, `BUILD.gn`, and `SkGifCodec.h`

