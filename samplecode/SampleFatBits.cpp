/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

static void erase(SkSurface* surface) {
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
}

static SkShader* createChecker() {
//    SkColor colors[] = { 0xFFFDFDFD, 0xFFF4F4F4 };
    SkColor colors[] = { 0xFFFFFFFF, 0xFFFFFFFF };
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 2, 2);
    bm.allocPixels();
    bm.lockPixels();
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = SkPreMultiplyColor(colors[0]);
    *bm.getAddr32(0, 1) = *bm.getAddr32(1, 0) = SkPreMultiplyColor(colors[1]);
    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);

    SkMatrix m;
    m.setScale(12, 12);
    s->setLocalMatrix(m);
    return s;
}

class FatBits {
public:
    FatBits() : fShader(createChecker()) {
        fAA = false;
        fStyle = kHair_Style;
        fGrid = true;
        fShowSkeleton = true;
        fUseGPU = false;
        fUseClip = false;
        fRectAsOval = false;

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
        fShader->setLocalMatrix(fMatrix);

        fMinSurface.reset(SkSurface::NewRasterPMColor(width, height));
        width *= zoom;
        height *= zoom;
        fMaxSurface.reset(SkSurface::NewRasterPMColor(width, height));
    }

    void drawBG(SkCanvas*);
    void drawFG(SkCanvas*);
    void drawLine(SkCanvas*, SkPoint pts[2]);
    void drawRect(SkCanvas* canvas, SkPoint pts[2]);

private:
    bool fAA, fGrid, fShowSkeleton, fUseGPU, fUseClip, fRectAsOval;
    Style fStyle;
    int fW, fH, fZoom;
    SkMatrix fMatrix, fInverse;
    SkRect   fBounds, fClipRect;
    SkAutoTUnref<SkShader> fShader;
    SkAutoTUnref<SkSurface> fMinSurface;
    SkAutoTUnref<SkSurface> fMaxSurface;

    void setupPaint(SkPaint* paint) {
        bool aa = this->getAA();
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
        fMinSurface->draw(canvas, 0, 0, NULL);
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
    paint.setShader(NULL);
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
            SkPath dst;
            p.getFillPath(path, &dst);
            path = dst;

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
        SkScalar dd = 0;//SK_Scalar1 / 50;
        pts[0].set(SkScalarRoundToScalar(pts[0].fX) + dd,
                   SkScalarRoundToScalar(pts[0].fY) + dd);
        pts[1].set(SkScalarRoundToScalar(pts[1].fX) + dd,
                   SkScalarRoundToScalar(pts[1].fY) + dd);
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

    fMaxSurface->draw(canvas, 0, 0, NULL);
}

void FatBits::drawRect(SkCanvas* canvas, SkPoint pts[2]) {
    SkPaint paint;

    fInverse.mapPoints(pts, 2);

    if (fGrid) {
        pts[0].set(SkScalarRoundToScalar(pts[0].fX), SkScalarRoundToScalar(pts[0].fY));
        pts[1].set(SkScalarRoundToScalar(pts[1].fX), SkScalarRoundToScalar(pts[1].fY));
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

    fMaxSurface->draw(canvas, 0, 0, NULL);
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
    FatBits fFB;
    SkPoint fPts[2];
    bool    fIsRect;
public:
    DrawLineView() {
        fFB.setWHZ(24, 16, 48);
        fPts[0].set(48, 48);
        fPts[1].set(48 * 5, 48 * 4);
        fIsRect = false;
    }

    void setStyle(FatBits::Style s) {
        fFB.setStyle(s);
        this->inval(NULL);
    }

protected:
    virtual bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FatBits");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'c':
                    fFB.setUseClip(!fFB.getUseClip());
                    this->inval(NULL);
                    return true;
                case 'r':
                    fIsRect = !fIsRect;
                    this->inval(NULL);
                    return true;
                case 'o':
                    fFB.toggleRectAsOval();
                    this->inval(NULL);
                    return true;
                case 'x':
                    fFB.setGrid(!fFB.getGrid());
                    this->inval(NULL);
                    return true;
                case 's':
                    if (FatBits::kStroke_Style == fFB.getStyle()) {
                        this->setStyle(FatBits::kHair_Style);
                    } else {
                        this->setStyle(FatBits::kStroke_Style);
                    }
                    return true;
                case 'a':
                    fFB.setAA(!fFB.getAA());
                    this->inval(NULL);
                    return true;
                case 'w':
                    fFB.setShowSkeleton(!fFB.getShowSkeleton());
                    this->inval(NULL);
                    return true;
                case 'g':
                    fFB.setUseGPU(!fFB.getUseGPU());
                    this->inval(NULL);
                    return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        fFB.drawBG(canvas);
        if (fIsRect) {
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

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) SK_OVERRIDE {
        SkPoint pt = { x, y };
        int index = -1;
        SkScalar tol = 12;
        if (fPts[0].equalsWithinTolerance(pt, tol)) {
            index = 0;
        } else if (fPts[1].equalsWithinTolerance(pt, tol)) {
            index = 1;
        }
        return new IndexClick(this, index);
    }

    virtual bool onClick(Click* click) SK_OVERRIDE {
        int index = IndexClick::GetIndex(click);
        if (index >= 0 && index <= 1) {
            fPts[index] = click->fCurr;
        } else {
            SkScalar dx = click->fCurr.fX - click->fPrev.fX;
            SkScalar dy = click->fCurr.fY - click->fPrev.fY;
            fPts[0].offset(dx, dy);
            fPts[1].offset(dx, dy);
        }
        this->inval(NULL);
        return true;
    }

private:

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawLineView; }
static SkViewRegister reg(MyFactory);
