/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/effects/SkGradientShader.h"
#include "src/base/SkRandom.h"
#include "tools/viewer/Slide.h"

// Implementation in C++ of some WebKit MotionMark tests
// Tests implemented so far:
// * Lines
// Based on https://github.com/WebKit/MotionMark/blob/main/MotionMark/

class MMObject {
public:
    virtual ~MMObject() = default;

    virtual void draw(SkCanvas* canvas) = 0;

    virtual void animate(double /*nanos*/) = 0;
};

class Stage {
public:
    Stage(SkSize size) : fSize(size) {}
    virtual ~Stage() = default;

    virtual void draw(SkCanvas* canvas) = 0;

    virtual bool animate(double /*nanos*/) = 0;

    virtual bool onChar(SkUnichar uni) = 0;

protected:
    [[maybe_unused]] SkSize fSize;
    std::vector<std::unique_ptr<MMObject>> fObjects;
    SkRandom fRandom;
};

namespace {
float time_counter_value(double nanos, float factor) {
    constexpr double kMillisPerNano = 0.0000001;
    return static_cast<float>(nanos*kMillisPerNano)/factor;
}

float time_fractional_value(double nanos, float cycleLengthMs) {
    return SkScalarFraction(time_counter_value(nanos, cycleLengthMs));
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Canvas Lines
///////////////////////////////////////////////////////////////////////////////////////////////////
struct LineSegmentParams {
    float fCircleRadius;
    SkPoint fCircleCenters[4];
    float fLineLengthMaximum;
    float fLineMinimum;
};

class CanvasLineSegment : public MMObject {
public:
    CanvasLineSegment(SkRandom* random, const LineSegmentParams& params) {
        int circle = random->nextRangeU(0, 3);

        static constexpr SkColor kColors[] = {
            0xffe01040, 0xff10c030, 0xff744cba, 0xffe05010
        };
        fColor = kColors[circle];
        fLineWidth = sk_float_pow(random->nextF(), 12) * 20 + 3;
        fOmega = random->nextF() * 3 + 0.2f;
        float theta = random->nextRangeF(0, 2*SK_ScalarPI);
        fCosTheta = sk_float_cos(theta);
        fSinTheta = sk_float_sin(theta);
        fStart = params.fCircleCenters[circle] + SkPoint::Make(params.fCircleRadius * fCosTheta,
                                                               params.fCircleRadius * fSinTheta);
        fLength = params.fLineMinimum;
        fLength += sk_float_pow(random->nextF(), 8) * params.fLineLengthMaximum;
        fSegmentDirection = random->nextF() > 0.5 ? -1 : 1;
    }

    ~CanvasLineSegment() override = default;

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(fColor);
        paint.setStrokeWidth(fLineWidth);
        paint.setStyle(SkPaint::kStroke_Style);

        SkPoint end = {
            fStart.fX + fSegmentDirection * fLength * fCosTheta,
            fStart.fY + fSegmentDirection * fLength * fSinTheta
        };
        canvas->drawLine(fStart, end, paint);
    }

    void animate(double nanos) override {
        fLength += sk_float_sin(time_counter_value(nanos, 100) * fOmega);
    }

private:
    SkColor fColor;
    float fLineWidth;
    float fOmega;
    float fCosTheta;
    float fSinTheta;
    SkPoint fStart;
    float fLength;
    float fSegmentDirection;
};

class CanvasLineSegmentStage : public Stage {
public:
    CanvasLineSegmentStage(SkSize size) : Stage(size) {
        fParams.fLineMinimum = 20;
        fParams.fLineLengthMaximum = 40;
        fParams.fCircleRadius = fSize.fWidth/8 - .4 * (fParams.fLineMinimum +
                                                       fParams.fLineLengthMaximum);
        fParams.fCircleCenters[0] = SkPoint::Make(5.5 / 32 * fSize.fWidth, 2.1 / 3*fSize.fHeight);
        fParams.fCircleCenters[1] = SkPoint::Make(12.5 / 32 * fSize.fWidth, .9 / 3*fSize.fHeight);
        fParams.fCircleCenters[2] = SkPoint::Make(19.5 / 32 * fSize.fWidth, 2.1 / 3*fSize.fHeight);
        fParams.fCircleCenters[3] = SkPoint::Make(26.5 / 32 * fSize.fWidth, .9 / 3*fSize.fHeight);
        fHalfSize = SkSize::Make(fSize.fWidth * 0.5f, fSize.fHeight * 0.5f);
        fTwoFifthsSizeX = fSize.fWidth * .4;

        for (int i = 0; i < 5000; ++i) {
            std::unique_ptr<CanvasLineSegment> segment =
                    std::make_unique<CanvasLineSegment>(&fRandom, fParams);
            fObjects.push_back(std::move(segment));
        }
    }

