/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PrecompileEffectFactories_DEFINED
#define PrecompileEffectFactories_DEFINED

#include "include/core/SkRefCnt.h"

class SkBlender;
class SkColorFilter;
class SkRuntimeEffect;
class SkShader;

namespace skgpu::graphite {
    class PrecompileBlender;
    class PrecompileColorFilter;
    class PrecompileShader;
}

namespace skiatest::graphite {

// These factory functions returns a normal-API/precompile-API pair that represent the same
// effect.
namespace PrecompileFactories {
    using BlenderPair = std::pair<sk_sp<SkBlender>,
                                  sk_sp<skgpu::graphite::PrecompileBlender>>;
    using ColorFilterPair = std::pair<sk_sp<SkColorFilter>,
                                      sk_sp<skgpu::graphite::PrecompileColorFilter>>;
    using ShaderPair = std::pair<sk_sp<SkShader>,
                                 sk_sp<skgpu::graphite::PrecompileShader>>;

    const char* GetAnnulusShaderCode();
    SkRuntimeEffect* GetAnnulusShaderEffect();
    ShaderPair CreateAnnulusRuntimeShader();

    SkRuntimeEffect* GetSrcBlenderEffect();
    BlenderPair CreateSrcRuntimeBlender();
    SkRuntimeEffect* GetDstBlenderEffect();
    BlenderPair CreateDstRuntimeBlender();
    SkRuntimeEffect* GetComboBlenderEffect();
    BlenderPair CreateComboRuntimeBlender();

    SkRuntimeEffect* GetDoubleColorFilterEffect();
    ColorFilterPair CreateDoubleRuntimeColorFilter();
    SkRuntimeEffect* GetHalfColorFilterEffect();
    ColorFilterPair CreateHalfRuntimeColorFilter();
    SkRuntimeEffect* GetComboColorFilterEffect();
    ColorFilterPair CreateComboRuntimeColorFilter();

} // namespace PrecompileFactories

} // namespace skiatest::graphite

#endif // PrecompileEffectFactories_DEFINED
