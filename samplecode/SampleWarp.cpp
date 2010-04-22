#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"

#include "SkBlurMaskFilter.h"
#include "SkTableMaskFilter.h"

#define kNearlyZero     (SK_Scalar1 / 8092)

static void test_bigblur(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorBLACK);

    SkBitmap orig, mask;
    SkImageDecoder::DecodeFile("/skimages/app_icon.png", &orig);

    SkMaskFilter* mf = SkBlurMaskFilter::Create(8, SkBlurMaskFilter::kNormal_BlurStyle);
    SkPaint paint;
    paint.setMaskFilter(mf)->unref();
    SkIPoint offset;
    orig.extractAlpha(&mask, &paint, &offset);

    paint.setColor(0xFFBB8800);
    paint.setColor(SK_ColorWHITE);

    int i;
    canvas->save();
    float gamma = 0.8;
    for (i = 0; i < 5; i++) {
        paint.setMaskFilter(SkTableMaskFilter::CreateGamma(gamma))->unref();
        canvas->drawBitmap(mask, 0, 0, &paint);
        paint.setMaskFilter(NULL);
        canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
        gamma -= 0.1;
        canvas->translate(120, 0);
    }
    canvas->restore();
    canvas->translate(0, 160);

    for (i = 0; i < 5; i++) {
        paint.setMaskFilter(SkTableMaskFilter::CreateClip(i*30, 255 - 20))->unref();
        canvas->drawBitmap(mask, 0, 0, &paint);
        paint.setMaskFilter(NULL);
        canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
        canvas->translate(120, 0);
    }

#if 0
    paint.setColor(0xFFFFFFFF);
    canvas->drawBitmap(mask, 0, 0, &paint);
    paint.setMaskFilter(NULL);
    canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
    
    canvas->translate(120, 0);
    
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
    
    canvas->translate(120, 0);
    
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
    
    canvas->translate(120, 0);
    
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
    
    canvas->translate(120, 0);
    
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(mask, 0, 0, &paint);
    canvas->drawBitmap(orig, -offset.fX, -offset.fY, &paint);
#endif
}

#include "SkMeshUtils.h"

static SkPoint SkMakePoint(SkScalar x, SkScalar y) {
    SkPoint pt;
    pt.set(x, y);
    return pt;
}

static SkPoint SkPointInterp(const SkPoint& a, const SkPoint& b, SkScalar t) {
    return SkMakePoint(SkScalarInterp(a.fX, b.fX, t),
                       SkScalarInterp(a.fY, b.fY, t));
}

#include "SkBoundaryPatch.h"

static void set_cubic(SkPoint pts[4], SkScalar x0, SkScalar y0,
                      SkScalar x3, SkScalar y3, SkScalar scale = 1) {
    SkPoint tmp, tmp2;

    pts[0].set(x0, y0);
    pts[3].set(x3, y3);
    
    tmp = SkPointInterp(pts[0], pts[3], SK_Scalar1/3);
    tmp2 = pts[0] - tmp;
    tmp2.rotateCW();
    tmp2.scale(scale);
    pts[1] = tmp + tmp2;
    
    tmp = SkPointInterp(pts[0], pts[3], 2*SK_Scalar1/3);
    tmp2 = pts[3] - tmp;
    tmp2.rotateCW();
    tmp2.scale(scale);
    pts[2] = tmp + tmp2;
}

static void test_patch(SkCanvas* canvas, const SkBitmap& bm, SkScalar scale) {
    SkCubicBoundary cubic;    
    set_cubic(cubic.fPts + 0, 0, 0, 100, 0, scale);
    set_cubic(cubic.fPts + 3, 100, 0, 100, 100, scale);
    set_cubic(cubic.fPts + 6, 100, 100,  0, 100, -scale);
    set_cubic(cubic.fPts + 9, 0, 100, 0, 0, 0);
    
    SkBoundaryPatch patch;
    patch.setBoundary(&cubic);
    
    const int Rows = 16;
    const int Cols = 16;
    SkPoint pts[Rows * Cols];
    patch.evalPatch(pts, Rows, Cols);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setFilterBitmap(true);
    paint.setStrokeWidth(1);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    
    canvas->translate(50, 50);
    canvas->scale(3, 3);
    
    SkMeshUtils::Draw(canvas, bm, Rows, Cols, pts, NULL, paint);
}

