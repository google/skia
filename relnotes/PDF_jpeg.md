The PDF code now directly depends on Skia's JPEG decoder and encoder. The build
time shims to avoid using a JPEG decoder and encoder have been removed. In the
future these may be made optional again by allowing the user to supply them at
runtime.
