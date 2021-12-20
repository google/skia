/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkFont.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/ToolUtils.h"

namespace {

// Samples child with a uniform matrix (functionally identical to GrMatrixEffect)
// Scales along Y
class UniformMatrixEffect : public GrFragmentProcessor {
public:
    inline static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 4;

    UniformMatrixEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child),
                            SkSL::SampleUsage::UniformMatrix(/*hasPerspective=*/false));
    }

    const char* name() const override { return "UniformMatrixEffect"; }
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
            void emitCode(EmitArgs& args) override {
                fMatrixVar =
                        args.fUniformHandler->addUniform(&args.fFp,
                                                         kFragment_GrShaderFlag,
                                                         kFloat3x3_GrSLType,
                                                         SkSL::SampleUsage::MatrixUniformName());
                SkString sample = this->invokeChildWithMatrix(0, args);
                args.fFragBuilder->codeAppendf("return %s;\n", sample.c_str());
            }

        private:
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& proc) override {
                pdman.setSkMatrix(fMatrixVar, SkMatrix::Scale(1, 0.5f));
            }
            UniformHandle fMatrixVar;
        };
        return std::make_unique<Impl>();
    }
};

// Samples child with explicit coords
// Translates along Y
class ExplicitCoordEffect : public GrFragmentProcessor {
public:
    inline static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 6;

    ExplicitCoordEffect(std::unique_ptr<GrFragmentProcessor> child)
            : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
        this->setUsesSampleCoordsDirectly();
    }

    const char* name() const override { return "ExplicitCoordEffect"; }
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
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
    inline static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 7;

    TestPatternEffect() : GrFragmentProcessor(CLASS_ID, kNone_OptimizationFlags) {
        this->setUsesSampleCoordsDirectly();
    }

    const char* name() const override { return "TestPatternEffect"; }
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }
    std::unique_ptr<GrFragmentProcessor> clone() const override { return nullptr; }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        class Impl : public ProgramImpl {
        public:
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
    kUniform,
    kExplicit,
    kDevice,
};

static std::unique_ptr<GrFragmentProcessor> wrap(std::unique_ptr<GrFragmentProcessor> fp,
                                                 EffectType effectType,
                                                 int drawX, int drawY) {
    switch (effectType) {
        case kUniform:
            return std::make_unique<UniformMatrixEffect>(std::move(fp));
        case kExplicit:
            return std::make_unique<ExplicitCoordEffect>(std::move(fp));
        case kDevice:
            // Subtract out upper-left corner of draw so that device is effectively identity.
            fp = GrMatrixEffect::Make(SkMatrix::Translate(-drawX, -drawY), std::move(fp));
            return GrFragmentProcessor::DeviceSpace(std::move(fp));
    }
    SkUNREACHABLE;
}

} // namespace

namespace skiagm {

DEF_SIMPLE_GPU_GM_CAN_FAIL(fp_sample_chaining, rContext, canvas, errorMsg, 232, 306) {
    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    SkBitmap bmp = make_test_bitmap();

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
        auto view = std::get<0>(GrMakeCachedBitmapProxyView(rContext, bmp, GrMipmapped::kNo));
        auto fp = GrTextureEffect::Make(std::move(view), bmp.alphaType());
#endif
        for (EffectType effectType : effects) {
            fp = wrap(std::move(fp), effectType, x, y);
        }
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::Translate(x, y),
                      SkRect::MakeIWH(64, 64));
        nextCol();
    };

    // Reminder, in every case, the chain is more complicated than it seems, because the
    // GrTextureEffect is wrapped in a GrMatrixEffect, which is subject to the same bugs that
    // we're testing (particularly the bug about owner/base in UniformMatrixEffect).

    // First row: no transform, then each one independently applied
    draw({});             // Identity (4 rows and columns)
    draw({ kUniform  });  // Scale Y axis by 2x (2 visible rows)
    draw({ kExplicit });  // Translate up by 8px
    nextRow();

    // Second row: transform duplicated
    draw({ kUniform,  kUniform  });  // Scale Y axis by 4x (1 visible row)
    draw({ kExplicit, kExplicit });  // Translate up by 16px
    nextRow();

    // Third row: Remember, these are applied inside out:
    draw({ kUniform,  kExplicit }); // Scale Y by 2x and translate up by 8px
    draw({ kExplicit, kUniform });  // Scale Y by 2x and translate up by 16px
    nextRow();

    // Fourth row: device space.
    draw({ kDevice, kUniform });                     // Same as identity (uniform applied *before*
                                                     // device so ignored).
    draw({ kExplicit, kUniform, kDevice });          // Scale Y by 2x and translate up by 16px
    draw({ kDevice, kExplicit, kUniform, kDevice }); // Identity, again.

    return DrawResult::kOk;
}

} // namespace skiagm
