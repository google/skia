
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"

#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkShader.h"

#define WIRE_FRAME_WIDTH    1.1f

static void tesselate(const SkPath& src, SkPath* dst) {
    SkPath::Iter iter(src, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                dst->moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                dst->lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb: {
                SkPoint p;
                for (int i = 1; i <= 8; ++i) {
                    SkEvalQuadAt(pts, i / 8.0f, &p, NULL);
                    dst->lineTo(p);
                }
            } break;
            case SkPath::kCubic_Verb: {
                SkPoint p;
                for (int i = 1; i <= 8; ++i) {
                    SkEvalCubicAt(pts, i / 8.0f, &p, NULL, NULL);
                    dst->lineTo(p);
                }
            } break;
        }
    }
}

static void setFade(SkPaint* paint, bool showGL) {
    paint->setAlpha(showGL ? 0x66 : 0xFF);
}

static void setGLFrame(SkPaint* paint) {
    paint->setColor(0xFFFF0000);
    paint->setStyle(SkPaint::kStroke_Style);
    paint->setAntiAlias(true);
    paint->setStrokeWidth(WIRE_FRAME_WIDTH);
}

static void show_mesh(SkCanvas* canvas, const SkRect& r) {
    SkPaint paint;
    setGLFrame(&paint);

    canvas->drawRect(r, paint);
    canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, paint);
}

static void drawLine(SkCanvas* canvas, const SkPoint& p0, const SkPoint& p1,
                     const SkPaint& paint) {
    canvas->drawLine(p0.fX, p0.fY, p1.fX, p1.fY, paint);
}

static void show_mesh(SkCanvas* canvas, const SkPoint pts[],
                      const uint16_t indices[], int count) {
    SkPaint paint;
    setGLFrame(&paint);

    for (int i = 0; i < count - 2; ++i) {
        drawLine(canvas, pts[indices[i]], pts[indices[i+1]], paint);
        drawLine(canvas, pts[indices[i+1]], pts[indices[i+2]], paint);
        drawLine(canvas, pts[indices[i+2]], pts[indices[i]], paint);
    }
}

static void show_glframe(SkCanvas* canvas, const SkPath& path) {
    SkPaint paint;
    setGLFrame(&paint);
    canvas->drawPath(path, paint);
}

static void show_mesh_between(SkCanvas* canvas, const SkPath& p0, const SkPath& p1) {
    SkPath d0, d1;
    tesselate(p0, &d0);
    tesselate(p1, &d1);

    SkPoint pts0[256*2], pts1[256];
    int count = d0.getPoints(pts0, SK_ARRAY_COUNT(pts0));
    int count1 = d1.getPoints(pts1, SK_ARRAY_COUNT(pts1));
    SkASSERT(count == count1);
    memcpy(&pts0[count], pts1, count * sizeof(SkPoint));

    uint16_t indices[256*6];
    uint16_t* ndx = indices;
    for (int i = 0; i < count; ++i) {
        *ndx++ = i;
        *ndx++ = i + count;
    }
    *ndx++ = 0;

    show_mesh(canvas, pts0, indices, ndx - indices);
}

static void show_fan(SkCanvas* canvas, const SkPath& path, SkScalar cx, SkScalar cy) {
    SkPaint paint;
    setGLFrame(&paint);

    canvas->drawPath(path, paint);

    SkPoint pts[256];
    int count = path.getPoints(pts, SK_ARRAY_COUNT(pts));
    for (int i = 0; i < count; ++i) {
        canvas->drawLine(pts[i].fX, pts[i].fY, cx, cy, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*DrawProc)(SkCanvas* canvas, bool showGL, int flags);

static void draw_line(SkCanvas* canvas, bool showGL, int flags) {
    SkPaint paint;
    paint.setAntiAlias(true);

    if (showGL) {
        setGLFrame(&paint);
    }
    canvas->drawLine(50, 50, 400, 100, paint);
    paint.setColor(SK_ColorBLACK);

    canvas->rotate(40);
    setFade(&paint, showGL);
    paint.setStrokeWidth(40);
    canvas->drawLine(100, 50, 450, 50, paint);
    if (showGL) {
        show_mesh(canvas, SkRect::MakeLTRB(100, 50-20, 450, 50+20));
    }
}

static void draw_rect(SkCanvas* canvas, bool showGL, int flags) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkRect r = SkRect::MakeLTRB(50, 70, 250, 370);

    setFade(&paint, showGL);
    canvas->drawRect(r, paint);
    if (showGL) {
        show_mesh(canvas, r);
    }

    canvas->translate(320, 0);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(25);
    canvas->drawRect(r, paint);
    if (showGL) {
        SkScalar rad = paint.getStrokeWidth() / 2;
        SkPoint pts[8];
        r.outset(rad, rad);
        r.toQuad(&pts[0]);
        r.inset(rad*2, rad*2);
        r.toQuad(&pts[4]);

        const uint16_t indices[] = {
            0, 4, 1, 5, 2, 6, 3, 7, 0, 4
        };
        show_mesh(canvas, pts, indices, SK_ARRAY_COUNT(indices));
    }
}

static void draw_oval(SkCanvas* canvas, bool showGL, int flags) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkRect r = SkRect::MakeLTRB(50, 70, 250, 370);

    setFade(&paint, showGL);
    canvas->drawOval(r, paint);
    if (showGL) {
        switch (flags) {
            case 0: {
                SkPath path;
                path.addOval(r);
                show_glframe(canvas, path);
            } break;
            case 1:
            case 3: {
                SkPath src, dst;
                src.addOval(r);
                tesselate(src, &dst);
                show_fan(canvas, dst, r.centerX(), r.centerY());
            } break;
            case 2: {
                SkPaint p(paint);
                show_mesh(canvas, r);
                setGLFrame(&p);
                paint.setStyle(SkPaint::kFill_Style);
                canvas->drawCircle(r.centerX(), r.centerY(), 3, p);
            } break;
        }
    }

    canvas->translate(320, 0);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(25);
    canvas->drawOval(r, paint);
    if (showGL) {
        switch (flags) {
            case 0: {
                SkPath path;
                SkScalar rad = paint.getStrokeWidth() / 2;
                r.outset(rad, rad);
                path.addOval(r);
                r.inset(rad*2, rad*2);
                path.addOval(r);
                show_glframe(canvas, path);
            } break;
            case 1: {
                SkPath path0, path1;
                SkScalar rad = paint.getStrokeWidth() / 2;
                r.outset(rad, rad);
                path0.addOval(r);
                r.inset(rad*2, rad*2);
                path1.addOval(r);
                show_mesh_between(canvas, path0, path1);
            } break;
            case 2: {
                SkPath path;
                path.addOval(r);
                show_glframe(canvas, path);
                SkScalar rad = paint.getStrokeWidth() / 2;
                r.outset(rad, rad);
                show_mesh(canvas, r);
            } break;
            case 3: {
                SkScalar rad = paint.getStrokeWidth() / 2;
                r.outset(rad, rad);
                SkPaint paint;
                paint.setAlpha(0x33);
                canvas->drawRect(r, paint);
                show_mesh(canvas, r);
            } break;
        }
    }
}

