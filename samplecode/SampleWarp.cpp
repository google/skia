#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"

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

static SkScalar gScale = 0;
static SkScalar gDScale = 0.02;

class WarpView : public SkView {
    Mesh        fMesh, fOrig;
    SkBitmap    fBitmap;
public:
	WarpView() {
        SkBitmap bm;
        SkImageDecoder::DecodeFile("/skimages/beach.jpg", &bm);
   //     SkImageDecoder::DecodeFile("/beach_shot.JPG", &bm);
        fBitmap = bm;
        
        SkRect bounds, texture;
        texture.set(0, 0, SkIntToScalar(fBitmap.width()),
                    SkIntToScalar(fBitmap.height()));
        bounds = texture;
        
//        fMesh.init(bounds, fBitmap.width() / 40, fBitmap.height() / 40, texture);
        fMesh.init(bounds, 30, 30, texture);
        fOrig = fMesh;
        
        fP0.set(0, 0);
        fP1 = fP0;
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
    
    void warp(const SkPoint& p0, const SkPoint& p1) {
        fP0 = p0;
        fP1 = p1;
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
        
        SkPaint paint;
        paint.setFilterBitmap(true);
        paint.setShader(SkShader::CreateBitmapShader(fBitmap,
                                                     SkShader::kClamp_TileMode,
                                                     SkShader::kClamp_TileMode))->unref();
     //   fMesh.draw(canvas, paint);
        
        paint.setShader(NULL);
        paint.setColor(SK_ColorRED);
    //    fMesh.draw(canvas, paint);

#if 0
        test_patch(canvas, fBitmap, gScale);
        gScale += gDScale;
        if (gScale > 2) {
            gDScale = -gDScale;
        } else if (gScale < -2) {
            gDScale = -gDScale;
        }
        this->inval(NULL);
#else
        test_drag(canvas, fBitmap, fP0, fP1);
#endif
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) {
        this->warp(click->fOrig, click->fCurr);
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

