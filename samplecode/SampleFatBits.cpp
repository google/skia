/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImage.h"
#include "SkSurface.h"

#define FAT_PIXEL_COLOR     SK_ColorBLACK
#define PIXEL_CENTER_SIZE   3
#define WIRE_FRAME_COLOR    0xFFFF0000  /*0xFF00FFFF*/
#define WIRE_FRAME_SIZE     1.5f

static SkScalar apply_grid(SkScalar x) {
    const SkScalar grid = 2;
    return SkScalarRoundToScalar(x * grid) / grid;
}

static void apply_grid(SkPoint pts[], int count) {
    for (int i = 0; i < count; ++i) {
        pts[i].set(apply_grid(pts[i].fX), apply_grid(pts[i].fY));
    }
}

static void erase(SkSurface* surface) {
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
}

class FatBits {
public:
    FatBits() {
        fAA = false;
        fStyle = kHair_Style;
        fGrid = false;
        fShowSkeleton = true;
        fUseGPU = false;
        fUseClip = false;
        fRectAsOval = false;
        fUseTriangle = false;
        fStrokeCap = SkPaint::kButt_Cap;

        fClipRect.set(2, 2, 11, 8 );
    }

    int getZoom() const { return fZoom; }

    bool getAA() const { return fAA; }
    void setAA(bool aa) { fAA = aa; }

    bool getGrid() const { return fGrid; }
    void setGrid(bool g) { fGrid = g; }

    bool getShowSkeleton() const { return fShowSkeleton; }
    void setShowSkeleton(bool ss) { fShowSkeleton = ss; }

    bool getUseGPU() const { return fUseGPU; }
    void setUseGPU(bool ug) { fUseGPU = ug; }

    bool getTriangle() const { return fUseTriangle; }
    void setTriangle(bool ut) { fUseTriangle = ut; }

    void toggleRectAsOval() { fRectAsOval = !fRectAsOval; }

    bool getUseClip() const { return fUseClip; }
    void setUseClip(bool uc) { fUseClip = uc; }

    enum Style {
        kHair_Style,
        kStroke_Style,
    };
    Style getStyle() const { return fStyle; }
    void setStyle(Style s) { fStyle = s; }

    void setWHZ(int width, int height, int zoom) {
        fW = width;
        fH = height;
        fZoom = zoom;
        fBounds.set(0, 0, SkIntToScalar(width * zoom), SkIntToScalar(height * zoom));
        fMatrix.setScale(SkIntToScalar(zoom), SkIntToScalar(zoom));
        fInverse.setScale(SK_Scalar1 / zoom, SK_Scalar1 / zoom);
        fShader.reset(sk_tool_utils::create_checkerboard_shader(
                              0xFFCCCCCC, 0xFFFFFFFF, zoom));

        SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        fMinSurface.reset(SkSurface::NewRaster(info));
        info = info.makeWH(width * zoom, height * zoom);
        fMaxSurface.reset(SkSurface::NewRaster(info));
    }

    void drawBG(SkCanvas*);
    void drawFG(SkCanvas*);
    void drawLine(SkCanvas*, SkPoint pts[2]);
    void drawRect(SkCanvas* canvas, SkPoint pts[2]);
    void drawTriangle(SkCanvas* canvas, SkPoint pts[3]);

    SkPaint::Cap fStrokeCap;

private:
    bool fAA, fGrid, fShowSkeleton, fUseGPU, fUseClip, fRectAsOval, fUseTriangle;
    Style fStyle;
    int fW, fH, fZoom;
    SkMatrix fMatrix, fInverse;
    SkRect   fBounds, fClipRect;
    SkAutoTUnref<SkShader> fShader;
    SkAutoTUnref<SkSurface> fMinSurface;
    SkAutoTUnref<SkSurface> fMaxSurface;

    void setupPaint(SkPaint* paint) {
        bool aa = this->getAA();
        paint->setStrokeCap(fStrokeCap);
        switch (fStyle) {
            case kHair_Style:
                paint->setStrokeWidth(0);
                break;
            case kStroke_Style:
                paint->setStrokeWidth(SK_Scalar1);
                break;
        }
        paint->setAntiAlias(aa);
    }