#include "SkImageDecoder.h"

static void draw_image(SkCanvas* canvas, bool showGL, int flags) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setFilterBitmap(true);
    setFade(&paint, showGL);

    static SkBitmap* gBM;
    if (NULL == gBM) {
        gBM = new SkBitmap;
        SkImageDecoder::DecodeFile("/skimages/startrek.png", gBM);
    }
    SkRect r = SkRect::MakeWH(gBM->width(), gBM->height());

    canvas->save();
    canvas->translate(30, 30);
    canvas->scale(0.8f, 0.8f);
    canvas->drawBitmap(*gBM, 0, 0, &paint);
    if (showGL) {
        show_mesh(canvas, r);
    }
    canvas->restore();

    canvas->translate(210, 290);
    canvas->rotate(-35);
    canvas->drawBitmap(*gBM, 0, 0, &paint);
    if (showGL) {
        show_mesh(canvas, r);
    }
}

static void draw_text(SkCanvas* canvas, bool showGL, int flags) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);
    const char text[] = "Graphics at Google";
    size_t len = strlen(text);
    setFade(&paint, showGL);

    canvas->translate(40, 50);
    for (int i = 0; i < 10; ++i) {
        paint.setTextSize(12 + i * 3);
        canvas->drawText(text, len, 0, 0, paint);
        if (showGL) {
            SkRect bounds[256];
            SkScalar widths[256];
            int count = paint.getTextWidths(text, len, widths, bounds);
            SkScalar adv = 0;
            for (int j = 0; j < count; ++j) {
                bounds[j].offset(adv, 0);
                show_mesh(canvas, bounds[j]);
                adv += widths[j];
            }
        }
        canvas->translate(0, paint.getTextSize() * 3 / 2);
    }
}

static const struct {
    DrawProc    fProc;
    const char* fName;
} gRec[] = {
    {   draw_line,  "Lines" },
    {   draw_rect,  "Rects" },
    {   draw_oval,  "Ovals" },
    {   draw_image, "Images" },
    {   draw_text,  "Text" },
};

class TalkGM : public skiagm::GM {
    DrawProc fProc;
    SkString fName;
    bool     fShowGL;
    int      fFlags;

public:
    TalkGM(int index, bool showGL, int flags = 0) {
        fProc = gRec[index].fProc;
        fName.set(gRec[index].fName);
        if (showGL) {
            fName.append("-gl");
        }
        fShowGL = showGL;
        fFlags = flags;
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkISize size = canvas->getDeviceSize();
        SkRect dst = SkRect::MakeWH(size.width(), size.height());
        SkRect src = SkRect::MakeWH(640, 480);
        SkMatrix matrix;
        matrix.setRectToRect(src, dst, SkMatrix::kCenter_ScaleToFit);

        canvas->concat(matrix);
        fProc(canvas, fShowGL, fFlags);
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

#define GM_CONCAT(X,Y) GM_CONCAT_IMPL(X,Y)
#define GM_CONCAT_IMPL(X,Y) X##Y

#define FACTORY_NAME  GM_CONCAT(Factory, __LINE__)
#define REGISTRY_NAME  GM_CONCAT(gReg, __LINE__)

#define ADD_GM(Class, args)    \
    static skiagm::GM* FACTORY_NAME(void*) { return new Class args; } \
    static skiagm::GMRegistry REGISTRY_NAME(FACTORY_NAME);

ADD_GM(TalkGM, (0, false))
ADD_GM(TalkGM, (0, true))
ADD_GM(TalkGM, (1, false))
ADD_GM(TalkGM, (1, true))
ADD_GM(TalkGM, (2, false))
ADD_GM(TalkGM, (2, true))
ADD_GM(TalkGM, (2, true, 1))
ADD_GM(TalkGM, (2, true, 2))
ADD_GM(TalkGM, (2, true, 3))
ADD_GM(TalkGM, (3, false))
ADD_GM(TalkGM, (3, true))
ADD_GM(TalkGM, (4, false))
ADD_GM(TalkGM, (4, true))

//static GM* MyFactory(void*) { return new TalkGM(0, false); }
//static GMRegistry reg(MyFactory);
