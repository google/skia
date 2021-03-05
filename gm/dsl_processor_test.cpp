/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkFont.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/sksl/dsl/priv/DSLFPs.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "tools/ToolUtils.h"

class SimpleDSLEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 100;

    SimpleDSLEffect() : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
    }

    const char* name() const override { return "DSLEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                using namespace SkSL::dsl;
                StartFragmentProcessor(this, &args);

                // Test for skbug.com/11384
                Var x(kInt, 1);
                Declare(x);
                SkASSERT(DSLWriter::Var(x).initialValue()->description() == "1");

                Var blueAlpha(kUniform_Modifier, kHalf2);
                fBlueAlphaUniform = VarUniformHandle(blueAlpha);
                Var coords(kFloat4, sk_FragCoord());
                Declare(coords);
                Return(Half4(Swizzle(coords, X, Y) / 100, blueAlpha));
                EndFragmentProcessor();
            }

            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& effect) override {
                pdman.set2f(fBlueAlphaUniform, 0.0, 1.0);
            }

            GrGLSLProgramDataManager::UniformHandle fBlueAlphaUniform;
        };
        return std::make_unique<Impl>();
    }
};

DEF_SIMPLE_GPU_GM(simple_dsl_test, ctx, rtCtx, canvas, 100, 100) {
    auto fp = std::make_unique<SimpleDSLEffect>();
    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));
    rtCtx->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeIWH(100, 100));
}