    void setupSkeletonPaint(SkPaint* paint) {
        paint->setStyle(SkPaint::kStroke_Style);
        paint->setStrokeWidth(WIRE_FRAME_SIZE);
        paint->setColor(fShowSkeleton ? WIRE_FRAME_COLOR : 0);
        paint->setAntiAlias(true);
    }

    void drawTriangleSkeleton(SkCanvas* max, const SkPoint pts[]);
    void drawLineSkeleton(SkCanvas* max, const SkPoint pts[]);
    void drawRectSkeleton(SkCanvas* max, const SkRect& r) {
        SkPaint paint;
        this->setupSkeletonPaint(&paint);
        SkPath path;

        if (fUseGPU && fAA) {
            SkRect rr = r;
            rr.inset(SkIntToScalar(fZoom)/2, SkIntToScalar(fZoom)/2);
            path.addRect(rr);
            path.moveTo(rr.fLeft, rr.fTop);
            path.lineTo(rr.fRight, rr.fBottom);
            rr = r;
            rr.inset(-SkIntToScalar(fZoom)/2, -SkIntToScalar(fZoom)/2);
            path.addRect(rr);
        } else {
            fRectAsOval ? path.addOval(r) : path.addRect(r);
            if (fUseGPU) {
                path.moveTo(r.fLeft, r.fTop);
                path.lineTo(r.fRight, r.fBottom);
            }
        }
        max->drawPath(path, paint);
    }

    void copyMinToMax() {
        erase(fMaxSurface);
        SkCanvas* canvas = fMaxSurface->getCanvas();
        canvas->save();
        canvas->concat(fMatrix);
        fMinSurface->draw(canvas, 0, 0, nullptr);
        canvas->restore();

        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kClear_Mode);
        for (int iy = 1; iy < fH; ++iy) {
            SkScalar y = SkIntToScalar(iy * fZoom);
            canvas->drawLine(0, y - SK_ScalarHalf, 999, y - SK_ScalarHalf, paint);
        }
        for (int ix = 1; ix < fW; ++ix) {
            SkScalar x = SkIntToScalar(ix * fZoom);
            canvas->drawLine(x - SK_ScalarHalf, 0, x - SK_ScalarHalf, 999, paint);
        }
    }
};

void FatBits::drawBG(SkCanvas* canvas) {
    SkPaint paint;

    paint.setShader(fShader);
    canvas->drawRect(fBounds, paint);
    paint.setShader(nullptr);
}

void FatBits::drawFG(SkCanvas* canvas) {
    SkPaint inner, outer;

    inner.setAntiAlias(true);
    inner.setColor(SK_ColorBLACK);
    inner.setStrokeWidth(PIXEL_CENTER_SIZE);

    outer.setAntiAlias(true);
    outer.setColor(SK_ColorWHITE);
    outer.setStrokeWidth(PIXEL_CENTER_SIZE + 2);

    SkScalar half = SkIntToScalar(fZoom) / 2;
    for (int iy = 0; iy < fH; ++iy) {
        SkScalar y = SkIntToScalar(iy * fZoom) + half;
        for (int ix = 0; ix < fW; ++ix) {
            SkScalar x = SkIntToScalar(ix * fZoom) + half;

            canvas->drawPoint(x, y, outer);
            canvas->drawPoint(x, y, inner);
        }
    }

    if (fUseClip) {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setColor(SK_ColorLTGRAY);
        SkRect r = {
            fClipRect.fLeft * fZoom,
            fClipRect.fTop * fZoom,
            fClipRect.fRight * fZoom,
            fClipRect.fBottom * fZoom
        };
        canvas->drawRect(r, p);
    }
}

