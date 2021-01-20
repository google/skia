/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static constexpr int kWidth       = 800;
static constexpr int kBoxSize     = 24;
static constexpr int kPadding     = 3;

static void next_row(SkCanvas* canvas) {
    canvas->restore();
    canvas->translate(0, kPadding + kBoxSize + kPadding);
    canvas->save();
}

static constexpr int rows_to_height(int rows) {
    return (kPadding + kBoxSize + kPadding) * rows;
}

static void draw_label(SkCanvas* canvas, SkString label) {
    SkFont font(ToolUtils::create_portable_typeface());
    SkPaint paint(SkColors::kBlack);
    canvas->drawSimpleText(label.c_str(), label.size(), SkTextEncoding::kUTF8,
                           kBoxSize + kPadding, kBoxSize * 2 / 3, font, paint);
}

class RuntimeSkSLGM : public skiagm::GM {
public:
    RuntimeSkSLGM(const char* name, std::vector<const char*> testFiles)
            : fName(SkString(name))
            , fTestFiles(std::move(testFiles)) {}

private:
    struct EffectInfo {
        sk_sp<SkRuntimeEffect> fEffect;
        SkString               fMessage;
    };

    SkString fName;
    std::vector<const char*> fTestFiles;
    std::vector<EffectInfo> fEffectInfo;

    void onOnceBeforeDraw() override {
        // Create a plain-black fallback shader for shaders that don't compile.
        static const SkString kFallbackShader{"vec4 main() { return vec4(0, 0, 0, 1); }"};
        auto [fallback, fallbackError] = SkRuntimeEffect::Make(kFallbackShader);
        SkASSERTF(fallback, "Unable to create fallback shader");

        fEffectInfo.reserve(fTestFiles.size());

        for (const char* testFile : fTestFiles) {
            SkString resourcePath = SkStringPrintf("sksl/%s", testFile);
            sk_sp<SkData> shaderData = GetResourceAsData(resourcePath.c_str());
            if (!shaderData) {
                fEffectInfo.push_back(
                        {/*fEffect=*/fallback,
                         /*fMessage=*/SkStringPrintf("%s: Unable to load file", testFile)});
                continue;
            }

            SkRuntimeEffect::EffectResult shader;
            SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()),
                                  shaderData->size()};
            auto [effect, error] = SkRuntimeEffect::Make(shaderString);
            if (effect) {
                fEffectInfo.push_back({/*fEffect=*/effect,
                                       /*fMessage=*/SkString(testFile)});
            } else {
                fEffectInfo.push_back(
                        {/*fEffect=*/fallback,
                         /*fMessage=*/SkStringPrintf("%s: %s", testFile, error.c_str())});
            }
        }
    }

    SkString onShortName() override { return fName; }

    SkISize onISize() override { return {kWidth, rows_to_height(fTestFiles.size())}; }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(kPadding, kPadding);
        canvas->save();

        for (EffectInfo& effectInfo : fEffectInfo) {
            draw_label(canvas, effectInfo.fMessage);

            SkRuntimeShaderBuilder builder(effectInfo.fEffect);
            sk_sp<SkShader> shader = builder.makeShader(/*localMatrix=*/nullptr, /*isOpaque=*/true);
            SkASSERT(shader);

            static const SkRect kRect = SkRect::MakeWH(kBoxSize, kBoxSize);

            SkPaint paintShader;
            paintShader.setShader(shader);
            canvas->drawRect(kRect, paintShader);

            SkPaint paintOutline;
            paintOutline.setStyle(SkPaint::kStroke_Style);
            paintOutline.setStrokeWidth(SkIntToScalar(2));
            paintOutline.setColor(SK_ColorBLACK);
            canvas->drawRect(kRect, paintOutline);

            next_row(canvas);
        }
    }
};

DEF_GM(return new RuntimeSkSLGM("runtime_sksl_folding",
                                {
                                    "folding/BoolFolding.sksl",
                                    "folding/FloatFolding.sksl",
                                    "folding/IntFoldingES2.sksl",
                                    "folding/IntFoldingES3.sksl",
                                    "folding/MatrixFoldingES2.sksl",
                                    "folding/MatrixFoldingES3.sksl",
                                    "folding/ShortCircuitBoolFolding.sksl",
                                });)
