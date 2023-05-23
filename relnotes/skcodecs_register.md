Clients now need to register codecs which Skia should use to decode raw bytes. For example:
`SkCodecs::Register(SkJpegDecoder::Decoder());`. Skia still provides many supported formats
(see `include/codec/*Decoder.h`). Clients are free to specify their own, either supplementing
the existing set or using a custom version instead of the one previously provided by default
by Skia. See `SkCodecs::Decoder` for the necessary data to provide when using a custom decoder
(in `include/codec/SkCodec.h`).

To ease the transition, Skia will continue (for a short while) to register codecs unless
`SK_DISABLE_LEGACY_INIT_DECODERS` is defined.