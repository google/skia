/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkOSFile.h"
#include "SkStream.h"

#include "SkGeometry.h" // private include :(

static sk_sp<SkShader> make_shader0(SkIPoint* size) {
    SkBitmap    bm;

//    decode_file("/skimages/progressivejpg.jpg", &bm);
    decode_file("/skimages/logo.png", &bm);
    size->set(bm.width(), bm.height());
    return SkShader::MakeBitmapShader(bm, SkShader::kClamp_TileMode,
                                       SkShader::kClamp_TileMode);
}

static sk_sp<SkShader> make_shader1(const SkIPoint& size) {
    SkPoint pts[] = { { 0, 0, },
                      { SkIntToScalar(size.fX), SkIntToScalar(size.fY) } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED };
    return SkGradientShader::MakeLinear(pts, colors, nullptr,
                    SK_ARRAY_COUNT(colors), SkShader::kMirror_TileMode);
}

///////////////////////////////////////////////////////////////////////////////

class Patch {
public:
    Patch() { sk_bzero(fPts, sizeof(fPts)); }
    ~Patch() {}

    void setPatch(const SkPoint pts[12]) {
        memcpy(fPts, pts, 12 * sizeof(SkPoint));
        fPts[12] = pts[0];  // the last shall be first
    }
    void setBounds(int w, int h) { fW = w; fH = h; }

    void draw(SkCanvas*, const SkPaint&, int segsU, int segsV,
              bool doTextures, bool doColors);

private:
    SkPoint fPts[13];
    int     fW, fH;
};

static void eval_patch_edge(const SkPoint cubic[], SkPoint samples[], int segs) {
    SkScalar t = 0;
    SkScalar dt = SK_Scalar1 / segs;

    samples[0] = cubic[0];
    for (int i = 1; i < segs; i++) {
        t += dt;
        SkEvalCubicAt(cubic, t, &samples[i], nullptr, nullptr);
    }
}

static void eval_sheet(const SkPoint edge[], int nu, int nv, int iu, int iv,
                       SkPoint* pt) {
    const int TL = 0;
    const int TR = nu;
    const int BR = TR + nv;
    const int BL = BR + nu;

    SkScalar u = SkIntToScalar(iu) / nu;
    SkScalar v = SkIntToScalar(iv) / nv;

    SkScalar uv = SkScalarMul(u, v);
    SkScalar Uv = SkScalarMul(SK_Scalar1 - u, v);
    SkScalar uV = SkScalarMul(u, SK_Scalar1 - v);
    SkScalar UV = SkScalarMul(SK_Scalar1 - u, SK_Scalar1 - v);

    SkScalar x0 = SkScalarMul(UV, edge[TL].fX) + SkScalarMul(uV, edge[TR].fX) +
                  SkScalarMul(Uv, edge[BL].fX) + SkScalarMul(uv, edge[BR].fX);
    SkScalar y0 = SkScalarMul(UV, edge[TL].fY) + SkScalarMul(uV, edge[TR].fY) +
                  SkScalarMul(Uv, edge[BL].fY) + SkScalarMul(uv, edge[BR].fY);

    SkScalar x =    SkScalarMul(SK_Scalar1 - v, edge[TL+iu].fX) +
                    SkScalarMul(u, edge[TR+iv].fX) +
                    SkScalarMul(v, edge[BR+nu-iu].fX) +
                    SkScalarMul(SK_Scalar1 - u, edge[BL+nv-iv].fX) - x0;
    SkScalar y =    SkScalarMul(SK_Scalar1 - v, edge[TL+iu].fY) +
                    SkScalarMul(u, edge[TR+iv].fY) +
                    SkScalarMul(v, edge[BR+nu-iu].fY) +
                    SkScalarMul(SK_Scalar1 - u, edge[BL+nv-iv].fY) - y0;
    pt->set(x, y);
}

static SkColor make_color(SkScalar s, SkScalar t) {
    return SkColorSetARGB(0xFF, SkUnitScalarClampToByte(s), SkUnitScalarClampToByte(t), 0);
}

