#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"


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
public:
	WarpView() {
        SkBitmap bm;
        SkImageDecoder::DecodeFile("/skimages/nytimes.png", &bm);
        SkIRect subset = { 0, 0, 420, 420 };
        bm.extractSubset(&fBitmap, subset);
        
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
        fMesh.draw(canvas, paint);
        
        paint.setShader(NULL);
        paint.setColor(SK_ColorRED);
    //    fMesh.draw(canvas, paint);
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