void FatBits::drawLineSkeleton(SkCanvas* max, const SkPoint pts[]) {
    SkPaint paint;
    this->setupSkeletonPaint(&paint);

    SkPath path;
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);

    switch (fStyle) {
        case kHair_Style:
            if (fUseGPU) {
                SkPaint p;
                p.setStyle(SkPaint::kStroke_Style);
                p.setStrokeWidth(SK_Scalar1 * fZoom);
                SkPath dst;
                p.getFillPath(path, &dst);
                path.addPath(dst);
            }
            break;
        case kStroke_Style: {
            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SK_Scalar1 * fZoom);
            p.setStrokeCap(fStrokeCap);
            SkPath dst;
            p.getFillPath(path, &dst);
            path = dst;

            path.moveTo(pts[0]);
            path.lineTo(pts[1]);

            if (fUseGPU) {
                path.moveTo(dst.getPoint(0));
                path.lineTo(dst.getPoint(2));
            }
        } break;
    }
    max->drawPath(path, paint);
}

void FatBits::drawLine(SkCanvas* canvas, SkPoint pts[]) {
    SkPaint paint;

    fInverse.mapPoints(pts, 2);

    if (fGrid) {
        apply_grid(pts, 2);
    }

    erase(fMinSurface);
    this->setupPaint(&paint);
    paint.setColor(FAT_PIXEL_COLOR);
    if (fUseClip) {
        fMinSurface->getCanvas()->save();
        SkRect r = fClipRect;
        r.inset(SK_Scalar1/3, SK_Scalar1/3);
        fMinSurface->getCanvas()->clipRect(r, SkRegion::kIntersect_Op, true);
    }
    fMinSurface->getCanvas()->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
    if (fUseClip) {
        fMinSurface->getCanvas()->restore();
    }
    this->copyMinToMax();

    SkCanvas* max = fMaxSurface->getCanvas();

    fMatrix.mapPoints(pts, 2);
    this->drawLineSkeleton(max, pts);

    fMaxSurface->draw(canvas, 0, 0, nullptr);
}

void FatBits::drawRect(SkCanvas* canvas, SkPoint pts[2]) {
    SkPaint paint;

    fInverse.mapPoints(pts, 2);

    if (fGrid) {
        apply_grid(pts, 2);
    }

    SkRect r;
    r.set(pts, 2);

    erase(fMinSurface);
    this->setupPaint(&paint);
    paint.setColor(FAT_PIXEL_COLOR);
    {
        SkCanvas* c = fMinSurface->getCanvas();
        fRectAsOval ? c->drawOval(r, paint) : c->drawRect(r, paint);
    }
    this->copyMinToMax();

    SkCanvas* max = fMaxSurface->getCanvas();

    fMatrix.mapPoints(pts, 2);
    r.set(pts, 2);
    this->drawRectSkeleton(max, r);

    fMaxSurface->draw(canvas, 0, 0, nullptr);
}

void FatBits::drawTriangleSkeleton(SkCanvas* max, const SkPoint pts[]) {
    SkPaint paint;
    this->setupSkeletonPaint(&paint);

    SkPath path;
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);
    path.lineTo(pts[2]);
    path.close();

    max->drawPath(path, paint);
}

