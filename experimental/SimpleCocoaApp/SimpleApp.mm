/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvas.h"
#include "SkCGUtils.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkWindow.h"

static void make_filepath(SkString* path, const char* dir, const SkString& name) {
    size_t len = strlen(dir);
    path->set(dir);
    if (len > 0 && dir[len - 1] != '/') {
        path->append("/");
    }
    path->append(name);
}

static SkPicture* LoadPicture(const char path[]) {
    SkPicture* pic = NULL;

    SkBitmap bm;
    if (SkImageDecoder::DecodeFile(path, &bm)) {
        bm.setImmutable();
        pic = new SkPicture;
        SkCanvas* can = pic->beginRecording(bm.width(), bm.height());
        can->drawBitmap(bm, 0, 0, NULL);
        pic->endRecording();
    } else {
        SkFILEStream stream(path);
        if (stream.isValid()) {
            pic = new SkPicture(&stream, NULL, &SkImageDecoder::DecodeStream);
        }

        if (false) { // re-record
            SkPicture p2;
            pic->draw(p2.beginRecording(pic->width(), pic->height()));
            p2.endRecording();

            SkString path2(path);
            path2.append(".new.skp");
            SkFILEWStream writer(path2.c_str());
            p2.serialize(&writer);
        }
    }
    return pic;
}

class SkSampleView : public SkView {
public:
    SkSampleView() {
        this->setVisibleP(true);
        this->setClipToBounds(false);
    };
protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(0xFFFFFFFF);
        SkPaint p;
        p.setTextSize(20);
        p.setAntiAlias(true);
        canvas->drawText("Hello World!", 13, 50, 30, p);
     //   SkRect r = {50, 50, 80, 80};
        p.setColor(0xAA11EEAA);
   //     canvas->drawRect(r, p);

        SkRect result;
        SkPath path;
        path.moveTo(0, 0);
        path.lineTo(1, 1);
        path.lineTo(1, 8);
        path.lineTo(0, 9);
        SkASSERT(path.hasRectangularInterior(&result));

        path.reset();
        path.addRect(10, 10, 100, 100, SkPath::kCW_Direction);
        path.addRect(20, 20, 50, 50, SkPath::kCW_Direction);
        path.addRect(50, 50, 90, 90, SkPath::kCCW_Direction);
        p.setColor(0xAA335577);
        canvas->drawPath(path, p);
        SkASSERT(!path.hasRectangularInterior(NULL));
        path.reset();
        path.addRect(10, 10, 100, 100, SkPath::kCW_Direction);
        path.addRect(20, 20, 80, 80, SkPath::kCW_Direction);
        SkRect expected = {20, 20, 80, 80};
        SkASSERT(path.hasRectangularInterior(&result));
        SkASSERT(result == expected);

    }
private:
    typedef SkView INHERITED;
};

void application_init();
void application_term();

