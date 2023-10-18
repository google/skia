/*
 * Copyright 2023 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrShaderCaps.h"

using namespace skia_private;

/**
 * The fuzzer treats the input bytes as an SkSL color-filter program. The requested number of
 * uniforms and children are automatically synthesized to match the program's needs.
 *
 * We fuzz twice, with two different settings for inlining in the SkSL compiler. By default, the
 * compiler inlines most small to medium functions. This can hide bugs related to function-calling.
 * So we run the fuzzer once with inlining disabled, and again with it enabled.
 * This gives us better coverage, and eases the burden on the fuzzer to inject useless noise into
 * functions to suppress inlining.
 */
static bool FuzzSkRuntimeColorFilter_Once(const SkString& shaderText,
                                          const SkRuntimeEffect::Options& options) {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForColorFilter(shaderText, options);
    SkRuntimeEffect* effect = result.effect.get();
    if (!effect) {
        return false;
    }

    sk_sp<SkData> uniformBytes;
    TArray<SkRuntimeEffect::ChildPtr> children;
    FuzzCreateValidInputsForRuntimeEffect(effect, uniformBytes, children);

    sk_sp<SkColorFilter> cf = effect->makeColorFilter(uniformBytes, SkSpan(children));
    if (!cf) {
        return false;
    }
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setColorFilter(std::move(cf));

    sk_sp<SkSurface> s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(4, 4));
    if (!s) {
        return false;
    }
    s->getCanvas()->drawPaint(paint);

    return true;
}

bool FuzzSkRuntimeColorFilter(const uint8_t *data, size_t size) {
    // Test once with optimization disabled...
    SkString shaderText{reinterpret_cast<const char*>(data), size};
    SkRuntimeEffect::Options options;
    options.forceUnoptimized = true;
    bool result = FuzzSkRuntimeColorFilter_Once(shaderText, options);

    // ... and then with optimization enabled.
    options.forceUnoptimized = false;
    result = FuzzSkRuntimeColorFilter_Once(shaderText, options) || result;

    return result;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    FuzzSkRuntimeColorFilter(data, size);
    return 0;
}
#endif
