Added `SkCodecs::DeferredImage` which is similar to `SkImages::DeferredFromEncodedData` except it
allows the caller to pass in a `SkCodec` directly instead of depending on compiled-in codecs.