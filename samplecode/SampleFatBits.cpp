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

static void erase(SkSurface* surface) {
    surface->getCanvas()->clear(0);
}

static SkShader* createChecker() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 2, 2);
    bm.allocPixels();
    bm.lockPixels();
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = SkPreMultiplyColor(0xFFFDFDFD);
    *bm.getAddr32(0, 1) = *bm.getAddr32(1, 0) = SkPreMultiplyColor(0xFFF4F4F4);
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
    }

    bool getAA() const { return fAA; }
    void setAA(bool aa) { fAA = aa; }

    bool getGrid() const { return fGrid; }
    void setGrid(bool g) { fGrid = g; }

    enum Style {
        kHair_Style,
        kStroke_Style
    };
    Style getStyle() const { return fStyle; }
    void setStyle(Style s) { fStyle = s; }

    void setWHZ(int width, int height, int zoom) {
        fW = width;
        fH = height;
        fZ = zoom;
        fBounds.set(0, 0, SkIntToScalar(width * zoom), SkIntToScalar(height * zoom));
        fMatrix.setScale(SkIntToScalar(zoom), SkIntToScalar(zoom));
        fInverse.setScale(SK_Scalar1 / zoom, SK_Scalar1 / zoom);
        fShader->setLocalMatrix(fMatrix);
        
        SkImage::Info info = {
            width, height, SkImage::kPMColor_ColorType, SkImage::kPremul_AlphaType
        };
        fMinSurface.reset(SkSurface::NewRaster(info, NULL));
        info.fWidth *= zoom;
        info.fHeight *= zoom;
        fMaxSurface.reset(SkSurface::NewRaster(info, NULL));
    }

    void drawBG(SkCanvas*);
    void drawFG(SkCanvas*);
    void drawLine(SkCanvas*, SkPoint pts[2]);
    void drawRect(SkCanvas* canvas, SkPoint pts[2]);

private:
    bool fAA, fGrid;
    Style fStyle;
    int fW, fH, fZ;
    SkMatrix fMatrix, fInverse;
    SkRect   fBounds;
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
                paint->setStrokeWidth(SK_Scalar1 + SK_Scalar1/500);
                break;
        }
        paint->setAntiAlias(aa);
    }

    void drawLineSkeleton(SkCanvas* max, const SkPoint pts[]);
    void drawRectSkeleton(SkCanvas* max, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);
        max->drawRect(r, paint);
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
            SkScalar y = SkIntToScalar(iy * fZ);
            canvas->drawLine(0, y - SK_ScalarHalf, 999, y - SK_ScalarHalf, paint);
        }
        for (int ix = 1; ix < fW; ++ix) {
            SkScalar x = SkIntToScalar(ix * fZ);
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
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFFFFAA66);
    paint.setStrokeWidth(SK_Scalar1 * 2);

    SkScalar half = SkIntToScalar(fZ) / 2;
    for (int iy = 0; iy < fH; ++iy) {
        SkScalar y = SkIntToScalar(iy * fZ) + half;
        for (int ix = 0; ix < fW; ++ix) {
            SkScalar x = SkIntToScalar(ix * fZ) + half;
            
            canvas->drawPoint(x, y, paint);
        }
    }
}

void FatBits::drawLineSkeleton(SkCanvas* max, const SkPoint pts[]) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    
    SkPath path;
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);
    
    switch (fStyle) {
        case kHair_Style:
            break;
        case kStroke_Style: {
            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SK_Scalar1 * fZ);
            SkPath dst;
            p.getFillPath(path, &dst);
            path = dst;
        } break;
    }
    max->drawPath(path, paint);
}

void FatBits::drawLine(SkCanvas* canvas, SkPoint pts[]) {
    SkPaint paint;
    
    fInverse.mapPoints(pts, 2);
    
    if (fGrid) {
        pts[0].set(SkScalarRoundToScalar(pts[0].fX), SkScalarRoundToScalar(pts[0].fY));
        pts[1].set(SkScalarRoundToScalar(pts[1].fX), SkScalarRoundToScalar(pts[1].fY));
    }
    
    erase(fMinSurface);
    this->setupPaint(&paint);
    paint.setColor(SK_ColorBLUE);
    fMinSurface->getCanvas()->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
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
    paint.setColor(SK_ColorBLUE);
    fMinSurface->getCanvas()->drawRect(r, paint);
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
        fPts[0].set(32, 32);
        fPts[1].set(32 * 5, 32 * 4);
        fIsRect = true;
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
                case 'r':
                    fIsRect = !fIsRect;
                    this->inval(NULL);
                    return true;
                case 'g':
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
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        SkPoint pt = { x, y };
        int index = -1;
        SkScalar tol = 8;
        if (fPts[0].equalsWithinTolerance(pt, tol)) {
            index = 0;
        } else if (fPts[1].equalsWithinTolerance(pt, tol)) {
            index = 1;
        }
        return index >= 0 ? new IndexClick(this, index) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        int index = IndexClick::GetIndex(click);
        SkASSERT(index >= 0 && index <= 1);
        fPts[index] = click->fCurr;
        this->inval(NULL);
        return true;
    }
    
private:
    
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawLineView; }
static SkViewRegister reg(MyFactory);

