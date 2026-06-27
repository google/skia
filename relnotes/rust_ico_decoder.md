Added a new `SkIcoRustDecoder` which decodes ICO and CUR images using Rust-based
PNG and BMP decoders for the embedded images. Register `SkIcoRustDecoder::Decoder()`
with `SkCodecs::Register` to enable it. This is built when `skia_use_rust_ico_decode`
is enabled (defining `SK_CODEC_DECODES_ICO_WITH_RUST`).
