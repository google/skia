#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkImageRef.h"
#include "SkOSFile.h"
#include "SkStream.h"

#include "SkGeometry.h" // private include :(

static void drawtriangle(SkCanvas* canvas, const SkPaint& paint,
                         const SkPoint pts[3]) {
    SkPath path;
    
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);
    path.lineTo(pts[2]);
    
    canvas->drawPath(path, paint);
}

static SkShader* make_shader0(SkIPoint* size) {
    SkBitmap    bm;
    
//    SkImageDecoder::DecodeFile("/skimages/progressivejpg.jpg", &bm);
    SkImageDecoder::DecodeFile("/skimages/beach.jpg", &bm);
    size->set(bm.width(), bm.height());
    return SkShader::CreateBitmapShader(bm, SkShader::kClamp_TileMode,
                                        SkShader::kClamp_TileMode);
}

static SkShader* make_shader1(const SkIPoint& size) {
    SkPoint pts[] = { 0, 0, SkIntToScalar(size.fX), SkIntToScalar(size.fY) };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED };
    return SkGradientShader::CreateLinear(pts, colors, NULL,
                    SK_ARRAY_COUNT(colors), SkShader::kMirror_TileMode, NULL);
}

///////////////////////////////////////////////////////////////////////////////

class Patch {
public:
    Patch() { bzero(fPts, sizeof(fPts)); }
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
        SkEvalCubicAt(cubic, t, &samples[i], NULL, NULL);
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

static int ScalarTo255(SkScalar v) {
    int scale = SkScalarToFixed(v) >> 8;
    if (scale < 0) {
        scale = 0;
    } else if (scale > 255) {
        scale = 255;
    }
    return scale;
}

static SkColor make_color(SkScalar s, SkScalar t) {
    int cs = ScalarTo255(s);
    int ct = ScalarTo255(t);    
    return SkColorSetARGB(0xFF, cs, 0, 0) + SkColorSetARGB(0, 0, ct, 0);
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
                             strip, doTextures ? tex : NULL,
                             doColors ? colors : NULL, NULL,
                             NULL, 0, paint);
    }
}

static void drawpatches(SkCanvas* canvas, const SkPaint& paint, int nu, int nv,
                        Patch* patch) {

    SkAutoCanvasRestore ar(canvas, true);

    patch->draw(canvas, paint, 10, 10, false, false);
    canvas->translate(SkIntToScalar(300), 0);
    patch->draw(canvas, paint, 10, 10, true, false);
    canvas->translate(SkIntToScalar(300), 0);
    patch->draw(canvas, paint, 10, 10, false, true);
    canvas->translate(SkIntToScalar(300), 0);
    patch->draw(canvas, paint, 10, 10, true, true);
}

class PatchView : public SkView {
    SkShader*   fShader0;
    SkShader*   fShader1;
    SkIPoint    fSize0, fSize1;
    SkPoint     fPts[12];
    
public:    
	PatchView() {
        fShader0 = make_shader0(&fSize0);
        fSize1 = fSize0;
        if (fSize0.fX == 0 || fSize0.fY == 0) {
            fSize1.set(2, 2);
        }
        fShader1 = make_shader1(fSize1);

        const SkScalar S = SkIntToScalar(90);
        const SkScalar T = SkIntToScalar(64);
        fPts[0].set(S*1, T);
        fPts[1].set(S*2, T);
        fPts[2].set(S*3, T);
        fPts[3].set(S*4, T);
        fPts[4].set(S*4, T*2);
        fPts[5].set(S*4, T*3);
        fPts[6].set(S*4, T*4);
        fPts[7].set(S*3, T*4);
        fPts[8].set(S*2, T*4);
        fPts[9].set(S*1, T*4);
        fPts[10].set(S*1, T*3);
        fPts[11].set(S*1, T*2);
    }
    
    virtual ~PatchView() {
        fShader0->safeUnref();
        fShader1->safeUnref();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt))
        {
            SkString str("Patch");
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkPaint paint;
        paint.setDither(true);
        paint.setFilterBitmap(true);

        if (false) {
            SkPath p;
            p.moveTo(0, 0);
            p.lineTo(SkIntToScalar(30000), SkIntToScalar(30000));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(4));
            paint.setAntiAlias(true);
            canvas->scale(SkIntToScalar(3), SkIntToScalar(3));
            canvas->drawPath(p, paint);
            return;
        }
        
        if (false) {
            for (int dy = -1; dy <= 2; dy++) {
                canvas->save();
                if (dy == 2) {
                    canvas->translate(0, SK_Scalar1/2);
                } else {
                    canvas->translate(0, SkIntToScalar(dy)/100);
                }
            
                SkBitmap bm;
                bm.setConfig(SkBitmap::kARGB_8888_Config, 20, 20);
                bm.allocPixels();
                SkCanvas c(bm);
                SkRect r = { 0, 0, 20*SK_Scalar1, SK_Scalar1 };
                for (int y = 0; y < 20; y++) {
                    SkPaint p;
                    p.setARGB(0xFF, y*5&0xFF, y*13&0xFF, y*29&0xFF);
                    c.drawRect(r, p);
                    r.offset(0, SK_Scalar1);
                }
                SkIRect src;
                SkRect  dst;
                
                static const int srcPts[] = {
                 //   2, 0, 15, 2,
                    2, 2, 15, 16,
                    17, 2, 2, 16,
                    19, 2, 1, 16,
                //    2, 18, 15, 2
                };
                static const double dstPts[] = {
                //    7, 262 15, 24.5,
                    7, 286.5, 15, 16,
                    22, 286.5, 5, 16,
                    27, 286.5, 1, 16,
                 //   7, 302.5, 15, 24.5
                };
                
                SkPaint p;
//                p.setFilterBitmap(true);
                const int* s = srcPts;
                const double* d = dstPts;
                for (int i = 0; i < 3; i++) {
                    src.set(s[0], s[1], s[0]+s[2], s[1]+s[3]);
                    dst.set(SkDoubleToScalar(d[0]),
                            SkDoubleToScalar(d[1]),
                            SkDoubleToScalar(d[0]+d[2]),
                            SkDoubleToScalar(d[1]+d[3]));
                    canvas->drawBitmapRect(bm, &src, dst, &p);
                    canvas->translate(SkDoubleToScalar(1), 0);
                    s += 4;
                    d += 4;
                }
                canvas->restore();
                canvas->translate(SkIntToScalar(32), 0);
            }
            return;
        }
        
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
        drawpatches(canvas, paint, 10, 10, &patch);
        
        paint.setShader(NULL);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(5));
        canvas->drawPoints(SkCanvas::kPoints_PointMode, SK_ARRAY_COUNT(fPts),
                           fPts, paint);
        
        canvas->translate(0, SkIntToScalar(300));
        
        paint.setAntiAlias(false);
        paint.setShader(fShader1);
        patch.setBounds(fSize1.fX, fSize1.fY);
        drawpatches(canvas, paint, 10, 10, &patch);
    }
    
    class PtClick : public Click {
    public:
        int fIndex;
        PtClick(SkView* view, int index) : Click(view), fIndex(index) {}
    };
    
    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(5);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        for (int i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            if (hittest(fPts[i], x, y)) {
                return new PtClick(this, i);
            }
        }
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        fPts[((PtClick*)click)->fIndex].set(click->fCurr.fX, click->fCurr.fY);
        this->inval(NULL);
        return true;
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PatchView; }
static SkViewRegister reg(MyFactory);