static void test_drag(SkCanvas* canvas, const SkBitmap& bm,
                      const SkPoint& p0, const SkPoint& p1) {
    SkCubicBoundary cubic;    
    set_cubic(cubic.fPts + 0, 0, 0, 100, 0, 0);
    set_cubic(cubic.fPts + 3, 100, 0, 100, 100, 0);
    set_cubic(cubic.fPts + 6, 100, 100,  0, 100, 0);
    set_cubic(cubic.fPts + 9, 0, 100, 0, 0, 0);
    
#if 0
    cubic.fPts[1] += p1 - p0;
    cubic.fPts[2] += p1 - p0;
#else
    SkScalar dx = p1.fX - p0.fX;
    if (dx > 0) dx = 0;
    SkScalar dy = p1.fY - p0.fY;
    if (dy > 0) dy = 0;

    cubic.fPts[1].fY += dy;
    cubic.fPts[2].fY += dy;
    cubic.fPts[10].fX += dx;
    cubic.fPts[11].fX += dx;
#endif

    SkBoundaryPatch patch;
    patch.setBoundary(&cubic);
    
    const int Rows = 16;
    const int Cols = 16;
    SkPoint pts[Rows * Cols];
    patch.evalPatch(pts, Rows, Cols);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setFilterBitmap(true);
    paint.setStrokeWidth(1);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    
    canvas->translate(50, 50);
    canvas->scale(3, 3);
    
    SkAutoCanvasRestore acr(canvas, true);

    SkRect r = { 0, 0, 100, 100 };
    canvas->clipRect(r);
    SkMeshUtils::Draw(canvas, bm, Rows, Cols, pts, NULL, paint);
}

///////////////////////////////////////////////////////////////////////////////

class Mesh {
public:
    Mesh();
    ~Mesh();

    Mesh& operator=(const Mesh& src);

    void init(const SkRect& bounds, int rows, int cols,
              const SkRect& texture);

    const SkRect& bounds() const { return fBounds; }

    int rows() const { return fRows; }
    int cols() const { return fCols; }
    SkPoint& pt(int row, int col) {
        return fPts[row * (fRows + 1) + col];
    }

    void draw(SkCanvas*, const SkPaint&);
    void drawWireframe(SkCanvas* canvas, const SkPaint& paint);

private:
    SkRect      fBounds;
    int         fRows, fCols;
    SkPoint*    fPts;
    SkPoint*    fTex;   // just points into fPts, not separately allocated
    int         fCount;
    uint16_t*   fIndices;
    int         fIndexCount;
};

Mesh::Mesh() : fPts(NULL), fCount(0), fIndices(NULL), fIndexCount(0) {}

Mesh::~Mesh() {
    delete[] fPts;
    delete[] fIndices;
}

Mesh& Mesh::operator=(const Mesh& src) {
    delete[] fPts;
    delete[] fIndices;

    fBounds = src.fBounds;
    fRows = src.fRows;
    fCols = src.fCols;

    fCount = src.fCount;
    fPts = new SkPoint[fCount * 2];
    fTex = fPts + fCount;
    memcpy(fPts, src.fPts, fCount * 2 * sizeof(SkPoint));
    
    delete[] fIndices;
    fIndexCount = src.fIndexCount;
    fIndices = new uint16_t[fIndexCount];
    memcpy(fIndices, src.fIndices, fIndexCount * sizeof(uint16_t));

    return *this;
}

void Mesh::init(const SkRect& bounds, int rows, int cols,
                const SkRect& texture) {
    SkASSERT(rows > 0 && cols > 0);

    fBounds = bounds;
    fRows = rows;
    fCols = cols;

    delete[] fPts;
    fCount = (rows + 1) * (cols + 1);
    fPts = new SkPoint[fCount * 2];
    fTex = fPts + fCount;

    delete[] fIndices;
    fIndexCount = rows * cols * 6;
    fIndices = new uint16_t[fIndexCount];
    
    SkPoint* pts = fPts;
    const SkScalar dx = bounds.width() / rows;
    const SkScalar dy = bounds.height() / cols;
    SkPoint* tex = fTex;
    const SkScalar dtx = texture.width() / rows;
    const SkScalar dty = texture.height() / cols;
    uint16_t* idx = fIndices;
    int index = 0;
    for (int y = 0; y <= cols; y++) {
        for (int x = 0; x <= rows; x++) {
            pts->set(bounds.fLeft + x*dx, bounds.fTop + y*dy);
            pts += 1;
            tex->set(texture.fLeft + x*dtx, texture.fTop + y*dty);
            tex += 1;
            
            if (y < cols && x < rows) {
                *idx++ = index;
                *idx++ = index + rows + 1;
                *idx++ = index + 1;

                *idx++ = index + 1;
                *idx++ = index + rows + 1;
                *idx++ = index + rows + 2;
                
                index += 1;
            }
        }
        index += 1;
    }
}

void Mesh::draw(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, fCount,
                         fPts, fTex, NULL, NULL, fIndices, fIndexCount,
                         paint);
}

