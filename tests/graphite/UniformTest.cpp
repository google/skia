/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/UniformCache.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

namespace {

std::tuple<SkPaint, int> create_paint(skgpu::Combination combo) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numUniforms = 0;
    switch (combo.fShaderType) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            numUniforms += 1;
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, combo.fTileMode,
                                            0, 359, 0, nullptr);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, combo.fTileMode);
            numUniforms += 6;
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(combo.fBlendMode);
    return { p, numUniforms };
}

} // anonymous namespace

DEF_GRAPHITE_TEST(UniformTest, reporter) {
    using namespace skgpu;

    UniformCache cache;

    for (auto s : { ShaderCombo::ShaderType::kNone,
                    ShaderCombo::ShaderType::kLinearGradient,
                    ShaderCombo::ShaderType::kRadialGradient,
                    ShaderCombo::ShaderType::kSweepGradient,
                    ShaderCombo::ShaderType::kConicalGradient }) {
        for (auto tm: { SkTileMode::kClamp,
                        SkTileMode::kRepeat,
                        SkTileMode::kMirror,
                        SkTileMode::kDecal }) {
            if (s == ShaderCombo::ShaderType::kNone) {
                tm = SkTileMode::kRepeat;  // the TileMode doesn't matter for this case
            }

            for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
                Combination expected;

                expected.fShaderType = s;
                expected.fTileMode = tm;
                expected.fBlendMode = bm;

                auto [ p, expectedNumUniforms ] = create_paint(expected);
                auto [ actual, ud] = ExtractCombo(&cache, p);
                REPORTER_ASSERT(reporter, expected == actual);
                REPORTER_ASSERT(reporter, expectedNumUniforms == ud->count());
                for (int i = 0; i < ud->count(); ++i) {
                    REPORTER_ASSERT(reporter, ud->offset(i) >= 0 && ud->offset(i) < ud->dataSize());
                }
                REPORTER_ASSERT(reporter, ud->id() != UniformData::kInvalidUniformID);
            }
        }
    }
}
