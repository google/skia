/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileShadersPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileShadersPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

class PrecompileShader;

//--------------------------------------------------------------------------------------------------
namespace PrecompileShadersPriv {
    // -- The first 6 factories are used to implement ImageFilters
    sk_sp<PrecompileShader> Blur(sk_sp<PrecompileShader> wrapped);

    sk_sp<PrecompileShader> Displacement(sk_sp<PrecompileShader> displacement,
                                         sk_sp<PrecompileShader> color);

    sk_sp<PrecompileShader> Lighting(sk_sp<PrecompileShader> wrapped);

    sk_sp<PrecompileShader> MatrixConvolution(sk_sp<PrecompileShader> wrapped);

    sk_sp<PrecompileShader> LinearMorphology(sk_sp<PrecompileShader> wrapped);

    sk_sp<PrecompileShader> SparseMorphology(sk_sp<PrecompileShader> wrapped);

    // TODO: This, technically, doesn't need to take an SkSpan since it is only called from
    // PaintOptions::setClipShaders with a single PrecompileShader. Leaving it be for now in case
    // the usage is revised.
    sk_sp<PrecompileShader> CTM(SkSpan<const sk_sp<PrecompileShader>> wrapped);

    // This factory variant should be used when the existence or non-existence of the local matrix
    // is known. If 'withLM' is true only the LMShader-wrapped shader will be created while, when
    // 'withLM' is false, no LMShader will wrap the base shader.
    sk_sp<PrecompileShader> Picture(bool withLM);

    // TODO: this factory function should go away (it is only used by the PrecompileShaders::Picture
    // entry point now).
    sk_sp<PrecompileShader> LocalMatrixBothVariants(SkSpan<const sk_sp<PrecompileShader>> wrapped);

} // namespace PrecompileShadersPriv

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileShadersPriv_DEFINED
