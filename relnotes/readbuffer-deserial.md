The behavior for SkPicture deserialization (via SkReadBuffer) to fallback to
`SkImages::DeferredFromEncodedData` when `SkDeserialImageProc` is not set or returns null is
deprecated and will be removed shortly.

`SkDeserialImageFromDataProc` has been added to SkDeserialProcs to allow clients to *safely*
avoid a copy when decoding image data in SkPictures.

`SkDeserialImageProc` now takes in an optional AlphaType which can be used to override the
AlphaType that an image was serialized with, if desired.
