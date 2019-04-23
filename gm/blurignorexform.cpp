/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "tools/ToolUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "src/core/SkBlurMask.h"

// This GM tests out the SkBlurMaskFilter's kIgnoreTransform flag. That flag causes the blur mask
// filter to not apply the CTM to the blur's radius.
class BlurIgnoreXformGM : public skiagm::GM {
public:
    enum class DrawType {
        kCircle,
        kRect,
        kRRect,
    };

    BlurIgnoreXformGM(DrawType drawType) : fDrawType(drawType) { }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        SkString name;
        name.printf("blur_ignore_xform_%s",
                    DrawType::kCircle == fDrawType ? "circle"
                        : DrawType::kRect == fDrawType ? "rect" : "rrect");
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(375, 475);
    }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < kNumBlurs; ++i) {
            fBlurFilters[i] = SkMaskFilter::MakeBlur(
                                    kNormal_SkBlurStyle,
                                    SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20)),
                                    kBlurFlags[i].fRespectCTM);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);

        canvas->translate(10, 25);
        canvas->save();
        canvas->translate(80, 0);
        for (size_t i = 0; i < kNumBlurs; ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(SkIntToScalar(i * 150), 0);
            for (auto scale : kMatrixScales) {
                canvas->save();
                canvas->scale(scale.fScale, scale.fScale);
                static const SkScalar kRadius = 20.0f;
                SkScalar coord = 50.0f * 1.0f / scale.fScale;
                SkRect rect = SkRect::MakeXYWH(coord - kRadius , coord - kRadius,
                                               2 * kRadius, 2 * kRadius);
                SkRRect rrect = SkRRect::MakeRectXY(rect, kRadius/2.0f, kRadius/2.0f);

                paint.setMaskFilter(fBlurFilters[i]);
                for (int j = 0; j < 2; ++j) {
                    canvas->save();
                    canvas->translate(10 * (1 - j), 10 * (1 - j));
                    if (DrawType::kCircle == fDrawType) {
                        canvas->drawCircle(coord, coord, kRadius, paint);
                    } else if (DrawType::kRect == fDrawType) {
                        canvas->drawRect(rect, paint);
                    } else {
                        canvas->drawRRect(rrect, paint);
                    }
                    paint.setMaskFilter(nullptr);
                    canvas->restore();
                }
                canvas->restore();
                canvas->translate(0, SkIntToScalar(150));
            }
        }
        canvas->restore();
        if (kBench_Mode != this->getMode()) {
            this->drawOverlay(canvas);
        }
    }

    void drawOverlay(SkCanvas* canvas) {
        canvas->translate(10, 0);
        SkFont font(ToolUtils::create_portable_typeface());
        canvas->save();
        for (int i = 0; i < kNumBlurs; ++i) {
            canvas->drawString(kBlurFlags[i].fName, 100, 0, font, SkPaint());
            canvas->translate(SkIntToScalar(130), 0);
        }
        canvas->restore();
        for (auto scale : kMatrixScales) {
            canvas->drawString(scale.fName, 0, 50, font, SkPaint());
            canvas->translate(0, SkIntToScalar(150));
        }
    }

private:
    static constexpr int kNumBlurs = 2;

    static const struct BlurFlags {
        bool fRespectCTM;
        const char* fName;
    } kBlurFlags[kNumBlurs];

    static const struct MatrixScale {
        float fScale;
        const char* fName;
    } kMatrixScales[3];

    DrawType fDrawType;
    sk_sp<SkMaskFilter> fBlurFilters[kNumBlurs];

    typedef         skiagm::GM INHERITED;
};

const BlurIgnoreXformGM::BlurFlags BlurIgnoreXformGM::kBlurFlags[] = {
    {true, "none"},
    {false, "IgnoreTransform"}
};

const BlurIgnoreXformGM::MatrixScale BlurIgnoreXformGM::kMatrixScales[] = {
    {1.0f, "Identity"},
    {0.5f, "Scale = 0.5"},
    {2.0f, "Scale = 2.0"}
};

DEF_GM(return new BlurIgnoreXformGM(BlurIgnoreXformGM::DrawType::kCircle);)
DEF_GM(return new BlurIgnoreXformGM(BlurIgnoreXformGM::DrawType::kRect);)
DEF_GM(return new BlurIgnoreXformGM(BlurIgnoreXformGM::DrawType::kRRect);)



