/*
 * Copyright 2019 Google LLC.
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
#include "tools/ToolUtils.h"

// Samples child with a constant (literal) matrix
// Scales along X
class ConstantMatrixEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 3;

    ConstantMatrixEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child),
                            SkSL::SampleUsage::UniformMatrix(
                                "float3x3(float3(0.5, 0.0, 0.0), "
                                        "float3(0.0, 1.0, 0.0), "
                                        "float3(0.0, 0.0, 1.0))"));
    }

    const char* name() const override { return "ConstantMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                SkString sample = this->invokeChildWithMatrix(0, args);
                args.fFragBuilder->codeAppendf("return %s;\n", sample.c_str());
            }
        };
        return std::make_unique<Impl>();
    }
};

// Samples child with a uniform matrix (functionally identical to GrMatrixEffect)
// Scales along Y
class UniformMatrixEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 4;

    UniformMatrixEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::UniformMatrix("matrix"));
    }

    const char* name() const override { return "UniformMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                fMatrixVar = args.fUniformHandler->addUniform(&args.fFp, kFragment_GrShaderFlag,
                                                              kFloat3x3_GrSLType, "matrix");
                SkString sample = this->invokeChildWithMatrix(0, args);
                args.fFragBuilder->codeAppendf("return %s;\n", sample.c_str());
            }
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& proc) override {
                pdman.setSkMatrix(fMatrixVar, SkMatrix::Scale(1, 0.5f));
            }
            UniformHandle fMatrixVar;
        };
        return std::make_unique<Impl>();
    }
};

// Samples child with a variable matrix
// Translates along X
// Typically, kVariable would be due to multiple sample(matrix) invocations, but this artificially
// uses kVariable with a single (constant) matrix.
class VariableMatrixEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 5;

    VariableMatrixEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::VariableMatrix());
    }

    const char* name() const override { return "VariableMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                SkString sample = this->invokeChildWithMatrix(
                        0, args, "float3x3(1, 0, 0, 0, 1, 0, 8, 0, 1)");
                args.fFragBuilder->codeAppendf("return %s;\n", sample.c_str());
            }
        };
        return std::make_unique<Impl>();
    }
};

// Samples child with explicit coords
// Translates along Y
class ExplicitCoordEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 6;

    ExplicitCoordEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
        this->setUsesSampleCoordsDirectly();
    }

    const char* name() const override { return "ExplicitCoordEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                args.fFragBuilder->codeAppendf("float2 coord = %s + float2(0, 8);",
                                               args.fSampleCoord);
                SkString sample = this->invokeChild(0, args, "coord");
                args.fFragBuilder->codeAppendf("return %s;\n", sample.c_str());
            }
        };
        return std::make_unique<Impl>();
    }
};

// Generates test pattern
class TestPatternEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 7;

    TestPatternEffect() : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->setUsesSampleCoordsDirectly();
    }

    const char* name() const override { return "TestPatternEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                auto fb = args.fFragBuilder;
                fb->codeAppendf("float2 coord = %s / 64.0;", args.fSampleCoord);
                fb->codeAppendf("coord = floor(coord * 4) / 3;");
                fb->codeAppendf("return half2(coord).rg01;\n");
            }
        };
        return std::make_unique<Impl>();
    }
};

SkBitmap make_test_bitmap() {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(64, 64);
    SkCanvas canvas(bitmap);

    SkFont font(ToolUtils::create_portable_typeface());
    const char* alpha = "ABCDEFGHIJKLMNOP";

    for (int i = 0; i < 16; ++i) {
        int tx = i % 4,
            ty = i / 4;
        int x = tx * 16,
            y = ty * 16;
        SkPaint paint;
        paint.setColor4f({ tx / 3.0f, ty / 3.0f, 0.0f, 1.0f });
        canvas.drawRect(SkRect::MakeXYWH(x, y, 16, 16), paint);
        paint.setColor4f({ (3-tx) / 3.0f, (3-ty)/3.0f, 1.0f, 1.0f });
        canvas.drawSimpleText(alpha + i, 1, SkTextEncoding::kUTF8, x + 3, y + 13, font, paint);
    }

    return bitmap;
}

enum EffectType {
    kConstant,
    kUniform,
    kVariable,
    kExplicit,
};

static std::unique_ptr<GrFragmentProcessor> wrap(std::unique_ptr<GrFragmentProcessor> fp,
                                                 EffectType effectType) {
    switch (effectType) {
        case kConstant:
            return std::make_unique<ConstantMatrixEffect>(std::move(fp));
        case kUniform:
            return std::make_unique<UniformMatrixEffect>(std::move(fp));
        case kVariable:
            return std::make_unique<VariableMatrixEffect>(std::move(fp));
        case kExplicit:
            return std::make_unique<ExplicitCoordEffect>(std::move(fp));
    }
    SkUNREACHABLE;
}

DEF_SIMPLE_GPU_GM(fp_sample_chaining, ctx, rtCtx, canvas, 380, 306) {
    SkBitmap bmp = make_test_bitmap();

    GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
    int x = 10, y = 10;

    auto nextCol = [&] { x += (64 + 10); };
    auto nextRow = [&] { x = 10; y += (64 + 10); };

    auto draw = [&](std::initializer_list<EffectType> effects) {
        // Enable TestPatternEffect to get a fully procedural inner effect. It's not quite as nice
        // visually (no text labels in each box), but it avoids the extra GrMatrixEffect.
        // Switching it on actually triggers *more* shader compilation failures.
#if 0
        auto fp = std::unique_ptr<GrFragmentProcessor>(new TestPatternEffect());
#else
        auto view = maker.view(GrMipmapped::kNo);
        auto fp = GrTextureEffect::Make(std::move(view), maker.alphaType());
#endif
        for (EffectType effectType : effects) {
            fp = wrap(std::move(fp), effectType);
        }
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        rtCtx->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::Translate(x, y),
                        SkRect::MakeIWH(64, 64));
        nextCol();
    };

    // Reminder, in every case, the chain is more complicated than it seems, because the
    // GrTextureEffect is wrapped in a GrMatrixEffect, which is subject to the same bugs that
    // we're testing (particularly the bug about owner/base in UniformMatrixEffect).

    // First row: no transform, then each one independently applied
    draw({});             // Identity (4 rows and columns)
    draw({ kConstant });  // Scale X axis by 2x (2 visible columns)
    draw({ kUniform  });  // Scale Y axis by 2x (2 visible rows)
    draw({ kVariable });  // Translate left by 8px
    draw({ kExplicit });  // Translate up by 8px
    nextRow();

    // Second row: transform duplicated
    draw({ kConstant, kUniform  });  // Scale XY by 2x (2 rows and columns)
    draw({ kConstant, kConstant });  // Scale X axis by 4x (1 visible column)
    draw({ kUniform,  kUniform  });  // Scale Y axis by 4x (1 visible row)
    draw({ kVariable, kVariable });  // Translate left by 16px
    draw({ kExplicit, kExplicit });  // Translate up by 16px
    nextRow();

    // Remember, these are applied inside out:
    draw({ kConstant, kExplicit }); // Scale X by 2x and translate up by 8px
    draw({ kConstant, kVariable }); // Scale X by 2x and translate left by 8px
    draw({ kUniform,  kVariable }); // Scale Y by 2x and translate left by 8px
    draw({ kUniform,  kExplicit }); // Scale Y by 2x and translate up by 8px
    draw({ kVariable, kExplicit }); // Translate left and up by 8px
    nextRow();

    draw({ kExplicit, kExplicit, kConstant }); // Scale X by 2x and translate up by 16px
    draw({ kVariable, kConstant }); // Scale X by 2x and translate left by 16px
    draw({ kVariable, kVariable, kUniform }); // Scale Y by 2x and translate left by 16px
    draw({ kExplicit, kUniform }); // Scale Y by 2x and translate up by 16px
    draw({ kExplicit, kUniform, kVariable, kConstant }); // Scale XY by 2x and translate xy 16px
}

const char* gConstantMatrixSkSL = R"(
    uniform shader child;
    half4 main(float2 xy) {
        return sample(child, float3x3(0.5, 0.0, 0.0,
                                      0.0, 1.0, 0.0,
                                      0.0, 0.0, 1.0));
    }
)";

const char* gUniformMatrixSkSL = R"(
    uniform shader child;
    uniform float3x3 matrix;
    half4 main(float2 xy) {
        return sample(child, matrix);
    }
)";

// This form (uniform * constant) is currently detected as variable, thanks to our limited analysis
// when scanning for sample matrices. With that pulled into a separate local, it's highly unlikely
// we'll ever treat this as anything else.
const char* gVariableMatrixSkSL = R"(
    uniform shader child;
    uniform float3x3 matrix;
    half4 main(float2 xy) {
        float3x3 varMatrix = matrix * 0.5;
        return sample(child, varMatrix);
    }
)";

const char* gExplicitCoordSkSL = R"(
    uniform shader child;
    half4 main(float2 xy) {
        return sample(child, xy + float2(0, 8));
    }
)";

// Version of fp_sample_chaining that uses SkRuntimeEffect
DEF_SIMPLE_GM(sksl_sample_chaining, canvas, 380, 306) {
    SkBitmap bmp = make_test_bitmap();

    sk_sp<SkRuntimeEffect> effects[4] = {
        SkRuntimeEffect::Make(SkString(gConstantMatrixSkSL)).effect,
        SkRuntimeEffect::Make(SkString(gUniformMatrixSkSL)).effect,
        SkRuntimeEffect::Make(SkString(gVariableMatrixSkSL)).effect,
        SkRuntimeEffect::Make(SkString(gExplicitCoordSkSL)).effect,
    };

    canvas->translate(10, 10);
    canvas->save();
    auto nextCol = [&] { canvas->translate(64 + 10, 0); };
    auto nextRow = [&] { canvas->restore(); canvas->translate(0, 64 + 10); canvas->save(); };

    auto draw = [&](std::initializer_list<EffectType> effectTypes) {
        auto shader = bmp.makeShader(SkSamplingOptions());

        for (EffectType effectType : effectTypes) {
            SkRuntimeShaderBuilder builder(effects[effectType]);
            builder.child("child") = shader;
            switch (effectType) {
                case kUniform:
                    builder.uniform("matrix") = SkMatrix::Scale(1.0f, 0.5f);
                    break;
                case kVariable:
                    builder.uniform("matrix") = SkMatrix::Translate(8, 0);
                    break;
                default:
                    break;
            }
            shader = builder.makeShader(nullptr, true);
        }
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeWH(64, 64), paint);
        nextCol();
    };

    // Reminder, in every case, the chain is more complicated than it seems, because the
    // GrTextureEffect is wrapped in a GrMatrixEffect, which is subject to the same bugs that
    // we're testing (particularly the bug about owner/base in UniformMatrixEffect).

    // First row: no transform, then each one independently applied
    draw({});             // Identity (4 rows and columns)
    draw({ kConstant });  // Scale X axis by 2x (2 visible columns)
    draw({ kUniform  });  // Scale Y axis by 2x (2 visible rows)
    draw({ kVariable });  // Translate left by 8px
    draw({ kExplicit });  // Translate up by 8px
    nextRow();

    // Second row: transform duplicated
    draw({ kConstant, kUniform  });  // Scale XY by 2x (2 rows and columns)
    draw({ kConstant, kConstant });  // Scale X axis by 4x (1 visible column)
    draw({ kUniform,  kUniform  });  // Scale Y axis by 4x (1 visible row)
    draw({ kVariable, kVariable });  // Translate left by 16px
    draw({ kExplicit, kExplicit });  // Translate up by 16px
    nextRow();

    // Remember, these are applied inside out:
    draw({ kConstant, kExplicit }); // Scale X by 2x and translate up by 8px
    draw({ kConstant, kVariable }); // Scale X by 2x and translate left by 8px
    draw({ kUniform,  kVariable }); // Scale Y by 2x and translate left by 8px
    draw({ kUniform,  kExplicit }); // Scale Y by 2x and translate up by 8px
    draw({ kVariable, kExplicit }); // Translate left and up by 8px
    nextRow();

    draw({ kExplicit, kExplicit, kConstant }); // Scale X by 2x and translate up by 16px
    draw({ kVariable, kConstant }); // Scale X by 2x and translate left by 16px
    draw({ kVariable, kVariable, kUniform }); // Scale Y by 2x and translate left by 16px
    draw({ kExplicit, kUniform }); // Scale Y by 2x and translate up by 16px
    draw({ kExplicit, kUniform, kVariable, kConstant }); // Scale XY by 2x and translate xy 16px
}
