When `SkCodec::SelectionPolicy::kPreferStillImage` is passed to `SkWuffsCodec`/`SkGifDecoder`
creation, and the input stream cannot be rewound, the resulting `SkWuffsCodec` will no longer copy
the stream. Because it will now have a non-seekable stream, it no longer supports `getFrameCount`,
which will now simply report `1`, or `getFrameInfo`, which is useful only for animation anyway.
Chromium uses `kPreferStillImage`, simply because it is the default, but will not be affected by
this change because it always supplies a seekable stream.
