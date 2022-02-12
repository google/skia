/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/GrShaderCaps.h"

#include "fuzz/Fuzz.h"

/**
 * The fuzzer treats the input bytes as an SkSL program. The requested number of uniforms and
 * children are automatically synthesized to match the program's needs.
 *
 * We fuzz twice, with two different settings for inlining in the SkSL compiler. By default, the
 * compiler inlines most small to medium functions. This can hide bugs related to function-calling.
 * So we run the fuzzer once with inlining disabled, and again with it enabled.
 * This gives us better coverage, and eases the burden on the fuzzer to inject useless noise into
 * functions to suppress inlining.
 */
static bool FuzzSkRuntimeEffect_Once(sk_sp<SkData> codeBytes,
                                     const SkRuntimeEffect::Options& options) {
    SkString shaderText{static_cast<const char*>(codeBytes->data()), codeBytes->size()};
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(shaderText, options);
    SkRuntimeEffect* effect = result.effect.get();
    if (!effect) {
        return false;
    }

    // Create storage for our uniforms.
    sk_sp<SkData> uniformBytes = SkData::MakeZeroInitialized(effect->uniformSize());
    void* uniformData = uniformBytes->writable_data();

    for (const SkRuntimeEffect::Uniform& u : effect->uniforms()) {
        // We treat scalars, vectors, matrices and arrays the same. We just figure out how many
        // uniform slots need to be filled, and write consecutive numbers into those slots.
        static_assert(sizeof(int) == 4 && sizeof(float) == 4);
        size_t numFields = u.sizeInBytes() / 4;

        if (u.type == SkRuntimeEffect::Uniform::Type::kInt ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt2 ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt3 ||
            u.type == SkRuntimeEffect::Uniform::Type::kInt4) {
            int intVal = 0;
            while (numFields--) {
                // Assign increasing integer values to each slot (0, 1, 2, ...).
                *static_cast<int*>(uniformData) = intVal++;
                uniformData = static_cast<int*>(uniformData) + 1;
            }
        } else {
            float floatVal = 0.0f;
            while (numFields--) {
                // Assign increasing float values to each slot (0.0, 1.0, 2.0, ...).
                *static_cast<float*>(uniformData) = floatVal++;
                uniformData = static_cast<float*>(uniformData) + 1;
            }
        }
    }

    // Create valid children for any requested child effects.
    std::vector<SkRuntimeEffect::ChildPtr> children;
    children.reserve(effect->children().size());
    for (const SkRuntimeEffect::Child& c : effect->children()) {
        switch (c.type) {
            case SkRuntimeEffect::ChildType::kShader:
                children.push_back(SkShaders::Color(SK_ColorRED));
                break;
            case SkRuntimeEffect::ChildType::kColorFilter:
                children.push_back(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kModulate));
                break;
            case SkRuntimeEffect::ChildType::kBlender:
                children.push_back(SkBlenders::Arithmetic(0.50f, 0.25f, 0.10f, 0.05f, false));
                break;
        }
    }

    sk_sp<SkShader> shader = effect->makeShader(uniformBytes, SkMakeSpan(children));
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

bool FuzzSkRuntimeEffect(sk_sp<SkData> bytes) {
    // Test once with the inliner disabled...
    SkRuntimeEffect::Options options;
    options.forceNoInline = true;
    bool result = FuzzSkRuntimeEffect_Once(bytes, options);

    // ... and then with the inliner enabled.
    options.forceNoInline = false;
    result = FuzzSkRuntimeEffect_Once(bytes, options) || result;

    return result;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSkRuntimeEffect(bytes);
    return 0;
}
#endif