void Patch::draw(SkCanvas* canvas, const SkPaint& paint, int nu, int nv,
                 bool doTextures, bool doColors) {
    if (nu < 1 || nv < 1) {
        return;
    }

    int i, npts = (nu + nv) * 2;
    SkAutoSTMalloc<16, SkPoint> storage(npts + 1);
    SkPoint* edge0 = storage.get();
    SkPoint* edge1 = edge0 + nu;
    SkPoint* edge2 = edge1 + nv;
    SkPoint* edge3 = edge2 + nu;

    // evaluate the edge points
    eval_patch_edge(fPts + 0, edge0, nu);
    eval_patch_edge(fPts + 3, edge1, nv);
    eval_patch_edge(fPts + 6, edge2, nu);
    eval_patch_edge(fPts + 9, edge3, nv);
    edge3[nv] = edge0[0];   // the last shall be first

    for (i = 0; i < npts; i++) {
//        canvas->drawLine(edge0[i].fX, edge0[i].fY, edge0[i+1].fX, edge0[i+1].fY, paint);
    }

    int row, vertCount = (nu + 1) * (nv + 1);
    SkAutoTMalloc<SkPoint>  vertStorage(vertCount);
    SkPoint* verts = vertStorage.get();

    // first row
    memcpy(verts, edge0, (nu + 1) * sizeof(SkPoint));
    // rows
    SkPoint* r = verts;
    for (row = 1; row < nv; row++) {
        r += nu + 1;
        r[0] = edge3[nv - row];
        for (int col = 1; col < nu; col++) {
            eval_sheet(edge0, nu, nv, col, row, &r[col]);
        }
        r[nu] = edge1[row];
    }
    // last row
    SkPoint* last = verts + nv * (nu + 1);
    for (i = 0; i <= nu; i++) {
        last[i] = edge2[nu - i];
    }

//    canvas->drawPoints(verts, vertCount, paint);

    int stripCount = (nu + 1) * 2;
    SkAutoTMalloc<SkPoint>  stripStorage(stripCount * 2);
    SkAutoTMalloc<SkColor>  colorStorage(stripCount);
    SkPoint* strip = stripStorage.get();
    SkPoint* tex = strip + stripCount;
    SkColor* colors = colorStorage.get();
    SkScalar t = 0;
    const SkScalar ds = SK_Scalar1 * fW / nu;
    const SkScalar dt = SK_Scalar1 * fH / nv;
    r = verts;
    for (row = 0; row < nv; row++) {
        SkPoint* upper = r;
        SkPoint* lower = r + nu + 1;
        r = lower;
        SkScalar s = 0;
        for (i = 0; i <= nu; i++)  {
            strip[i*2 + 0] = *upper++;
            strip[i*2 + 1] = *lower++;
            tex[i*2 + 0].set(s, t);
            tex[i*2 + 1].set(s, t + dt);
            colors[i*2 + 0] = make_color(s/fW, t/fH);
            colors[i*2 + 1] = make_color(s/fW, (t + dt)/fH);
            s += ds;
        }
        t += dt;
        canvas->drawVertices(SkCanvas::kTriangleStrip_VertexMode, stripCount,
                             strip, doTextures ? tex : nullptr,
                             doColors ? colors : nullptr, nullptr,
                             nullptr, 0, paint);
    }
}

static void drawpatches(SkCanvas* canvas, const SkPaint& paint, int nu, int nv,
                        Patch* patch) {
    SkAutoCanvasRestore ar(canvas, true);

    patch->draw(canvas, paint, nu, nv, false, false);
    canvas->translate(SkIntToScalar(180), 0);
    patch->draw(canvas, paint, nu, nv, true, false);
    canvas->translate(SkIntToScalar(180), 0);
    patch->draw(canvas, paint, nu, nv, false, true);
    canvas->translate(SkIntToScalar(180), 0);
    patch->draw(canvas, paint, nu, nv, true, true);
}

const SkScalar DX = 20;
const SkScalar DY = 0;

class PatchView : public SampleView {
    SkScalar    fAngle;
    sk_sp<SkShader> fShader0;
    sk_sp<SkShader> fShader1;
    SkIPoint    fSize0, fSize1;
    SkPoint     fPts[12];

public:
    PatchView() : fAngle(0) {
        fShader0 = make_shader0(&fSize0);
        fSize1 = fSize0;
        if (fSize0.fX == 0 || fSize0.fY == 0) {
            fSize1.set(2, 2);
        }
        fShader1 = make_shader1(fSize1);

        const SkScalar S = SkIntToScalar(50);
        const SkScalar T = SkIntToScalar(40);
        fPts[0].set(S*0, T);
        fPts[1].set(S*1, T);
        fPts[2].set(S*2, T);
        fPts[3].set(S*3, T);
        fPts[4].set(S*3, T*2);
        fPts[5].set(S*3, T*3);
        fPts[6].set(S*3, T*4);
        fPts[7].set(S*2, T*4);
        fPts[8].set(S*1, T*4);
        fPts[9].set(S*0, T*4);
        fPts[10].set(S*0, T*3);
        fPts[11].set(S*0, T*2);

        this->setBGColor(SK_ColorGRAY);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt)  override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Patch");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        const int nu = 10;
        const int nv = 10;

        SkPaint paint;
        paint.setDither(true);
        paint.setFilterQuality(kLow_SkFilterQuality);

        canvas->translate(DX, DY);

        Patch   patch;

        paint.setShader(fShader0);
        if (fSize0.fX == 0) {
            fSize0.fX = 1;
        }
        if (fSize0.fY == 0) {
            fSize0.fY = 1;
        }
        patch.setBounds(fSize0.fX, fSize0.fY);

        patch.setPatch(fPts);
        drawpatches(canvas, paint, nu, nv, &patch);

        paint.setShader(nullptr);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(5));
        canvas->drawPoints(SkCanvas::kPoints_PointMode, SK_ARRAY_COUNT(fPts), fPts, paint);

        canvas->translate(0, SkIntToScalar(300));

        paint.setAntiAlias(false);
        paint.setShader(fShader1);
        if (true) {
            SkMatrix m;
            m.setSkew(1, 0);
            paint.setShader(paint.getShader()->makeWithLocalMatrix(m));
        }
        if (true) {
            SkMatrix m;
            m.setRotate(fAngle);
            paint.setShader(paint.getShader()->makeWithLocalMatrix(m));
        }
        patch.setBounds(fSize1.fX, fSize1.fY);
        drawpatches(canvas, paint, nu, nv, &patch);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fAngle = timer.scaled(60, 360);
        return true;
    }

    class PtClick : public Click {
    public:
        int fIndex;
        PtClick(SkView* view, int index) : Click(view), fIndex(index) {}
    };

    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(5);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        x -= DX;
        y -= DY;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            if (hittest(fPts[i], x, y)) {
                return new PtClick(this, (int)i);
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        fPts[((PtClick*)click)->fIndex].set(click->fCurr.fX - DX, click->fCurr.fY - DY);
        this->inval(nullptr);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PatchView; }
static SkViewRegister reg(MyFactory);