static int showPathContour(SkPath::Iter& iter) {
    uint8_t verb;
    SkPoint pts[4];
    int moves = 0;
    bool waitForClose = false;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (!waitForClose) {
                    ++moves;
                    waitForClose = true;
                }
                SkDebugf("path.moveTo(%1.9g, %1.9g);\n", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kLine_Verb:
                SkDebugf("path.lineTo(%1.9g, %1.9g);\n", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("path.quadTo(%1.9g, %1.9g, %1.9g, %1.9g);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("path.cubicTo(%1.9g, %1.9g, %1.9g, %1.9g, %1.9g, %1.9g);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                    pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                waitForClose = false;
                SkDebugf("path.close();\n");
                break;
            default:
                SkDEBUGFAIL("bad verb");
                SkASSERT(0);
                return 0;
        }
    }
    return moves;
}

class PathCanvas : public SkCanvas {
    virtual void drawPath(const SkPath& path, const SkPaint& paint) {
        if (nameonly) {
            SkDebugf("    %s%d,\n", filename.c_str(), ++count);
            return;
        }
        SkPath::Iter iter(path, true);
        SkDebugf("<div id=\"%s%d\">\n", filename.c_str(), ++count);
        SkASSERT(path.getFillType() < SkPath::kInverseWinding_FillType);
        SkDebugf("path.setFillType(SkPath::k%s_FillType);\n",
            path.getFillType() == SkPath::kWinding_FillType ? "Winding" : "EvenOdd");
        int contours = showPathContour(iter);
        SkRect r;
        SkRect copy = r;
        bool hasOne = path.hasRectangularInterior(&r);
        bool expected = (path.getFillType() == SkPath::kWinding_FillType && contours == 1)
            || (path.getFillType() == SkPath::kEvenOdd_FillType && contours == 2);
        if (!expected) {
            SkDebugf("suspect contours=%d\n", contours);
        }
        int verbs = path.countVerbs();
        int points = path.countPoints();
        if (hasOne) {
            if (rectVerbsMin > verbs) {
                rectVerbsMin = verbs;
            }
            if (rectVerbsMax < verbs) {
                rectVerbsMax = verbs;
            }
            if (rectPointsMin > points) {
                rectPointsMin = points;
            }
            if (rectPointsMax < points) {
                rectPointsMax = points;
            }
            SkDebugf("path.addRect(%1.9g, %1.9g, %1.9g, %1.9g);\n",
                    r.fLeft, r.fTop, r.fRight, r.fBottom);
        } else {
            if (verbsMin > verbs) {
                verbsMin = verbs;
            }
            if (verbsMax < verbs) {
                verbsMax = verbs;
            }
            if (pointsMin > points) {
                pointsMin = points;
            }
            if (pointsMax < points) {
                pointsMax = points;
            }
            SkDebugf("no interior bounds\n");
        }
        path.hasRectangularInterior(&copy);
        SkDebugf("</div>\n\n");
    }

    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) {
    }

public:
    void divName(const SkString& str, bool only) {
        filename = str;
        char* chars = filename.writable_str();
        while (*chars) {
            if (*chars == '.' || *chars == '-') *chars = '_';
            chars++;
        }
        count = 0;
        nameonly = only;
    }

    void init() {
        pointsMin = verbsMin = SK_MaxS32;
        pointsMax = verbsMax = SK_MinS32;
        rectPointsMin = rectVerbsMin = SK_MaxS32;
        rectPointsMax = rectVerbsMax = SK_MinS32;
    }

    SkString filename;
    int count;
    bool nameonly;
    int pointsMin;
    int pointsMax;
    int verbsMin;
    int verbsMax;
    int rectPointsMin;
    int rectPointsMax;
    int rectVerbsMin;
    int rectVerbsMax;
};

bool runone = false;

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
    if (runone) {
        return;
    }
    const char pictDir[] = "/Volumes/chrome/nih/skia/skp/skp";
    SkOSFile::Iter iter(pictDir, "skp");
    SkString filename;
    PathCanvas canvas;
    canvas.init();
    while (iter.next(&filename)) {
        SkString path;
     //   if (true) filename.set("tabl_www_sahadan_com.skp");
        make_filepath(&path, pictDir, filename);
        canvas.divName(filename, false);
        SkPicture* pic = LoadPicture(path.c_str());
        pic->draw(&canvas);
        delete pic;
    }
    SkDebugf("\n</div>\n\n");

    SkDebugf("<script type=\"text/javascript\">\n\n");
    SkDebugf("var testDivs = [\n");

    iter.reset(pictDir, "skp");
    while (iter.next(&filename)) {
        SkString path;
        make_filepath(&path, pictDir, filename);
        canvas.divName(filename, true);
        SkPicture* pic = LoadPicture(path.c_str());
        pic->draw(&canvas);
        delete pic;
    }
    SkDebugf("];\n\n");

    SkDebugf("points min=%d max=%d verbs min=%d max=%d\n", canvas.pointsMin, canvas.pointsMax,
            canvas.verbsMin, canvas.verbsMax);
    SkDebugf("rect points min=%d max=%d verbs min=%d max=%d\n", canvas.rectPointsMin, canvas.rectPointsMax,
            canvas.rectVerbsMin, canvas.rectVerbsMax);

    SkDebugf("\n");
}

void application_term() {
    SkEvent::Term();
}

class FillLayout : public SkView::Layout {
protected:
    virtual void onLayoutChildren(SkView* parent) {
        SkView* view = SkView::F2BIter(parent).next();
        view->setSize(parent->width(), parent->height());
    }
};

#import "SimpleApp.h"
@implementation SimpleNSView

- (id)initWithDefaults {
    if ((self = [super initWithDefaults])) {
        fWind = new SkOSWindow(self);
        fWind->setLayout(new FillLayout, false);
        fWind->attachChildToFront(new SkSampleView)->unref();
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    SkCGDrawBitmap(ctx, fWind->getBitmap(), 0, 0);
}

@end
