Define a new public enum, `SampleCount`, that enforces the valid sample count values that Graphite
supports (1, 2, 4, 8, 16). `TextureInfo::numSamples() -> uint8_t` is replaced with
`TextureInfo::sampleCount() -> SampleCount`.

Backend specific texture infos, e.g. `DawnTextureInfo`,
`VulkanTextureInfo`, and `MtlTextureInfo` still represent sample count as a `uint8_t` for
convience with the backend APIs. This `uint8_t` value is validated when wrapping the backend info
into a `TextureInfo`; if it's not a `SampleCount` value, then an empty `TextureInfo` is returned.