void Mesh::drawWireframe(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, fCount,
                         fPts, NULL, NULL, NULL, fIndices, fIndexCount,
                         paint);
}

///////////////////////////////////////////////////////////////////////////////

class WarpView : public SkView {
    Mesh        fMesh, fOrig;
    SkBitmap    fBitmap;
    SkMatrix    fMatrix, fInverse;
public:
	WarpView() {
        SkBitmap bm;
//        SkImageDecoder::DecodeFile("/skimages/marker.png", &bm);
        SkImageDecoder::DecodeFile("/skimages/logo.gif", &bm);
   //     SkImageDecoder::DecodeFile("/beach_shot.JPG", &bm);
        fBitmap = bm;
        
        SkRect bounds, texture;
        texture.set(0, 0, SkIntToScalar(fBitmap.width()),
                    SkIntToScalar(fBitmap.height()));
        bounds = texture;
        
//        fMesh.init(bounds, fBitmap.width() / 40, fBitmap.height() / 40, texture);
        fMesh.init(bounds, fBitmap.width()/16, fBitmap.height()/16, texture);
        fOrig = fMesh;
        
        fP0.set(0, 0);
        fP1 = fP0;

        fMatrix.setScale(2, 2);
        fMatrix.invert(&fInverse);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Warp");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    static SkPoint apply_warp(const SkVector& drag, SkScalar dragLength,
                              const SkPoint& dragStart, const SkPoint& dragCurr,
                              const SkPoint& orig) {
        SkVector delta = orig - dragCurr;
        SkScalar length = SkPoint::Normalize(&delta);
        if (length <= kNearlyZero) {
            return orig;
        }
        
        const SkScalar period = 20;
        const SkScalar mag = dragLength / 3;
        
        SkScalar d = length / (period);
        d = mag * SkScalarSin(d) / d;
        SkScalar dx = delta.fX * d;
        SkScalar dy = delta.fY * d;
        SkScalar px = orig.fX + dx;
        SkScalar py = orig.fY + dy;
        return SkPoint::Make(px, py);
    }
    
    static SkPoint apply_warp2(const SkVector& drag, SkScalar dragLength,
                              const SkPoint& dragStart, const SkPoint& dragCurr,
                              const SkPoint& orig) {
        SkVector delta = orig - dragCurr;
        SkScalar length = SkPoint::Normalize(&delta);
        if (length <= kNearlyZero) {
            return orig;
        }
        
        const SkScalar period = 10 + dragLength/4;
        const SkScalar mag = dragLength / 3;
        
        SkScalar d = length / (period);
        if (d > SK_ScalarPI) {
            d = SK_ScalarPI;
        }

        d = -mag * SkScalarSin(d);
        
        SkScalar dx = delta.fX * d;
        SkScalar dy = delta.fY * d;
        SkScalar px = orig.fX + dx;
        SkScalar py = orig.fY + dy;
        return SkPoint::Make(px, py);
    }
    
    typedef SkPoint (*WarpProc)(const SkVector& drag, SkScalar dragLength,
                             const SkPoint& dragStart, const SkPoint& dragCurr,
                             const SkPoint& orig);

    void warp(const SkPoint& p0, const SkPoint& p1) {
        WarpProc proc = apply_warp2;
        SkPoint delta = p1 - p0;
        SkScalar length = SkPoint::Normalize(&delta);
        for (int y = 0; y < fMesh.rows(); y++) {
            for (int x = 0; x < fMesh.cols(); x++) {
                fMesh.pt(x, y) = proc(delta, length, p0, p1, fOrig.pt(x, y));
            }
        }
        fP0 = p0;
        fP1 = p1;
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorLTGRAY);
     //   test_bigblur(canvas); return;
        
        canvas->concat(fMatrix);

        SkPaint paint;
        paint.setFilterBitmap(true);
        paint.setShader(SkShader::CreateBitmapShader(fBitmap,
                                                     SkShader::kClamp_TileMode,
                                                     SkShader::kClamp_TileMode))->unref();
        fMesh.draw(canvas, paint); //return;
        
        paint.setShader(NULL);
        paint.setColor(SK_ColorRED);
        fMesh.draw(canvas, paint);

    //    test_drag(canvas, fBitmap, fP0, fP1);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) {
        SkPoint pts[2] = { click->fOrig, click->fCurr };
        fInverse.mapPoints(pts, 2);
        this->warp(pts[0], pts[1]);
        this->inval(NULL);
        return true;
    }
    
private:
    SkIRect    fBase, fRect;
    SkPoint     fP0, fP1;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new WarpView; }
static SkViewRegister reg(MyFactory);

