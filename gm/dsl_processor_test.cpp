/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/sksl/dsl/priv/DSLFPs.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLVariable.h"

class SimpleDSLEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 100;

    SimpleDSLEffect() : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
    }

    const char* name() const override { return "DSLEffect"; }
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
            void emitCode(EmitArgs& args) override {
                using namespace SkSL::dsl;
                StartFragmentProcessor(this, &args);

                // Test for skbug.com/11384
                Var x(kInt_Type, 1);
                Declare(x);
                SkASSERT(DSLWriter::Var(x)->initialValue()->description() == "1");

                GlobalVar blueAlpha(kUniform_Modifier, kHalf2_Type, "blueAlpha");
                Declare(blueAlpha);
                fBlueAlphaUniform = VarUniformHandle(blueAlpha);
                Var coords(kFloat4_Type, sk_FragCoord());
                Declare(coords);
                Return(Half4(Swizzle(coords, X, Y) / 100, blueAlpha));
                EndFragmentProcessor();
            }

        private:
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& effect) override {
                pdman.set2f(fBlueAlphaUniform, 0.0, 1.0);
            }

            GrGLSLProgramDataManager::UniformHandle fBlueAlphaUniform;
        };
        return std::make_unique<Impl>();
    }
};

DEF_SIMPLE_GPU_GM(simple_dsl_test, rContext, canvas, 100, 100) {
    auto sfc = SkCanvasPriv::TopDeviceSurfaceFillContext(canvas);
    if (!sfc) {
        return;
    }

    sfc->fillWithFP(std::make_unique<SimpleDSLEffect>());
}