void FatBits::drawTriangle(SkCanvas* canvas, SkPoint pts[3]) {
    SkPaint paint;

    fInverse.mapPoints(pts, 3);

    if (fGrid) {
        apply_grid(pts, 3);
    }

    SkPath path;
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);
    path.lineTo(pts[2]);
    path.close();

    erase(fMinSurface);
    this->setupPaint(&paint);
    paint.setColor(FAT_PIXEL_COLOR);
    fMinSurface->getCanvas()->drawPath(path, paint);
    this->copyMinToMax();

    SkCanvas* max = fMaxSurface->getCanvas();

    fMatrix.mapPoints(pts, 3);
    this->drawTriangleSkeleton(max, pts);

    fMaxSurface->draw(canvas, 0, 0, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class IndexClick : public SkView::Click {
    int fIndex;
public:
    IndexClick(SkView* v, int index) : SkView::Click(v), fIndex(index) {}

    static int GetIndex(SkView::Click* click) {
        return ((IndexClick*)click)->fIndex;
    }
};

class DrawLineView : public SampleView {
    enum {
        kZoom = 96
    };

    FatBits fFB;
    SkPoint fPts[3];
    bool    fIsRect;
public:
    DrawLineView() {
        fFB.setWHZ(24, 16, kZoom);
        fPts[0].set(1, 1);
        fPts[1].set(5, 4);
        fPts[2].set(2, 6);
        SkMatrix::MakeScale(kZoom, kZoom).mapPoints(fPts, 3);
        fIsRect = false;
    }

    void setStyle(FatBits::Style s) {
        fFB.setStyle(s);
        this->inval(nullptr);
    }

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FatBits");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'c':
                    fFB.setUseClip(!fFB.getUseClip());
                    this->inval(nullptr);
                    return true;
                case 'r':
                    fIsRect = !fIsRect;
                    this->inval(nullptr);
                    return true;
                case 'o':
                    fFB.toggleRectAsOval();
                    this->inval(nullptr);
                    return true;
                case 'x':
                    fFB.setGrid(!fFB.getGrid());
                    this->inval(nullptr);
                    return true;
                case 's':
                    if (FatBits::kStroke_Style == fFB.getStyle()) {
                        this->setStyle(FatBits::kHair_Style);
                    } else {
                        this->setStyle(FatBits::kStroke_Style);
                    }
                    return true;
                case 'k': {
                    const SkPaint::Cap caps[] = {
                        SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap,
                    };
                    fFB.fStrokeCap = caps[(fFB.fStrokeCap + 1) % 3];
                    this->inval(nullptr);
                    return true;
                } break;
                case 'a':
                    fFB.setAA(!fFB.getAA());
                    this->inval(nullptr);
                    return true;
                case 'w':
                    fFB.setShowSkeleton(!fFB.getShowSkeleton());
                    this->inval(nullptr);
                    return true;
                case 'g':
                    fFB.setUseGPU(!fFB.getUseGPU());
                    this->inval(nullptr);
                    return true;
                case 't':
                    fFB.setTriangle(!fFB.getTriangle());
                    this->inval(nullptr);
                    return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        fFB.drawBG(canvas);
        if (fFB.getTriangle()) {
            fFB.drawTriangle(canvas, fPts);
        }
        else if (fIsRect) {
            fFB.drawRect(canvas, fPts);
        } else {
            fFB.drawLine(canvas, fPts);
        }
        fFB.drawFG(canvas);

        {
            SkString str;
            str.printf("%s %s %s %s",
                       fFB.getAA() ? "AA" : "BW",
                       FatBits::kHair_Style == fFB.getStyle() ? "Hair" : "Stroke",
                       fFB.getUseGPU() ? "GPU" : "CPU",
                       fFB.getUseClip() ? "clip" : "noclip");
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setTextSize(16);
            paint.setColor(SK_ColorBLUE);
            canvas->drawText(str.c_str(), str.size(), 10, 16, paint);
        }
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        SkPoint pt = { x, y };
        int index = -1;
        int count = fFB.getTriangle() ? 3 : 2;
        SkScalar tol = 12;

        for (int i = 0; i < count; ++i) {
            if (fPts[i].equalsWithinTolerance(pt, tol)) {
                index = i;
                break;
            }
        }
        return new IndexClick(this, index);
    }

    bool onClick(Click* click) override {
        int index = IndexClick::GetIndex(click);
        if (index >= 0 && index <= 2) {
            fPts[index] = click->fCurr;
        } else {
            SkScalar dx = click->fCurr.fX - click->fPrev.fX;
            SkScalar dy = click->fCurr.fY - click->fPrev.fY;
            fPts[0].offset(dx, dy);
            fPts[1].offset(dx, dy);
            fPts[2].offset(dx, dy);
        }
        this->inval(nullptr);
        return true;
    }

private:

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawLineView; }
static SkViewRegister reg(MyFactory);
