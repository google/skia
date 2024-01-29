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
private:
    static const int kStartingObjectCount = 5000;
    static const int kObjectIncrement = 1000;

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

        for (int i = 0; i < kStartingObjectCount; ++i) {
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
                for (int i = 0; i < kObjectIncrement; ++i) {
                    std::unique_ptr<CanvasLineSegment> segment =
                            std::make_unique<CanvasLineSegment>(&fRandom,fParams);
                    fObjects.push_back(std::move(segment));
                }
                handled = true;
                break;
            case '-':
            case '_':
                if (fObjects.size() > kObjectIncrement) {
                    fObjects.resize(fObjects.size() - kObjectIncrement);
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
    float fCurrentAngle = 0;
    float fCurrentGradientStep = 0.5f;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Canvas Arcs
///////////////////////////////////////////////////////////////////////////////////////////////////

class CanvasArc : public MMObject {
public:
    CanvasArc(SkRandom* random, SkSize size) {
        constexpr float kMaxX = 6;
        constexpr float kMaxY = 3;

        const SkColor baseColors[3] = {
            0xff101010, 0xff808080, 0xffc0c0c0
        };
        const SkColor bonusColors[3] = {
            0xffe01040, 0xff10c030, 0xffe05010
        };
        float distanceX = size.fWidth / kMaxX;
        float distanceY = size.fHeight / (kMaxY + 1);
        int randY = random->nextRangeU(0, kMaxY);
        int randX = random->nextRangeU(0, kMaxX - 1 * (randY % 2));

        fPoint = SkPoint::Make(distanceX * (randX + (randY % 2) / 2), distanceY * (randY + 0.5f));

        fRadius = 20 + sk_float_pow(random->nextF(), 5) * (std::min(distanceX, distanceY) / 1.8f);
        fStartAngle = random->nextRangeF(0, 2*SK_ScalarPI);
        fEndAngle = random->nextRangeF(0, 2*SK_ScalarPI);
        fOmega = (random->nextF() - 0.5f) * 0.3f;
        fCounterclockwise = random->nextBool();
        // The MotionMark code appends a random element from an array and appends it to the color
        // array, then randomly picks from that. We'll just pick that random element and use it
        // if the index is out of bounds for the base color array.
        SkColor bonusColor = bonusColors[(randX + sk_float_ceil2int(randY * 0.5f)) % 3];
        int colorIndex = random->nextRangeU(0, 3);
        fColor = colorIndex == 3 ? bonusColor : baseColors[colorIndex];
        fLineWidth = 1 + sk_float_pow(random->nextF(), 5) * 30;
        fDoStroke = random->nextRangeU(0, 3) != 0;
    }

    ~CanvasArc() override = default;

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(fColor);
        SkRect arcRect = SkRect::MakeXYWH(fPoint.fX - fRadius, fPoint.fY - fRadius,
                                          2*fRadius, 2*fRadius);
        float startAngleDeg = fStartAngle * 180.f/SK_ScalarPI;
        float endAngleDeg = fEndAngle * 180.f/SK_ScalarPI;
        float sweepAngle = (endAngleDeg > startAngleDeg) ? endAngleDeg - startAngleDeg
                                                         : 360 - (startAngleDeg - endAngleDeg);
        if (fCounterclockwise) {
            sweepAngle = 360 - sweepAngle;
        }

        if (fDoStroke) {
            paint.setStrokeWidth(fLineWidth);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawArc(arcRect, startAngleDeg, sweepAngle, false, paint);
        } else {
            paint.setStyle(SkPaint::kFill_Style);
            // The MotionMark code creates a path for fills via lineTo(point), arc(), lineTo(point).
            // For now we'll just use drawArc for both but might need to revisit.
            canvas->drawArc(arcRect, startAngleDeg, sweepAngle, true, paint);
        }
    }

    void animate(double /*nanos*/) override {
        fStartAngle += fOmega;
        fEndAngle += fOmega / 2;
    }

private:
    SkPoint fPoint;
    float fRadius;
    float fStartAngle; // in radians
    float fEndAngle;   // in radians
    SkColor fColor;
    float fOmega;      // in radians
    bool fDoStroke;
    bool fCounterclockwise;
    float fLineWidth;
};

class CanvasArcStage : public Stage {
private:
    static const int kStartingObjectCount = 1000;
    static const int kObjectIncrement = 200;

public:
    CanvasArcStage(SkSize size) : Stage(size) {
        for (int i = 0; i < kStartingObjectCount; ++i) {
            std::unique_ptr<CanvasArc> arc = std::make_unique<CanvasArc>(&fRandom, fSize);
            fObjects.push_back(std::move(arc));
        }
    }

    ~CanvasArcStage() override = default;

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        for (size_t i = 0; i < fObjects.size(); ++i) {
            fObjects[i]->draw(canvas);
        }
    }

    bool animate(double nanos) override {
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
                for (int i = 0; i < kObjectIncrement; ++i) {
                    std::unique_ptr<CanvasArc> arc = std::make_unique<CanvasArc>(&fRandom, fSize);
                    fObjects.push_back(std::move(arc));
                }
                handled = true;
                break;
            case '-':
            case '_':
                if (fObjects.size() > kObjectIncrement) {
                    fObjects.resize(fObjects.size() - kObjectIncrement);
                }
                handled = true;
                break;
            default:
                break;
        }

        return handled;
    }

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Paths
///////////////////////////////////////////////////////////////////////////////////////////////////

