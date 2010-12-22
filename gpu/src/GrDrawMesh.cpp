#include "GrMesh.h"
#include "SkCanvas.h"

GrMesh::GrMesh() : fPts(NULL), fCount(0), fIndices(NULL), fIndexCount(0) {}

GrMesh::~GrMesh() {
    delete[] fPts;
    delete[] fIndices;
}

GrMesh& GrMesh::operator=(const GrMesh& src) {
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

void GrMesh::init(const SkRect& bounds, int rows, int cols,
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

void GrMesh::draw(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, fCount,
                         fPts, fTex, NULL, NULL, fIndices, fIndexCount,
                         paint);
}

/////////////////////////////////////////////

#include "SkBoundaryPatch.h"
#include "SkMeshUtils.h"

static SkPoint SkPointInterp(const SkPoint& a, const SkPoint& b, SkScalar t) {
    return SkPoint::Make(SkScalarInterp(a.fX, b.fX, t),
                         SkScalarInterp(a.fY, b.fY, t));
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

void test_patch(SkCanvas* canvas, const SkBitmap& bm, SkScalar scale) {
    const float w = bm.width();
    const float h = bm.height();
    SkCubicBoundary cubic;    
    set_cubic(cubic.fPts + 0, 0, 0, w, 0, scale);
    set_cubic(cubic.fPts + 3, w, 0, w, h, scale);
    set_cubic(cubic.fPts + 6, w, h, 0, h, -scale);
    set_cubic(cubic.fPts + 9, 0, h, 0, 0, scale);
    
    SkBoundaryPatch patch;
    patch.setBoundary(&cubic);
    
    const int Rows = 16;
    const int Cols = 16;
    SkPoint pts[Rows * Cols];
    patch.evalPatch(pts, Rows, Cols);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setFilterBitmap(true);

    SkMeshUtils::Draw(canvas, bm, Rows, Cols, pts, NULL, paint);
}


