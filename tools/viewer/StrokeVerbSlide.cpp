/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/core/SkGeometry.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/ClickHandlerSlide.h"

enum class VerbType {
    kTriangles,
    kQuadratics,
    kCubics,
    kConics
};

static const char* verb_type_name(VerbType verbType) {
    switch (verbType) {
        case VerbType::kTriangles: return "kTriangles";
        case VerbType::kQuadratics: return "kQuadratics";
        case VerbType::kCubics: return "kCubics";
        case VerbType::kConics: return "kConics";
    }
    SkUNREACHABLE;
};

/**
 * This sample visualizes simple strokes.
 */
class StrokeVerbSlide : public ClickHandlerSlide {
public:
    StrokeVerbSlide() { fName = "StrokeVerb"; }

    void load(SkScalar w, SkScalar h) override { this->updatePath(); }

    void draw(SkCanvas*) override;

    bool onChar(SkUnichar) override;

protected:
    class Click;

    ClickHandlerSlide::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                                 skui::ModifierKey) override;
    bool onClick(ClickHandlerSlide::Click*) override;


private:
    void updateAndInval() { this->updatePath(); }

    void updatePath();

    VerbType fVerbType = VerbType::kCubics;

    SkPoint fPoints[4] = {
            {100.05f, 100.05f}, {400.75f, 100.05f}, {400.75f, 300.95f}, {100.05f, 300.95f}};

    float fConicWeight = .5;
    float fStrokeWidth = 40;
    SkPaint::Join fStrokeJoin = SkPaint::kMiter_Join;
    SkPaint::Cap fStrokeCap = SkPaint::kButt_Cap;

    SkPath fPath;
};

void StrokeVerbSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    SkPaint outlinePaint;
    outlinePaint.setColor(0xff808080);
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(fStrokeWidth);
    outlinePaint.setStrokeJoin(fStrokeJoin);
    outlinePaint.setStrokeCap(fStrokeCap);
    outlinePaint.setAntiAlias(true);
    canvas->drawPath(fPath, outlinePaint);

    SkString caption;
    caption.appendf("VerbType_%s", verb_type_name(fVerbType));
    if (VerbType::kCubics == fVerbType) {
        caption.appendf(" (%s)", SkCubicTypeName(SkClassifyCubic(fPoints)));
    } else if (VerbType::kConics == fVerbType) {
        caption.appendf(" (w=%f)", fConicWeight);
    }

    caption.appendf(" (stroke_width=%f)", fStrokeWidth);

    SkPaint pointsPaint;
    pointsPaint.setColor(SK_ColorBLUE);
    pointsPaint.setStrokeWidth(8);
    pointsPaint.setAntiAlias(true);

    if (VerbType::kCubics == fVerbType) {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, fPoints, pointsPaint);
    } else {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, fPoints, pointsPaint);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 1, fPoints + 3, pointsPaint);
    }

    SkFont font(ToolUtils::DefaultTypeface(), 20);
    SkPaint captionPaint;
    captionPaint.setColor(SK_ColorWHITE);
    canvas->drawString(caption, 10, 30, font, captionPaint);
}

void StrokeVerbSlide::updatePath() {
    fPath.reset();
    fPath.moveTo(fPoints[0]);
    switch (fVerbType) {
        case VerbType::kCubics:
            fPath.cubicTo(fPoints[1], fPoints[2], fPoints[3]);
            break;
        case VerbType::kQuadratics:
            fPath.quadTo(fPoints[1], fPoints[3]);
            break;
        case VerbType::kConics:
            fPath.conicTo(fPoints[1], fPoints[3], fConicWeight);
            break;
        case VerbType::kTriangles:
            fPath.lineTo(fPoints[1]);
            fPath.lineTo(fPoints[3]);
            fPath.close();
            break;
    }
}

class StrokeVerbSlide::Click : public ClickHandlerSlide::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SkPoint points[]) {
        if (fPtIdx >= 0) {
            points[fPtIdx] += fCurr - fPrev;
        } else {
            for (int i = 0; i < 4; ++i) {
                points[i] += fCurr - fPrev;
            }
        }
    }

private:
    int fPtIdx;
};

ClickHandlerSlide::Click* StrokeVerbSlide::onFindClickHandler(SkScalar x, SkScalar y,
                                                              skui::ModifierKey) {
    for (int i = 0; i < 4; ++i) {
        if (VerbType::kCubics != fVerbType && 2 == i) {
            continue;
        }
        if (fabs(x - fPoints[i].x()) < 20 && fabsf(y - fPoints[i].y()) < 20) {
            return new Click(i);
        }
    }
    return new Click(-1);
}

bool StrokeVerbSlide::onClick(ClickHandlerSlide::Click* click) {
    Click* myClick = (Click*)click;
    myClick->doClick(fPoints);
    this->updateAndInval();
    return true;
}

bool StrokeVerbSlide::onChar(SkUnichar unichar) {
        if (unichar >= '1' && unichar <= '4') {
            fVerbType = VerbType(unichar - '1');
            this->updateAndInval();
            return true;
        }
        float* valueToScale = nullptr;
        if (VerbType::kConics == fVerbType) {
            valueToScale = &fConicWeight;
        } else {
            valueToScale = &fStrokeWidth;
        }
        if (valueToScale) {
            if (unichar == '+') {
                *valueToScale *= 2;
                this->updateAndInval();
                return true;
            }
            if (unichar == '=') {
                *valueToScale *= 5/4.f;
                this->updateAndInval();
                return true;
            }
            if (unichar == '-') {
                *valueToScale *= 4/5.f;
                this->updateAndInval();
                return true;
            }
            if (unichar == '_') {
                *valueToScale *= .5f;
                this->updateAndInval();
                return true;
            }
        }
        if (unichar == 'D') {
            SkDebugf("    SkPoint fPoints[4] = {\n");
            SkDebugf("        {%ff, %ff},\n", fPoints[0].x(), fPoints[0].y());
            SkDebugf("        {%ff, %ff},\n", fPoints[1].x(), fPoints[1].y());
            SkDebugf("        {%ff, %ff},\n", fPoints[2].x(), fPoints[2].y());
            SkDebugf("        {%ff, %ff}\n", fPoints[3].x(), fPoints[3].y());
            SkDebugf("    };\n");
            return true;
        }
        if (unichar == 'J') {
            fStrokeJoin = (SkPaint::Join)((fStrokeJoin + 1) % 3);
            this->updateAndInval();
            return true;
        }
        if (unichar == 'C') {
            fStrokeCap = (SkPaint::Cap)((fStrokeCap + 1) % 3);
            this->updateAndInval();
            return true;
        }
        return false;
}

DEF_SLIDE(return new StrokeVerbSlide;)

#endif  // defined(SK_GANESH)