class CanvasLinePoint : public MMObject {
protected:
    void setEndPoint(SkRandom* random, SkSize size, SkPoint* prevCoord) {
        const SkSize kGridSize = { 80, 40 };
        const SkPoint kGridCenter = { 40, 20 };
        const SkPoint kOffsets[4] = {
            {-4, 0},
            {2, 0},
            {1, -2},
            {1, 2}
        };

        SkPoint coordinate = prevCoord ? *prevCoord : kGridCenter;
        if (prevCoord) {
            SkPoint offset = kOffsets[random->nextRangeU(0, 3)];
            coordinate += offset;
            if (coordinate.fX < 0 || coordinate.fX > kGridSize.width())
                coordinate.fX -= offset.fX * 2;
            if (coordinate.fY < 0 || coordinate.fY > kGridSize.height())
                coordinate.fY -= offset.fY * 2;
        }

        fPoint = SkPoint::Make((coordinate.fX + 0.5f) * size.width() / (kGridSize.width() + 1),
                               (coordinate.fY + 0.5f) * size.height() / (kGridSize.height() + 1));
        fCoordinate = coordinate;
    }

public:
    CanvasLinePoint(SkRandom* random, SkSize size, SkPoint* prev) {
        const SkColor kColors[7] = {
            0xff101010, 0xff808080, 0xffc0c0c0, 0xff101010, 0xff808080, 0xffc0c0c0, 0xffe01040
        };
        fColor = kColors[random->nextRangeU(0, 6)];

        fWidth = sk_float_pow(random->nextF(), 5) * 20 + 1;
        fIsSplit = random->nextBool();

        this->setEndPoint(random, size, prev);
    }

    ~CanvasLinePoint() override = default;

    virtual void append(SkPath* path) {
        path->lineTo(fPoint);
    }

    // unused, all the work is done by append
    void draw(SkCanvas*) override {}
    void animate(double) override {}

    SkColor getColor() { return fColor; }
    float getWidth() { return fWidth; }
    SkPoint getPoint() { return fPoint; }
    SkPoint getCoord() { return fCoordinate; }
    bool isSplit() { return fIsSplit; }
    void toggleIsSplit() { fIsSplit = !fIsSplit; }

private:
    SkPoint fPoint;
    SkPoint fCoordinate;
    SkColor fColor;
    float fWidth;
    bool fIsSplit;
};

class CanvasQuadraticSegment : public CanvasLinePoint {
public:
    CanvasQuadraticSegment(SkRandom* random, SkSize size, SkPoint* prev)
            : CanvasLinePoint(random, size, prev) {
        // Note: The construction of these points is odd but mirrors the Javascript code.

        // The chosen point from the base constructor is instead the control point.
        fPoint2 = this->getPoint();

        // Get another random point for the actual end point of the segment.
        this->setEndPoint(random, size, prev);
    }

    void append(SkPath* path) override {
        path->quadTo(fPoint2, this->getPoint());
    }

private:
    SkPoint fPoint2;
};

class CanvasBezierSegment : public CanvasLinePoint {
public:
    CanvasBezierSegment(SkRandom* random, SkSize size, SkPoint* prev)
            : CanvasLinePoint(random, size, prev) {
        // Note: The construction of these points is odd but mirrors the Javascript code.

        // The chosen point from the base constructor is instead the control point.
        fPoint2 = this->getPoint();

        // Get the second control point.
        this->setEndPoint(random, size, prev);
        fPoint3 = this->getPoint();

        // Get third random point for the actual end point of the segment.
        this->setEndPoint(random, size, prev);
    }

    void append(SkPath* path) override {
        path->cubicTo(fPoint2, fPoint3, this->getPoint());
    }

private:
    SkPoint fPoint2;
    SkPoint fPoint3;
};


