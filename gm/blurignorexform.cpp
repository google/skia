/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"

class BlurIgnoreXformGM : public skiagm::GM {
public:
    BlurIgnoreXformGM() { }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        return SkString("blur_ignore_xform");
    }

    SkISize onISize() override {
        return SkISize::Make(975, 475);
    }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < kNumBlurs; ++i) {
            fBlurFilters[i] = SkBlurMaskFilter::Make(
                                    kNormal_SkBlurStyle,
                                    SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20)),
                                    SkBlurMaskFilter::kHighQuality_BlurFlag | kBlurFlags[i].fFlags);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 25);
        canvas->save();
        canvas->translate(80, 0);
        for (int drawType = 0; drawType < 3; ++drawType) {
            for (size_t i = 0; i < kNumBlurs; ++i) {
                SkAutoCanvasRestore autoRestore(canvas, true);
                canvas->translate(SkIntToScalar(i * 150), 0);
                for (auto scale : kMatrixScales) {
                    canvas->save();
                    canvas->scale(scale.fScale, scale.fScale);
                    SkPaint paint;
                    paint.setColor(SK_ColorBLACK);
                    paint.setAntiAlias(true);
                    static const SkScalar kRadius = 20.0f;
                    SkScalar coord = 50.0f * 1.0f / scale.fScale;
                    SkRect rect = SkRect::MakeXYWH(coord - kRadius , coord - kRadius,
                                                   2 * kRadius, 2 * kRadius);
                    SkRRect rrect = SkRRect::MakeRectXY(rect, kRadius/2.0f, kRadius/2.0f);
                    for (int k = 0; k < 2; ++k) {
                        if (0 == drawType) {
                            canvas->drawCircle(coord, coord, kRadius, paint);
                        } else if (1 == drawType) {
                            canvas->drawRect(rect, paint);
                        } else {
                            canvas->drawRRect(rrect, paint);
                        }
                        canvas->translate(10,10);
                        paint.setMaskFilter(fBlurFilters[i]);
                    }
                    canvas->restore();
                    canvas->translate(0, SkIntToScalar(150));
                }
            }
            canvas->translate(SkIntToScalar(300), 0);
        }
        canvas->restore();
        if (kBench_Mode != this->getMode()) {
            this->drawOverlay(canvas);
        }
    }

    void drawOverlay(SkCanvas* canvas) {
        canvas->translate(10, 0);
        SkPaint textPaint;
        sk_tool_utils::set_portable_typeface(&textPaint);
        textPaint.setAntiAlias(true);
        canvas->save();
        for (int j = 0; j < 3; ++j) {
            for (int i = 0; i < kNumBlurs; ++i) {
                canvas->drawString(kBlurFlags[i].fName, 100, 0, textPaint);
                canvas->translate(SkIntToScalar(130), 0);
            }
            canvas->translate(SkIntToScalar(40), 0);
        }
        canvas->restore();
        for (auto scale : kMatrixScales) {
            canvas->drawString(scale.fName, 0, 50, textPaint);
            canvas->translate(0, SkIntToScalar(150));
        }
    }

private:
    static constexpr int kNumBlurs = 2;

    static const struct BlurFlags {
        uint32_t fFlags;
        const char* fName;
    } kBlurFlags[kNumBlurs];

    static const struct MatrixScale {
        float fScale;
        const char* fName;
    } kMatrixScales[3];

    sk_sp<SkMaskFilter> fBlurFilters[kNumBlurs];

    typedef         skiagm::GM INHERITED;
};

const BlurIgnoreXformGM::BlurFlags BlurIgnoreXformGM::kBlurFlags[] = {
    {0, "none"},
    {SkBlurMaskFilter::kIgnoreTransform_BlurFlag, "IgnoreTransform"}
};

const BlurIgnoreXformGM::MatrixScale BlurIgnoreXformGM::kMatrixScales[] = {
    {1.0f, "Identity"},
    {0.5f, "Scale = 0.5"},
    {2.0f, "Scale = 2.0"}
};

DEF_GM(return new BlurIgnoreXformGM();)
