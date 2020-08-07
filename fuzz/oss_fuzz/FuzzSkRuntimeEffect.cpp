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

#include "fuzz/Fuzz.h"

static constexpr size_t kReservedBytes = 256;
/**
 * The fuzzer will take in the bytes and divide into two parts.
 * original bytes : [... code bytes ... | 256 bytes]
 * The first part is codeBytes, the original bytes minus 256 bytes, which will be treated
 * as sksl code, intending to create SkRuntimeEffect.
 * For the second part, it will first reserve 256 bytes and then allocate bytes with same size
 * as effect->inputSize() to uniformBytes. The uniformBytes is intended to create makeShader().
 * Note that if uniformBytes->size() != effect->inputSize() the shader won't be created.
 */
bool FuzzSkRuntimeEffect(sk_sp<SkData> bytes) {
    if (bytes->size() < kReservedBytes) {
        return false;
    }
    sk_sp<SkData> codeBytes = SkData::MakeSubset(bytes.get(), 0, bytes->size() - kReservedBytes);

    SkRuntimeEffect::EffectResult tuple = SkRuntimeEffect::Make(
        SkString((const char*) codeBytes->data(), codeBytes->size())
    );
    SkRuntimeEffect* effect = std::get<0>(tuple).get();

    if (!effect || effect->inputSize() > kReservedBytes) { // if there is not enough uniform bytes
        return false;
    }
    sk_sp<SkData> uniformBytes = SkData::MakeSubset(bytes.get(), bytes->size() - kReservedBytes,
                                                                effect->inputSize());
    auto shader = effect->makeShader(uniformBytes, nullptr, 0, nullptr, false);
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
    FuzzSkRuntimeEffect(bytes);
    return 0;
}
#endif
