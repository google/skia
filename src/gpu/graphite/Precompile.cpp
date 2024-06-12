/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix({ sk_ref_sp(this) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({ sk_ref_sp(this) }, { std::move(cf) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> cs) {
    if (!cs) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpace({ sk_ref_sp(this) }, { std::move(cs) });
}

sk_sp<PrecompileColorFilter> PrecompileColorFilter::makeComposed(
        sk_sp<PrecompileColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return PrecompileColorFilters::Compose({ sk_ref_sp(this) }, { std::move(inner) });
}

} // namespace skgpu::graphite
