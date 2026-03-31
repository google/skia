* `SkImages::WrapTexture` no longer requires providing an `SkColorType`. A closest compatible
   SkColorType will be chosen as long as the texture's format is supported and the texture is
   sampleable. Additionally, wrapped textures can now be forced to opaque by specifying
   `kUnknown_SkAlphaType`. Single-channel texture formats map to either alpha-only or red color
   types based on the provided `SkAlphaType`.
