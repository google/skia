`SkRuntimeEffectBuilder::uniforms()`, `SkRuntimeEffectBuilder::children()`,
`SkRuntimeShaderBuilder::makeShader()`, `SkRuntimeColorFilterBuilder::makeColorFilter()`, and
`SkRuntimeBlendBuilder::makeBlender()` are now marked as const. No functional changes internally,
just making explicit what had been implicit.