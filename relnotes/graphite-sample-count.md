Backend specific texture infos, e.g. `DawnTextureInfo`,
`VulkanTextureInfo`, and `MtlTextureInfo`'s `fSampleCount` field, and the
`ContextOptions::fInternalMultisampleCount` field are now `SampleCount`. A helper
function, `ToSampleCount(uint32_t) -> SampleCount` is provided if needing to convert a variable value vs. just updating a constant.
