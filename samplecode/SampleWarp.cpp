#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"

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

static void set_pts(SkPoint pts[], int R, int C, SkBoundaryPatch* patch) {
    SkScalar invR = SkScalarInvert(SkIntToScalar(R - 1));
    SkScalar invC = SkScalarInvert(SkIntToScalar(C - 1));

    for (int y = 0; y < C; y++) {
        SkScalar yy = y * invC;
        for (int x = 0; x < R; x++) {
            *pts++ = patch->evaluate(x * invR, yy);
        }
    }
}

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

static void draw_texture(SkCanvas* canvas, const SkPoint verts[], int R, int C,
                         const SkBitmap& texture) {
    int vertCount = R * C;
    const int rows = R - 1;
    const int cols = C - 1;
    int idxCount = rows * cols * 6;

    SkAutoTArray<SkPoint> texStorage(vertCount);
    SkPoint* tex = texStorage.get();
    SkAutoTArray<uint16_t> idxStorage(idxCount);
    uint16_t* idx = idxStorage.get();


    const SkScalar dtx = texture.width() / rows;
    const SkScalar dty = texture.height() / cols;
    int index = 0;
    for (int y = 0; y <= cols; y++) {
        for (int x = 0; x <= rows; x++) {
            tex->set(x*dtx, y*dty);
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

    SkPaint paint;
    paint.setShader(SkShader::CreateBitmapShader(texture,
                                                 SkShader::kClamp_TileMode,
                                                 SkShader::kClamp_TileMode))->unref();

    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, vertCount, verts,
                         texStorage.get(), NULL, NULL, idxStorage.get(),
                         idxCount, paint);                         
}

static void test_patch(SkCanvas* canvas, const SkBitmap& bm, SkScalar scale) {
    SkCubicBoundaryCurve L, T, R, B;
    
    set_cubic(L.fPts, 0, 0, 0, 100, scale);
    set_cubic(T.fPts, 0, 0, 100, 0, scale);
    set_cubic(R.fPts, 100, 0, 100, 100, -scale);
    set_cubic(B.fPts, 0, 100, 100, 100, 0);

    SkBoundaryPatch patch;
    patch.setCurve(SkBoundaryPatch::kLeft, &L);
    patch.setCurve(SkBoundaryPatch::kTop, &T);
    patch.setCurve(SkBoundaryPatch::kRight, &R);
    patch.setCurve(SkBoundaryPatch::kBottom, &B);

    const int Rows = 25;
    const int Cols = 25;
    SkPoint pts[Rows * Cols];
    set_pts(pts, Rows, Cols, &patch);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    canvas->translate(50, 50);
    canvas->scale(3, 3);

    draw_texture(canvas, pts, Rows, Cols, bm);
//    canvas->drawPoints(SkCanvas::kPoints_PointMode, SK_ARRAY_COUNT(pts),
//                       pts, paint);
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
static SkScalar gDScale = 0.01;

class WarpView : public SkView {
    Mesh        fMesh, fOrig;
    SkBitmap    fBitmap;
public:
	WarpView() {
        SkBitmap bm;
   //     SkImageDecoder::DecodeFile("/skimages/beach.jpg", &bm);
        SkImageDecoder::DecodeFile("/beach_shot.JPG", &bm);
        fBitmap = bm;
        
        SkRect bounds, texture;
        texture.set(0, 0, SkIntToScalar(fBitmap.width()),
                    SkIntToScalar(fBitmap.height()));
        bounds = texture;
        
//        fMesh.init(bounds, fBitmap.width() / 40, fBitmap.height() / 40, texture);
        fMesh.init(bounds, 30, 30, texture);
        fOrig = fMesh;
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
    
    static SkPoint make_pt(SkScalar x, SkScalar y) {
        SkPoint pt;
        pt.set(x, y);
        return pt;
    }

    static SkScalar mapx0(SkScalar min, SkScalar max, SkScalar x0, SkScalar x1,
                         SkScalar x) {
        if (x < x0) {
            SkASSERT(x0 > min);
            return x1 - SkScalarMulDiv(x1 - min, x0 - x, x0 - min);
        } else {
            SkASSERT(max > x0);
            return x1 + SkScalarMulDiv(max - x1, x - x0, max - x0);
        }
    }
    
    static SkScalar mapx1(SkScalar min, SkScalar max, SkScalar x0, SkScalar x1,
                         SkScalar x) {
        SkScalar newx;
        if (x < x0) {
            SkASSERT(x0 > min);
            newx = x1 - SkScalarMulDiv(x1 - min, x0 - x, x0 - min);
        } else {
            SkASSERT(max > x0);
            newx = x1 + SkScalarMulDiv(max - x1, x - x0, max - x0);
        }
        return x + (newx - x) * 0.5f;
    }
    
    static SkPoint mappt(const SkRect& r, const SkPoint& p0, const SkPoint& p1,
                         const SkPoint& pt) {
        return make_pt(mapx0(r.fLeft, r.fRight, p0.fX, p1.fX, pt.fX),
                       mapx0(r.fTop, r.fBottom, p0.fY, p1.fY, pt.fY));
    }
    
    void warp(const SkPoint& p0, const SkPoint& p1) {
        const SkRect& bounds = fOrig.bounds();
        int rows = fMesh.rows();
        int cols = fMesh.cols();
        
        SkRect r = bounds;
        r.inset(bounds.width() / 256, bounds.height() / 256);
        if (r.contains(p0)) {
            for (int y = 1; y < cols; y++) {
                for (int x = 1; x < rows; x++) {
                    fMesh.pt(x, y) = mappt(bounds, p0, p1, fOrig.pt(x, y));
                }
            }
        }
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
        
        test_patch(canvas, fBitmap, gScale);
        gScale += gDScale;
        if (gScale > 2) {
            gDScale = -gDScale;
        } else if (gScale < -2) {
            gDScale = -gDScale;
        }
        this->inval(NULL);
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
    
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new WarpView; }
static SkViewRegister reg(MyFactory);

