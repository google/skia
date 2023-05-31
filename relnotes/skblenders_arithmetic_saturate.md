The intermediate color computed by `SkBlenders::Arithmetic` is now always clamped to between 0 and 1 (inclusive), and then `enforcePremul` is applied when that parameter is true.
