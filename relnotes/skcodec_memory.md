`SkCodec::Options` now contains `fMaxDecodeMemory`. If Skia detects or estimates it would use more
than that amount of memory (in aggregate) for decoding the image, it will return nullptr instead
of attempting to decode it. Failures in this way will result in returning the new
`SkCodec::Result::kOutOfMemory`.