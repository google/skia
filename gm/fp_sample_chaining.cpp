/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkFont.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
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
        child->setSampleMatrix(SkSL::SampleMatrix(SkSL::SampleMatrix::Kind::kConstantOrUniform,
                                                  child.get(), "float3x3(float3(0.5, 0.0, 0.0), "
                                                                        "float3(0.0, 1.0, 0.0), "
                                                                        "float3(0.0, 0.0, 1.0))"));
        this->registerChildProcessor(std::move(child));
    }

    const char* name() const override { return "ConstantMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                SkString sample = this->invokeChild(0, args);
                args.fFragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, sample.c_str());
            }
        };
        return new Impl;
    }
};

// Samples child with a uniform matrix (functionally identical to GrMatrixEffect)
// Scales along Y
class UniformMatrixEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 4;

    UniformMatrixEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        // We should be passing 'this' as the owner (not child). As written, we fail to apply the
        // transform correctly, because the uniform is here (not on child). But if we use 'this',
        // the base pointer chaining is wrong, and we assert when emitting transform code.
        child->setSampleMatrix(SkSL::SampleMatrix(SkSL::SampleMatrix::Kind::kConstantOrUniform,
                                                  child.get(), "matrix"));
        this->registerChildProcessor(std::move(child));
    }

    const char* name() const override { return "UniformMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                fMatrixVar = args.fUniformHandler->addUniform(&args.fFp, kFragment_GrShaderFlag,
                                                              kFloat3x3_GrSLType, "matrix");
                SkString sample = this->invokeChild(0, args);
                args.fFragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, sample.c_str());
            }
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& proc) override {
                pdman.setSkMatrix(fMatrixVar, SkMatrix::Scale(1, 0.5f));
            }
            UniformHandle fMatrixVar;
        };
        return new Impl;
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
        child->setSampleMatrix(SkSL::SampleMatrix::Kind::kVariable);
        this->registerChildProcessor(std::move(child));
    }

    const char* name() const override { return "VariableMatrixEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                SkString sample = this->invokeChildWithMatrix(
                        0, args, "float3x3(1, 0, 0, 0, 1, 0, 8, 0, 1)");
                args.fFragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, sample.c_str());
            }
        };
        return new Impl;
    }
};

// Samples child with explicit coords
// Translates along Y
class ExplicitCoordEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 6;

    ExplicitCoordEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        child->setSampledWithExplicitCoords();
        this->registerChildProcessor(std::move(child));
        this->addCoordTransform(&fCoordTransform);
    }

    const char* name() const override { return "ExplicitCoordEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                SkString coords2D = args.fFragBuilder->ensureCoords2D(
                        args.fTransformedCoords[0].fVaryingPoint, args.fFp.sampleMatrix());
                args.fFragBuilder->codeAppendf("float2 coord = %s + float2(0, 8);",
                                               coords2D.c_str());
                SkString sample = this->invokeChild(0, args, "coord");
                args.fFragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, sample.c_str());
            }
        };
        return new Impl;
    }
    // Placeholder identity coord transform to allow access to local coords
    GrCoordTransform fCoordTransform = {};
};

// Generates test pattern
class TestPatternEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 7;

    TestPatternEffect() : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->addCoordTransform(&fCoordTransform);
    }

    const char* name() const override { return "TestPatternEffect"; }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class Impl : public GrGLSLFragmentProcessor {
            void emitCode(EmitArgs& args) override {
                auto fb = args.fFragBuilder;
                SkString coords2D = fb->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint,
                                                       args.fFp.sampleMatrix());
                fb->codeAppendf("float2 coord = %s / 64.0;", coords2D.c_str());
                fb->codeAppendf("coord = floor(coord * 4) / 3;");
                fb->codeAppendf("%s = half4(half2(coord.rg), 0, 1);\n", args.fOutputColor);
            }
        };
        return new Impl;
    }
    // Placeholder identity coord transform to allow access to local coords
    GrCoordTransform fCoordTransform = {};
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
            return std::unique_ptr<GrFragmentProcessor>(new ConstantMatrixEffect(std::move(fp)));
        case kUniform:
            return std::unique_ptr<GrFragmentProcessor>(new UniformMatrixEffect(std::move(fp)));
        case kVariable:
            return std::unique_ptr<GrFragmentProcessor>(new VariableMatrixEffect(std::move(fp)));
        case kExplicit:
            return std::unique_ptr<GrFragmentProcessor>(new ExplicitCoordEffect(std::move(fp)));
    }
    SkUNREACHABLE;
}

DEF_SIMPLE_GPU_GM(fp_sample_chaining, ctx, rtCtx, canvas, 1024, 256) {
    SkBitmap bmp = make_test_bitmap();

    GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
    int x = 10, y = 10;

    auto nextCol = [&] { x += (64 + 10); };
    auto nextRow = [&] { x = 10; y += (64 + 10); };

    auto draw = [&](std::initializer_list<EffectType> effects) {
        // Eanble TestPatternEffect to get a fully procedural inner effect. It's not quite as nice
        // visually (no text labels in each box), but it avoids the extra GrMatrixEffect.
        // Switching it on actually triggers *more* shader compilation failures.
#if 0
        auto fp = std::unique_ptr<GrFragmentProcessor>(new TestPatternEffect());
#else
        auto view = maker.view(GrMipMapped::kNo);
        auto fp = GrTextureEffect::Make(std::move(view), maker.alphaType());
#endif
        for (EffectType effectType : effects) {
            fp = wrap(std::move(fp), effectType);
        }
        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        rtCtx->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::Translate(x, y),
                        SkRect::MakeIWH(64, 64));
        nextCol();
    };

    // Reminder, in every case, the chain is more complicated than it seems, because the
    // GrTextureEffect is wrapped in a GrMatrixEffect, which is subject to the same bugs that
    // we're testing (particularly the bug about owner/base in UniformMatrixEffect).

    // First row: no transform, then each one independently applied
    draw({});
    draw({ kConstant });  // Correct
    draw({ kUniform  });  // Bug #1: No transform applied (see comment in UniformMatrixEffect)
    draw({ kVariable });  // Correct
    draw({ kExplicit });  // Correct
    nextRow();

    nextCol();
    draw({ kConstant, kConstant });  // Correct
    draw({ kUniform,  kUniform  });  // Bug #2 (#1B): Only one transform applied
    draw({ kVariable, kVariable });  // Bug #3: Only one transform applied (unknown issue)
    draw({ kExplicit, kExplicit });  // Correct
    nextRow();

    // Remember, these are applied inside out:
//    draw({ kConstant, kUniform  });  // Bug #4 (#1C?): Shader error (unknown identifier 'matrix')
//    draw({ kConstant, kVariable });  // Bug #5 (#3B?): Shader error (unknown identifier 'uCoordTransformMatrix_0_Stage0)
    draw({ kConstant, kExplicit });  // Correct
    nextRow();

//    draw({ kExplicit, kConstant });  // Bug #6: Shader error (unknown identifier 'uCoordTransformMatrix_0_Stage0)
//    draw({ kExplicit, kUniform  });  // Bug #7 (#6B?): Shader error (unknown identifier 'uCoordTransformMatrix_0_Stage0)
//    draw({ kExplicit, kVariable });  // Bug #8: SkASSERT(!hasVariableMatrix) in writeProcessorFunction
}
