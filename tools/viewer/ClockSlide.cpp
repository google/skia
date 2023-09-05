/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "src/base/SkRandom.h"
#include "src/pdf/SkPDFUtils.h"
#include "tools/viewer/Slide.h"

// Implementation in C++ of Mozilla Canvas2D benchmark Canvas Clock Test
// See https://code.google.com/p/skia/issues/detail?id=1626

#define USE_PATH 1

class ClockSlide : public Slide {
public:
    ClockSlide() { fName = "Clock"; }

    void draw(SkCanvas* canvas) override {
        SkPaint paintFill;
        SkPaint paintStroke;
        SkPath  path;

        canvas->save();
        canvas->translate(150, 150);
        canvas->scale(0.4f, 0.4f);
        canvas->rotate(-180.f/2.f);

        paintFill.setAntiAlias(true);
        paintFill.setColor(SK_ColorBLACK);
        paintStroke.setAntiAlias(true);
        paintStroke.setStyle(SkPaint::kStroke_Style);
        paintStroke.setColor(SK_ColorBLACK);
        paintStroke.setStrokeWidth(8);
        paintStroke.setStrokeCap(SkPaint::kRound_Cap);

        // Hour marks
        SkRect rect;
#ifndef USE_PATH
        rect = SkRect::MakeLTRB(200-4, -4, 240+4, 4);
        SkRRect rrect;
        SkVector radii[4] = {{4,4}, {4,4}, {4,4}, {4,4}};
        rrect.setRectRadii(rect, radii);
#endif
        canvas->save();
        for (int i=0;i<12;i++){
            canvas->rotate(180.f/6.f);
#ifdef USE_PATH
            path.reset();
            path.moveTo(200,0);
            path.lineTo(240,0);
            canvas->drawPath(path, paintStroke);
#else
            canvas->drawRRect(rrect, paintFill);
#endif
        }
        canvas->restore();

        // Minute marks
        canvas->save();
#ifdef USE_PATH
        paintStroke.setStrokeWidth(5);
#else
        rect = SkRect::MakeLTRB(231.5f, -2.5f, 242.5, 2.5f);
        radii[0] = SkPoint::Make(2.5f,2.5f);
        radii[1] = SkPoint::Make(2.5f,2.5f);
        radii[2] = SkPoint::Make(2.5f,2.5f);
        radii[3] = SkPoint::Make(2.5f,2.5f);
        rrect.setRectRadii(rect, radii);
#endif
        for (int i=0;i<60;i++){
            if (i%5 == 0) {
                canvas->rotate(180.f/30.f);
                continue;
            }
#ifdef USE_PATH
            path.reset();
            path.moveTo(234,0);
            path.lineTo(240,0);
            canvas->drawPath(path, paintStroke);
#else
            canvas->drawRRect(rrect, paintFill);
#endif
            canvas->rotate(180.f/30.f);
        }
        canvas->restore();

        // TODO(herb,kjlubick) Switch to std::chrono::system_clock
        SkPDF::DateTime time;
        SkPDFUtils::GetDateTime(&time);
        time.fHour = time.fHour >= 12 ? time.fHour-12 : time.fHour;
        paintFill.setColor(SK_ColorBLACK);

        // Write hours
        canvas->save();
        canvas->rotate(time.fHour*(180.f/6.f) + time.fMinute*(180.f/360.f)
                       + time.fSecond*(180.f/21600.f) );
#ifdef USE_PATH
        paintStroke.setStrokeWidth(14);
        path.reset();
        path.moveTo(-20,0);
        path.lineTo(80,0);
        canvas->drawPath(path, paintStroke);
#else
        rect = SkRect::MakeLTRB(-20-7, -7, 80+7, 7);
        radii[0] = SkPoint::Make(7,7);
        radii[1] = SkPoint::Make(7,7);
        radii[2] = SkPoint::Make(7,7);
        radii[3] = SkPoint::Make(7,7);
        rrect.setRectRadii(rect, radii);
        canvas->drawRRect(rrect, paintFill);
#endif
        canvas->restore();

        // Write minutes
        canvas->save();
        canvas->rotate(time.fMinute*(180.f/30.f)
                       + time.fSecond*(180.f/1800.f) );
#ifdef USE_PATH
        paintStroke.setStrokeWidth(10);
        path.reset();
        path.moveTo(-56,0);
        path.lineTo(224,0);
        canvas->drawPath(path, paintStroke);
#else
        rect = SkRect::MakeLTRB(-56-5, -5, 224+5, 5);
        radii[0] = SkPoint::Make(5,5);
        radii[1] = SkPoint::Make(5,5);
        radii[2] = SkPoint::Make(5,5);
        radii[3] = SkPoint::Make(5,5);
        rrect.setRectRadii(rect, radii);
        canvas->drawRRect(rrect, paintFill);
#endif
        canvas->restore();

        // Write seconds
        canvas->save();
        canvas->rotate(time.fSecond*(180.f/30.f));
        paintFill.setColor(0xffd40000);
        paintStroke.setColor(0xffd40000);
        paintStroke.setStrokeWidth(6);
#ifdef USE_PATH
        path.reset();
        path.moveTo(-60,0);
        path.lineTo(166,0);
        canvas->drawPath(path, paintStroke);
#else
        rect = SkRect::MakeLTRB(-60-3, -3, 166+3, 3);
        radii[0] = SkPoint::Make(3,3);
        radii[1] = SkPoint::Make(3,3);
        radii[2] = SkPoint::Make(3,3);
        radii[3] = SkPoint::Make(3,3);
        rrect.setRectRadii(rect, radii);
        canvas->drawRRect(rrect, paintFill);
#endif
        rect = SkRect::MakeLTRB(-20, -20, 20, 20);
#ifdef USE_PATH
        path.reset();
        path.arcTo(rect, 0, 0, false);
        path.addOval(rect, SkPathDirection::kCCW);
        path.arcTo(rect, 360, 0, true);
        canvas->drawPath(path, paintFill);
#else
        canvas->drawOval(rect, paintFill);
#endif
        rect = SkRect::MakeLTRB(-20+190, -20, 20+190, 20);
#ifdef USE_PATH
        path.reset();
        path.arcTo(rect, 0, 0, false);
        path.addOval(rect, SkPathDirection::kCCW);
        path.arcTo(rect, 360, 0, true);
        canvas->drawPath(path, paintStroke);
#else
        canvas->drawOval(rect, paintStroke);
#endif
        paintFill.setColor(0xff505050);
#ifdef USE_PATH
        rect = SkRect::MakeLTRB(-6, -6, 6, 6);
        path.arcTo(rect, 0, 0, false);
        path.addOval(rect, SkPathDirection::kCCW);
        path.arcTo(rect, 360, 0, true);
        canvas->drawPath(path, paintFill);
#else
        canvas->drawOval(rect, paintFill);
        rect = SkRect::MakeLTRB(-6, -6, 6, 6);
        canvas->drawOval(rect, paintFill);
#endif
        canvas->restore();

        paintStroke.setStrokeWidth(18);
        paintStroke.setColor(0xff325FA2);
        rect = SkRect::MakeLTRB(-284, -284, 284, 284);
#ifdef USE_PATH
        path.reset();
        path.arcTo(rect, 0, 0, false);
        path.addOval(rect, SkPathDirection::kCCW);
        path.arcTo(rect, 360, 0, true);
        canvas->drawPath(path, paintStroke);
#else
        canvas->drawOval(rect, paintStroke);
#endif

        canvas->restore();
    }

    bool animate(double /*nanos*/) override { return true; }
};

DEF_SLIDE( return new ClockSlide(); )
