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
#include "SkNinePatch.h"

void setup_vertexbug(SkPoint verts[], SkPoint texs[], uint16_t index[]);

static void drawbug(SkCanvas* canvas, SkScalar scale) {
    SkBitmap    bm, bm2;
    
    SkImageDecoder::DecodeFile("/skimages/btn_default_normal.9.png", &bm);
    SkPaint paint;
    
    SkIRect subset;
    subset.set(1, 1, bm.width() - 1, bm.height() - 1);
    bm.extractSubset(&bm2, subset);
    
#if 0
    SkPoint verts[16], texs[16];
    uint16_t    index[54];
    
    SkShader* s = SkShader::CreateBitmapShader(bm2, SkShader::kClamp_TileMode,
                                        SkShader::kClamp_TileMode);
    paint.setShader(s)->unref();
    
    setup_vertexbug(verts, texs, index);
    int indexCount = 6;    // 54
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, 16, verts, texs,
                         NULL, NULL, &index[6], indexCount, paint);

#if 0
    paint.setShader(NULL);
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, 16, verts, NULL,
                         NULL, NULL, index, indexCount, paint);
#endif
#else
    SkRect  dst;
    SkIRect margin;
    
    dst.set(SkIntToScalar(10), SkIntToScalar(10),
            SkIntToScalar(100) + scale,
            SkIntToScalar(40) + scale);
    margin.set(9, 9, 9, 9);
    SkNinePatch::DrawNine(canvas, dst, bm2, margin, NULL);
#endif
}

static SkShader* make_shader0(SkIPoint* size) {
    SkBitmap    bm;
    
    SkImageDecoder::DecodeFile("/skimages/logo.gif", &bm);
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

class VerticesView : public SkView {
    SkShader*   fShader0;
    SkShader*   fShader1;

public:    
	VerticesView() {
        SkIPoint    size;
        
        fShader0 = make_shader0(&size);
        fShader1 = make_shader1(size);

        make_strip(&fRecs[0], size.fX, size.fY);
        make_fan(&fRecs[1], size.fX, size.fY);
        make_tris(&fRecs[2]);
        
        fScale = SK_Scalar1;
    }
    
    virtual ~VerticesView() {
        fShader0->safeUnref();
        fShader1->safeUnref();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt))
        {
            SkString str("Vertices");
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }
    
    SkScalar fScale;
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
#if 1
        canvas->drawColor(SK_ColorWHITE);
        canvas->translate(SK_Scalar1/2, SkIntToScalar(15) + SK_Scalar1/2);
        canvas->scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);
        drawbug(canvas, fScale);
        fScale += SK_Scalar1/93;
        this->inval(NULL);
        return;
#endif
        
        SkPaint paint;
        paint.setDither(true);
        paint.setFilterBitmap(true);
        
        for (int i = 0; i < SK_ARRAY_COUNT(fRecs); i++) {
            canvas->save();
            
            paint.setShader(NULL);
            canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                                 fRecs[i].fVerts, fRecs[i].fTexs,
                                 NULL, NULL, NULL, 0, paint);
            
            canvas->translate(SkIntToScalar(250), 0);
            
            paint.setShader(fShader0);
            canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                                 fRecs[i].fVerts, fRecs[i].fTexs,
                                 NULL, NULL, NULL, 0, paint);

            canvas->translate(SkIntToScalar(250), 0);
            
            paint.setShader(fShader1);
            canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                                 fRecs[i].fVerts, fRecs[i].fTexs,
                                 NULL, NULL, NULL, 0, paint);
            canvas->restore();
            
            canvas->translate(0, SkIntToScalar(250));
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) {
    //    fCurrX = click->fICurr.fX;
    //    fCurrY = click->fICurr.fY;
        this->inval(NULL);
        return true;
    }
    
private:
    struct Rec {
        SkCanvas::VertexMode    fMode;
        int                     fCount;
        SkPoint*                fVerts;
        SkPoint*                fTexs;
        
        Rec() : fCount(0), fVerts(NULL), fTexs(NULL) {}
        ~Rec() { delete[] fVerts; delete[] fTexs; }
    };

    void make_tris(Rec* rec) {
        int n = 10;
        SkRandom    rand;
        
        rec->fMode = SkCanvas::kTriangles_VertexMode;
        rec->fCount = n * 3;
        rec->fVerts = new SkPoint[rec->fCount];
        
        for (int i = 0; i < n; i++) {
            SkPoint* v = &rec->fVerts[i*3];
            for (int j = 0; j < 3; j++) {
                v[j].set(rand.nextUScalar1() * 250, rand.nextUScalar1() * 250);
            }
        }
    }
    
    void make_fan(Rec* rec, int texWidth, int texHeight) {
        const SkScalar tx = SkIntToScalar(texWidth);
        const SkScalar ty = SkIntToScalar(texHeight);
        const int n = 24;
        
        rec->fMode = SkCanvas::kTriangleFan_VertexMode;
        rec->fCount = n + 2;
        rec->fVerts = new SkPoint[rec->fCount];
        rec->fTexs  = new SkPoint[rec->fCount];
        
        SkPoint* v = rec->fVerts;
        SkPoint* t = rec->fTexs;

        v[0].set(0, 0);
        t[0].set(0, 0);
        for (int i = 0; i < n; i++) {
            SkScalar cos;
            SkScalar sin = SkScalarSinCos(SK_ScalarPI * 2 * i / n, &cos);
            v[i+1].set(cos, sin);
            t[i+1].set(i*tx/n, ty);
        }
        v[n+1] = v[1];
        t[n+1].set(tx, ty);
        
        SkMatrix m;
        m.setScale(SkIntToScalar(100), SkIntToScalar(100));
        m.postTranslate(SkIntToScalar(110), SkIntToScalar(110));
        m.mapPoints(v, rec->fCount);
    }
    
    void make_strip(Rec* rec, int texWidth, int texHeight) {
        const SkScalar tx = SkIntToScalar(texWidth);
        const SkScalar ty = SkIntToScalar(texHeight);
        const int n = 24;
        
        rec->fMode = SkCanvas::kTriangleStrip_VertexMode;
        rec->fCount = 2 * (n + 1);
        rec->fVerts = new SkPoint[rec->fCount];
        rec->fTexs  = new SkPoint[rec->fCount];
        
        SkPoint* v = rec->fVerts;
        SkPoint* t = rec->fTexs;
        
        for (int i = 0; i < n; i++) {
            SkScalar cos;
            SkScalar sin = SkScalarSinCos(SK_ScalarPI * 2 * i / n, &cos);
            v[i*2 + 0].set(cos/2, sin/2);
            v[i*2 + 1].set(cos, sin);
            
            t[i*2 + 0].set(tx * i / n, ty);
            t[i*2 + 1].set(tx * i / n, 0);
        }
        v[2*n + 0] = v[0];
        v[2*n + 1] = v[1];
        
        t[2*n + 0].set(tx, ty);
        t[2*n + 1].set(tx, 0);
        
        SkMatrix m;
        m.setScale(SkIntToScalar(100), SkIntToScalar(100));
        m.postTranslate(SkIntToScalar(110), SkIntToScalar(110));
        m.mapPoints(v, rec->fCount);
    }
    
    Rec fRecs[3];

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new VerticesView; }
static SkViewRegister reg(MyFactory);

