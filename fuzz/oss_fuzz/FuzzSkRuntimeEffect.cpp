/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"

#include "fuzz/Fuzz.h"

bool FuzzSkRuntimeEffect(sk_sp<SkData> bytes) {
    sk_sp<SkData> numPtr = SkData::MakeSubset(bytes.get(), 0, 1);
    uint8_t num = *(uint8_t*) numPtr->data();
    sk_sp<SkData> codeBytes = SkData::MakeSubset(bytes.get(), num, bytes->size() - num);

    SkRuntimeEffect::EffectResult tuple = SkRuntimeEffect::Make(
        SkString((const char*) codeBytes->data(), codeBytes->size())
    );
    SkRuntimeEffect* effect = std::get<0>(tuple).get();
    if (!effect || effect->inputSize() > num) { // if there is not enough uniform bytes to be passed in
        return false;
    }

    sk_sp<SkData> uniformBytes = SkData::MakeSubset(bytes.get(), 0, effect->inputSize());
    SkMatrix localM;
    localM.setRotate(90, 128, 128);
    auto shader = effect->makeShader(codeBytes, nullptr, 0, &localM, true);
    if (!shader) {
        return false;
    }
    SkPaint paint;
    paint.setShader(std::move(shader));

    sk_sp<SkSurface> s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        return false;
    }

    s->getCanvas()->drawPaint(paint);

    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Pipeline(bytes);
    return 0;
}
#endif
