/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPath.h"

#include <iostream>
#include <cmath>

#define PI SK_ScalarPI

#define LIN_SEGMENTS 10

class OverstrokeView : public SampleView {
   public:
    SkScalar fStroke;
    int fPathType;  // super lazy enum
    bool fClosePath;
    bool fDrawFillPath;
    bool fDumpHex;
    OverstrokeView() {
        fStroke = 5;
        fPathType = 0;
        fClosePath = false;
        fDrawFillPath = false;
        fDumpHex = false;
        this->setBGColor(0xFFFFFFFF);
    }

   protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathOverstroke");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case ',':
                    fStroke += 1.0;
                    this->inval(nullptr);
                    return true;
                case '.':
                    fStroke -= 1.0;
                    this->inval(nullptr);
                    return true;
                case 'x':
                    fPathType = (fPathType + 1) % 4;
                    this->inval(nullptr);
                    return true;
                case 'c':
                    fClosePath = !fClosePath;
                    this->inval(nullptr);
                    return true;
                case 'f':
                    fDrawFillPath = !fDrawFillPath;
                    this->inval(nullptr);
                    return true;
                case 'D':
                    fDumpHex = !fDumpHex;
                    this->inval(nullptr);
                    return true;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    SkPath quadPath(SkPoint p1, SkPoint p2) {
        SkASSERT(p1.y() == p2.y());

        SkPath path;
        path.moveTo(p1);
        path.lineTo(p2);

        SkPoint p3 = SkPoint::Make((p1.x() + p2.x()) / 2.0f, p1.y() * 0.7f);

        path.quadTo(p3, p1);

        return path;
    }

    SkPath cubicPath(SkPoint p1, SkPoint p2) {
        SkASSERT(p1.y() == p2.y());

        SkPath path;
        path.moveTo(p1);

        SkPoint p3 = SkPoint::Make((p1.x() + p2.x()) / 3.0f, p1.y() * 0.7f);
        SkPoint p4 = SkPoint::Make(2.0f*(p1.x() + p2.x()) / 3.0f, p1.y() * 1.5f);

        path.cubicTo(p3, p4, p2);

        return path;
    }

    SkPath linSemicirclePath(SkPoint p1, SkPoint p2) {
        SkASSERT(p1.y() == p2.y());

        SkPath path;
        path.moveTo(p1);
        path.lineTo(p2);

        SkPoint pt;

        for (int i = 0; i < LIN_SEGMENTS; i++) {
            float theta = i * PI / (LIN_SEGMENTS);
            SkScalar x = 65 + 15 * cos(theta);
            SkScalar y = 50 - 15 * sin(theta);
            pt = SkPoint::Make(x, y);
            path.lineTo(pt);
        }
        path.lineTo(p1);

        return path;
    }

    SkPath rectPath(SkPoint p1) {
        SkRect r = SkRect::MakeXYWH(p1.fX, p1.fY, 20, 20);
        SkPath path;
        path.addRect(r);

        return path;
    }

    void onDrawContent(SkCanvas* canvas) override {
        const float SCALE = 1;

        canvas->translate(30, 40);
        canvas->scale(SCALE, SCALE);

        SkPoint p1 = SkPoint::Make(50, 50);
        SkPoint p2 = SkPoint::Make(80, 50);

        SkPath path;
        switch (fPathType) {
            case 0:
                path = quadPath(p1, p2);
                break;
            case 1:
                path = cubicPath(p1, p2);
                break;
            case 2:
                path = rectPath(p1);
                break;
            case 3:
                path = linSemicirclePath(p1, p2);
                break;
            default:
                path = quadPath(p1, p2);
                break;
        }

        if (fClosePath) {
            path.close();
        }

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(fStroke);

        canvas->drawPath(path, p);

        if (fDumpHex) {
            std::cerr << "path dumpHex" << std::endl;
            path.dumpHex();
        }

        SkPaint hairp;
        hairp.setColor(SK_ColorBLACK);
        hairp.setAntiAlias(true);
        hairp.setStyle(SkPaint::kStroke_Style);

        if (fDrawFillPath) {
            SkPath fillpath;
            p.getFillPath(path, &fillpath);

            canvas->drawPath(fillpath, hairp);

            if (fDumpHex) {
                std::cerr << "fillpath dumpHex" << std::endl;
                fillpath.dumpHex();
            }
        }

        if (fDumpHex) {
            std::cerr << std::endl;

            fDumpHex = false;
        }

        // draw original path with green hairline
        hairp.setColor(SK_ColorGREEN);
        canvas->drawPath(path, hairp);
    }

   private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new OverstrokeView; }
static SkViewRegister reg(MyFactory);