std::unique_ptr<CanvasLinePoint> make_line_path(SkRandom* random, SkSize size, SkPoint* prev) {
    int choice = random->nextRangeU(0, 3);
    switch (choice) {
        case 0:
            return std::make_unique<CanvasQuadraticSegment>(random, size, prev);
            break;
        case 1:
            return std::make_unique<CanvasBezierSegment>(random, size, prev);
            break;
        case 2:
        case 3:
        default:
            return std::make_unique<CanvasLinePoint>(random, size, prev);
            break;
    }
}

class CanvasLinePathStage : public Stage {
private:
    static constexpr int kStartingObjectCount = 5000;
    static constexpr int kObjectIncrement = 1000;

public:
    CanvasLinePathStage(SkSize size) : Stage(size) {
        SkPoint prevCoord;
        for (int i = 0; i < kStartingObjectCount; ++i) {
            std::unique_ptr<CanvasLinePoint> obj;
            if (i > 0) {
                obj = make_line_path(&fRandom, fSize, &prevCoord);
            } else {
                obj = make_line_path(&fRandom, fSize, nullptr);
            }
            prevCoord = obj->getCoord();
            fObjects.push_back(std::move(obj));
        }
    }

    ~CanvasLinePathStage() override = default;

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        SkPath currentPath;
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        for (size_t i = 0; i < fObjects.size(); ++i) {
            CanvasLinePoint* object = reinterpret_cast<CanvasLinePoint*>(fObjects[i].get());
            if (i == 0) {
                paint.setStrokeWidth(object->getWidth());
                paint.setColor(object->getColor());
                currentPath.moveTo(object->getPoint());
            } else {
                object->append(&currentPath);

                if (object->isSplit()) {
                    canvas->drawPath(currentPath, paint);

                    paint.setStrokeWidth(object->getWidth());
                    paint.setColor(object->getColor());
                    currentPath.reset();
                    currentPath.moveTo(object->getPoint());
                }

                if (fRandom.nextF() > 0.995) {
                    object->toggleIsSplit();
                }
            }
        }
        canvas->drawPath(currentPath, paint);
    }

    bool animate(double /*nanos*/) override {
        // Nothing to do, but return true so we redraw.
        return true;
    }

    bool onChar(SkUnichar uni) override {
        bool handled = false;
        switch (uni) {
            case '+':
            case '=':
                for (int i = 0; i < kObjectIncrement; ++i) {
                    CanvasLinePoint* prevObject =
                        reinterpret_cast<CanvasLinePoint*>(fObjects[fObjects.size()-1].get());
                    SkPoint coord = prevObject->getCoord();
                    std::unique_ptr<CanvasLinePoint> path = make_line_path(&fRandom, fSize, &coord);
                    fObjects.push_back(std::move(path));
                }
                handled = true;
                break;
            case '-':
            case '_':
                if (fObjects.size() > kObjectIncrement) {
                    fObjects.resize(fObjects.size() - kObjectIncrement);
                }
                handled = true;
                break;
            default:
                break;
        }

        return handled;
    }

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class MotionMarkSlide : public Slide {
public:
    MotionMarkSlide() {fName = "MotionMark"; }

    void load(SkScalar w, SkScalar h) override {
        SkSize size = SkSize::Make(w, h);

        std::unique_ptr<CanvasLineSegmentStage> lineStage =
                std::make_unique<CanvasLineSegmentStage>(size);
        fStages.push_back(std::move(lineStage));

        std::unique_ptr<CanvasArcStage> arcStage =
                std::make_unique<CanvasArcStage>(size);
        fStages.push_back(std::move(arcStage));

        std::unique_ptr<CanvasLinePathStage> pathStage =
                std::make_unique<CanvasLinePathStage>(size);
        fStages.push_back(std::move(pathStage));
    }

    bool onChar(SkUnichar uni) override {
        bool handled = false;
        switch (uni) {
            case '<':
            case ',':
            case '4':
                fCurrentStage = (fCurrentStage == 0) ? fStages.size() - 1
                                                     : (fCurrentStage - 1) % fStages.size();
                handled = true;
                break;
            case '>':
            case '.':
            case '6':
                fCurrentStage = (fCurrentStage + 1) % fStages.size();
                handled = true;
                break;
            default:
                break;
        }
        if (!handled) {
            handled = fStages[fCurrentStage]->onChar(uni);
        }

        return handled;
    }

    void draw(SkCanvas* canvas) override {
        fStages[fCurrentStage]->draw(canvas);
    }

    bool animate(double nanos) override {
        return fStages[fCurrentStage]->animate(nanos);
    }

private:
    std::vector<std::unique_ptr<Stage>> fStages;
    int fCurrentStage = 0;
};

DEF_SLIDE( return new MotionMarkSlide(); )