    ~CanvasLineSegmentStage() override = default;

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        float dx = fTwoFifthsSizeX * sk_float_cos(fCurrentAngle);
        float dy = fTwoFifthsSizeX * sk_float_sin(fCurrentAngle);

        float colorStopStep = SkScalarInterp(-.1f, .1f, fCurrentGradientStep);
        int brightnessStep = SkScalarRoundToInt(SkScalarInterp(32, 64, fCurrentGradientStep));

        SkColor color1Step = SkColorSetARGB(brightnessStep,
                                            brightnessStep,
                                            (brightnessStep << 1),
                                            102);
        SkColor color2Step = SkColorSetARGB((brightnessStep << 1),
                                            (brightnessStep << 1),
                                            brightnessStep,
                                            102);
        SkPoint pts[2] = {
            {fHalfSize.fWidth + dx, fHalfSize.fHeight + dy},
            {fHalfSize.fWidth - dx, fHalfSize.fHeight - dy}
        };
        SkColor colors[] = {
            color1Step,
            color1Step,
            color2Step,
            color2Step
        };
        float pos[] = {
            0,
            0.2f + colorStopStep,
            0.8f - colorStopStep,
            1
        };
        sk_sp<SkShader> gradientShader = SkGradientShader::MakeLinear(pts, colors, pos, 4,
                                                                      SkTileMode::kClamp, 0);

        SkPaint paint;
        paint.setStrokeWidth(15);
        for (int i = 0; i < 4; i++) {
            const SkColor strokeColors[] = {
                0xffe01040, 0xff10c030, 0xff744cba, 0xffe05010
            };
            const SkColor fillColors[] = {
                0xff70051d, 0xff016112, 0xff2F0C6E, 0xff702701
            };
            paint.setColor(strokeColors[i]);
            paint.setStyle(SkPaint::kStroke_Style);
            SkRect arcRect = SkRect::MakeXYWH(fParams.fCircleCenters[i].fX - fParams.fCircleRadius,
                                              fParams.fCircleCenters[i].fY- fParams.fCircleRadius,
                                              2*fParams.fCircleRadius,
                                              2*fParams.fCircleRadius);
            canvas->drawArc(arcRect, 0, 360, false, paint);
            paint.setColor(fillColors[i]);
            paint.setStyle(SkPaint::kFill_Style);
            canvas->drawArc(arcRect, 0, 360, false, paint);
            paint.setShader(gradientShader);
            canvas->drawArc(arcRect, 0, 360, false, paint);
            paint.setShader(nullptr);
        }

        for (size_t i = 0; i < fObjects.size(); ++i) {
            fObjects[i]->draw(canvas);
        }
    }

    bool animate(double nanos) override {
        fCurrentAngle = time_fractional_value(nanos, 3000) * SK_ScalarPI * 2;
        fCurrentGradientStep = 0.5f + 0.5f * sk_float_sin(
                                       time_fractional_value(nanos, 5000) * SK_ScalarPI * 2);

        for (size_t i = 0; i < fObjects.size(); ++i) {
            fObjects[i]->animate(nanos);
        }
        return true;
    }

    bool onChar(SkUnichar uni) override {
        bool handled = false;
        switch (uni) {
            case '+':
            case '=':
                // Add 1000
                for (int i = 0; i < 1000; ++i) {
                    std::unique_ptr<CanvasLineSegment> segment =
                            std::make_unique<CanvasLineSegment>(&fRandom,fParams);
                    fObjects.push_back(std::move(segment));
                }
                handled = true;
                break;
            case '-':
            case '_':
                if (fObjects.size() > 1000) {
                    fObjects.resize(fObjects.size() - 1000);
                }
                handled = true;
                break;
            default:
                break;
        }

        return handled;
    }

private:
    LineSegmentParams fParams;
    SkSize fHalfSize;
    float fTwoFifthsSizeX;
    float fCurrentAngle;
    float fCurrentGradientStep;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class MotionMarkSlide : public Slide {
public:
    MotionMarkSlide() {fName = "MotionMark"; }

    void load(SkScalar w, SkScalar h) override {
        // for now we only have one stage type
        Stage* stage = new CanvasLineSegmentStage(SkSize::Make(w, h));
        fCurrentStage.reset(stage);
    }

    bool onChar(SkUnichar uni) override {
        bool handled = false;
        /* TODO
        switch (uni) {
            default:
                break;
        }
        */
        if (!handled) {
            handled = fCurrentStage->onChar(uni);
        }

        return handled;
    }

    void draw(SkCanvas* canvas) override {
        fCurrentStage->draw(canvas);
    }

    bool animate(double nanos) override {
        return fCurrentStage->animate(nanos);
    }

private:
    std::unique_ptr<Stage> fCurrentStage;
};

DEF_SLIDE( return new MotionMarkSlide(); )
