/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileRuntimeEffect_DEFINED
#define skgpu_graphite_precompile_PrecompileRuntimeEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

class SkRuntimeEffect;

namespace skgpu::graphite {

class PrecompileBase;
class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileShader;

using PrecompileChildOptions = SkSpan<const sk_sp<PrecompileBase>>;

//--------------------------------------------------------------------------------------------------
// These factory functions correspond to the main API's SkRuntimeEffect makeShader, makeColorFilter,
// and makeBlender functions. The uniform values found in the main API are abstracted away since
// they do not alter the generated code.
namespace PrecompileRuntimeEffects {

SK_API sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

SK_API sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

SK_API sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

} // namespace PrecompileRuntimeEffects

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileRuntimeEffect_DEFINED
