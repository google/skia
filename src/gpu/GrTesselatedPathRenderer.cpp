
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTesselatedPathRenderer.h"

#include "GrDrawState.h"
#include "GrPathUtils.h"
#include "GrPoint.h"
#include "GrTDArray.h"

#include "SkTemplates.h"

#include <limits.h>
#include <sk_glu.h>

typedef GrTDArray<GrDrawState::Edge> GrEdgeArray;
typedef GrTDArray<GrPoint> GrPointArray;
typedef GrTDArray<uint16_t> GrIndexArray;
typedef void (*TESSCB)();

// limit the allowable vertex range to approximately half of the representable
// IEEE exponent in order to avoid overflow when doing multiplies between
// vertex components,
const float kMaxVertexValue = 1e18f;

static inline GrDrawState::Edge computeEdge(const GrPoint& p,
                                             const GrPoint& q,
                                             float sign) {
    GrVec tangent = GrVec::Make(p.fY - q.fY, q.fX - p.fX);
    float scale = sign / tangent.length();
    float cross2 = p.fX * q.fY - q.fX * p.fY;
    return GrDrawState::Edge(tangent.fX * scale,
                              tangent.fY * scale,
                              cross2 * scale);
}

static inline GrPoint sanitizePoint(const GrPoint& pt) {
    GrPoint r;
    r.fX = SkScalarPin(pt.fX, -kMaxVertexValue, kMaxVertexValue);
    r.fY = SkScalarPin(pt.fY, -kMaxVertexValue, kMaxVertexValue);
    return r;
}

class GrTess {
public:
    GrTess(int count, unsigned winding_rule) {
        fTess = Sk_gluNewTess();
        Sk_gluTessProperty(fTess, GLU_TESS_WINDING_RULE, winding_rule);
        Sk_gluTessNormal(fTess, 0.0f, 0.0f, 1.0f);
        Sk_gluTessCallback(fTess, GLU_TESS_BEGIN_DATA, (TESSCB) &beginCB);
        Sk_gluTessCallback(fTess, GLU_TESS_VERTEX_DATA, (TESSCB) &vertexCB);
        Sk_gluTessCallback(fTess, GLU_TESS_END_DATA, (TESSCB) &endCB);
        Sk_gluTessCallback(fTess, GLU_TESS_EDGE_FLAG_DATA, (TESSCB) &edgeFlagCB);
        Sk_gluTessCallback(fTess, GLU_TESS_COMBINE_DATA, (TESSCB) &combineCB);
        fInVertices = new double[count * 3];
    }
    ~GrTess() {
        Sk_gluDeleteTess(fTess);
        delete[] fInVertices;
    }
    void addVertex(const GrPoint& pt, int index) {
        if (index > USHRT_MAX) return;
        double* inVertex = &fInVertices[index * 3];
        inVertex[0] = pt.fX;
        inVertex[1] = pt.fY;
        inVertex[2] = 0.0;
        *fVertices.append() = pt;
        Sk_gluTessVertex(fTess, inVertex, reinterpret_cast<void*>(index));
    }
    void addVertices(const GrPoint* points, const uint16_t* contours, int numContours) {
        Sk_gluTessBeginPolygon(fTess, this);
        size_t i = 0;
        for (int j = 0; j < numContours; ++j) {
            Sk_gluTessBeginContour(fTess);
            size_t end = i + contours[j];
            for (; i < end; ++i) {
                addVertex(points[i], i);
            }
            Sk_gluTessEndContour(fTess);
        }
        Sk_gluTessEndPolygon(fTess);
    }
    GLUtesselator* tess() { return fTess; }
    const GrPointArray& vertices() const { return fVertices; }
protected:
    virtual void begin(GLenum type) = 0;
    virtual void vertex(int index) = 0;
    virtual void edgeFlag(bool flag) = 0;
    virtual void end() = 0;
    virtual int combine(GLdouble coords[3], int vertexIndices[4], 
                         GLfloat weight[4]) = 0;
    static void beginCB(GLenum type, void* data) {
        static_cast<GrTess*>(data)->begin(type);
    }
    static void vertexCB(void* vertexData, void* data) {
        static_cast<GrTess*>(data)->vertex(reinterpret_cast<long>(vertexData));
    }
    static void edgeFlagCB(GLboolean flag, void* data) {
        static_cast<GrTess*>(data)->edgeFlag(flag != 0);
    }
    static void endCB(void* data) {
        static_cast<GrTess*>(data)->end();
    }
    static void combineCB(GLdouble coords[3], void* vertexData[4],
                          GLfloat weight[4], void **outData, void* data) {
        int vertexIndex[4];
        vertexIndex[0] = reinterpret_cast<long>(vertexData[0]);
        vertexIndex[1] = reinterpret_cast<long>(vertexData[1]);
        vertexIndex[2] = reinterpret_cast<long>(vertexData[2]);
        vertexIndex[3] = reinterpret_cast<long>(vertexData[3]);
        GrTess* tess = static_cast<GrTess*>(data);
        int outIndex = tess->combine(coords, vertexIndex, weight);
        *reinterpret_cast<long*>(outData) = outIndex;
    }
protected:
    GLUtesselator* fTess;
    GrPointArray fVertices;
    double* fInVertices;
};

class GrPolygonTess : public GrTess {
public:
    GrPolygonTess(int count, unsigned winding_rule)
      : GrTess(count, winding_rule) {
    }
    ~GrPolygonTess() {
    }
    const GrIndexArray& indices() const { return fIndices; }
protected:
    virtual void begin(GLenum type) {
        GR_DEBUGASSERT(type == GL_TRIANGLES);
    }
    virtual void vertex(int index) {
        *fIndices.append() = index;
    }
    virtual void edgeFlag(bool flag) {}
    virtual void end() {}
    virtual int combine(GLdouble coords[3], int vertexIndices[4],
                         GLfloat weight[4]) {
        int index = fVertices.count();
        GrPoint p = GrPoint::Make(static_cast<float>(coords[0]),
                                  static_cast<float>(coords[1]));
        *fVertices.append() = p;
        return index;
    }
protected:
    GrIndexArray fIndices;
};

class GrEdgePolygonTess : public GrPolygonTess {
public:
    GrEdgePolygonTess(int count, unsigned winding_rule, const SkMatrix& matrix)
      : GrPolygonTess(count, winding_rule),
        fMatrix(matrix),
        fEdgeFlag(false),
        fEdgeVertex(-1),
        fTriStartVertex(-1),
        fEdges(NULL) {
    }
    ~GrEdgePolygonTess() {
        delete[] fEdges;
    }
    const GrDrawState::Edge* edges() const { return fEdges; }
private:
    void addEdge(int index0, int index1) {
        GrPoint p = fVertices[index0];
        GrPoint q = fVertices[index1];
        fMatrix.mapPoints(&p, 1);
        fMatrix.mapPoints(&q, 1);
        p = sanitizePoint(p);
        q = sanitizePoint(q);
        if (p == q) return;
        GrDrawState::Edge edge = computeEdge(p, q, 1.0f);
        fEdges[index0 * 2 + 1] = edge;
        fEdges[index1 * 2] = edge;
    }
    virtual void begin(GLenum type) {
        GR_DEBUGASSERT(type == GL_TRIANGLES);
        int count = fVertices.count() * 2;
        fEdges = new GrDrawState::Edge[count];
        memset(fEdges, 0, count * sizeof(GrDrawState::Edge));
    }
    virtual void edgeFlag(bool flag) {
        fEdgeFlag = flag;
    }
    virtual void vertex(int index) {
        bool triStart = fIndices.count() % 3 == 0;
        GrPolygonTess::vertex(index);
        if (fEdgeVertex != -1) {
            if (triStart) {
                addEdge(fEdgeVertex, fTriStartVertex);
            } else {
                addEdge(fEdgeVertex, index);
            }
        }
        if (triStart) {
            fTriStartVertex = index;
        }
        if (fEdgeFlag) {
            fEdgeVertex = index;
        } else {
            fEdgeVertex = -1;
        }
    }
    virtual void end() {
        if (fEdgeVertex != -1) {
            addEdge(fEdgeVertex, fTriStartVertex);
        }
    }
    GrMatrix fMatrix;
    bool fEdgeFlag;
    int fEdgeVertex, fTriStartVertex;
    GrDrawState::Edge* fEdges;
};

class GrBoundaryTess : public GrTess {
public:
    GrBoundaryTess(int count, unsigned winding_rule)
      : GrTess(count, winding_rule),
        fContourStart(0) {
        Sk_gluTessProperty(fTess, GLU_TESS_BOUNDARY_ONLY, 1);
    }
    ~GrBoundaryTess() {
    }
    GrPointArray& contourPoints() { return fContourPoints; }
    const GrIndexArray& contours() const { return fContours; }
private:
    virtual void begin(GLenum type) {
        fContourStart = fContourPoints.count();
    }
    virtual void vertex(int index) {
        *fContourPoints.append() = fVertices.at(index);
    }
    virtual void edgeFlag(bool flag) {}
    virtual void end() {
        *fContours.append() = fContourPoints.count() - fContourStart;
    }
    virtual int combine(GLdouble coords[3], int vertexIndices[4],
                        GLfloat weight[4]) {
        int index = fVertices.count();
        *fVertices.append() = GrPoint::Make(static_cast<float>(coords[0]),
                                            static_cast<float>(coords[1]));
        return index;
    }
    GrPointArray fContourPoints;
    GrIndexArray fContours;
    size_t fContourStart;
};

static bool nearlyEqual(float a, float b) {
    return fabsf(a - b) < 0.0001f;
}

static bool nearlyEqual(const GrPoint& a, const GrPoint& b) {
    return nearlyEqual(a.fX, b.fX) && nearlyEqual(a.fY, b.fY);
}

static bool parallel(const GrDrawState::Edge& a, const GrDrawState::Edge& b) {
    return (nearlyEqual(a.fX, b.fX) && nearlyEqual(a.fY, b.fY)) ||
           (nearlyEqual(a.fX, -b.fX) && nearlyEqual(a.fY, -b.fY));
}

static unsigned fill_type_to_glu_winding_rule(GrPathFill fill) {
    switch (fill) {
        case kWinding_PathFill:
            return GLU_TESS_WINDING_NONZERO;
        case kEvenOdd_PathFill:
            return GLU_TESS_WINDING_ODD;
        case kInverseWinding_PathFill:
            return GLU_TESS_WINDING_POSITIVE;
        case kInverseEvenOdd_PathFill:
            return GLU_TESS_WINDING_ODD;
        case kHairLine_PathFill:
            return GLU_TESS_WINDING_NONZERO;  // FIXME:  handle this
        default:
            GrAssert(!"Unknown path fill!");
            return 0;
    }
}

GrTesselatedPathRenderer::GrTesselatedPathRenderer() {
}

static bool isCCW(const GrPoint* pts, int count) {
    GrVec v1, v2;
    do {
        v1 = pts[1] - pts[0];
        v2 = pts[2] - pts[1];
        pts++;
        count--;
    } while (nearlyEqual(v1, v2) && count > 3);
    return v1.cross(v2) < 0;
}

static bool validEdge(const GrDrawState::Edge& edge) {
    return !(edge.fX == 0.0f && edge.fY == 0.0f && edge.fZ == 0.0f);
}

static size_t computeEdgesAndIntersect(const GrMatrix& matrix,
                                       const GrMatrix& inverse,
                                       GrPoint* vertices,
                                       size_t numVertices,
                                       GrEdgeArray* edges,
                                       float sign) {
    if (numVertices < 3) {
        return 0;
    }
    matrix.mapPoints(vertices, numVertices);
    if (sign == 0.0f) {
        sign = isCCW(vertices, numVertices) ? -1.0f : 1.0f;
    }
    GrPoint p = sanitizePoint(vertices[numVertices - 1]);
    for (size_t i = 0; i < numVertices; ++i) {
        GrPoint q = sanitizePoint(vertices[i]);
        if (p == q) {
            continue;
        }
        GrDrawState::Edge edge = computeEdge(p, q, sign);
        edge.fZ += 0.5f;    // Offset by half a pixel along the tangent.
        *edges->append() = edge;
        p = q;
    }
    int count = edges->count();
    if (count == 0) {
        return 0;
    }
    GrDrawState::Edge prev_edge = edges->at(0);
    for (int i = 0; i < count; ++i) {
        GrDrawState::Edge edge = edges->at(i < count - 1 ? i + 1 : 0);
        if (parallel(edge, prev_edge)) {
            // 3 points are collinear; offset by half the tangent instead
            vertices[i].fX -= edge.fX * 0.5f;
            vertices[i].fY -= edge.fY * 0.5f;
        } else {
            vertices[i] = prev_edge.intersect(edge);
        }
        inverse.mapPoints(&vertices[i], 1);
        prev_edge = edge;
    }
    return edges->count();
}

void GrTesselatedPathRenderer::drawPath(GrDrawTarget::StageBitfield stages) {
    GrDrawTarget::AutoStateRestore asr(fTarget);
    // face culling doesn't make sense here
    GrAssert(GrDrawState::kBoth_DrawFace == fTarget->getDrawFace());

    GrMatrix viewM = fTarget->getViewMatrix();

    GrScalar tol = GR_Scalar1;
    tol = GrPathUtils::scaleToleranceToSrc(tol, viewM, fPath->getBounds());
    GrScalar tolSqd = GrMul(tol, tol);

    int subpathCnt;
    int maxPts = GrPathUtils::worstCasePointCount(*fPath, &subpathCnt, tol);

    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    bool inverted = GrIsFillInverted(fFill);
    if (inverted) {
        maxPts += 4;
        subpathCnt++;
    }
    if (maxPts > USHRT_MAX) {
        return;
    }
    SkAutoSTMalloc<8, GrPoint> baseMem(maxPts);
    GrPoint* base = baseMem;
    GrPoint* vert = base;
    GrPoint* subpathBase = base;

    SkAutoSTMalloc<8, uint16_t> subpathVertCount(subpathCnt);

    GrPoint pts[4];
    SkPath::Iter iter(*fPath, false);

    bool first = true;
    int subpath = 0;

    for (;;) {
        switch (iter.next(pts)) {
            case kMove_PathCmd:
                if (!first) {
                    subpathVertCount[subpath] = vert-subpathBase;
                    subpathBase = vert;
                    ++subpath;
                }
                *vert = pts[0];
                vert++;
                break;
            case kLine_PathCmd:
                *vert = pts[1];
                vert++;
                break;
            case kQuadratic_PathCmd: {
                GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     tolSqd, &vert,
                                                     GrPathUtils::quadraticPointCount(pts, tol));
                break;
            }
            case kCubic_PathCmd: {
                GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
                                                 tolSqd, &vert,
                                                 GrPathUtils::cubicPointCount(pts, tol));
                break;
            }
            case kClose_PathCmd:
                break;
            case kEnd_PathCmd:
                subpathVertCount[subpath] = vert-subpathBase;
                ++subpath; // this could be only in debug
                goto FINISHED;
        }
        first = false;
    }
FINISHED:
    if (0 != fTranslate.fX || 0 != fTranslate.fY) {
        for (int i = 0; i < vert - base; i++) {
            base[i].offset(fTranslate.fX, fTranslate.fY);
        }
    }

    if (inverted) {
        GrRect bounds;
        GrAssert(NULL != fTarget->getRenderTarget());
        bounds.setLTRB(0, 0,
                       GrIntToScalar(fTarget->getRenderTarget()->width()),
                       GrIntToScalar(fTarget->getRenderTarget()->height()));
        GrMatrix vmi;
        if (fTarget->getViewInverse(&vmi)) {
            vmi.mapRect(&bounds);
        }
        *vert++ = GrPoint::Make(bounds.fLeft, bounds.fTop);
        *vert++ = GrPoint::Make(bounds.fLeft, bounds.fBottom);
        *vert++ = GrPoint::Make(bounds.fRight, bounds.fBottom);
        *vert++ = GrPoint::Make(bounds.fRight, bounds.fTop);
        subpathVertCount[subpath++] = 4;
    }

    GrAssert(subpath == subpathCnt);
    GrAssert((vert - base) <= maxPts);

    size_t count = vert - base;

    if (count < 3) {
        return;
    }

    if (subpathCnt == 1 && !inverted && fPath->isConvex()) {
        if (fAntiAlias) {
            GrEdgeArray edges;
            GrMatrix inverse, matrix = fTarget->getViewMatrix();
            fTarget->getViewInverse(&inverse);

            count = computeEdgesAndIntersect(matrix, inverse, base, count, &edges, 0.0f);
            size_t maxEdges = fTarget->getMaxEdges();
            if (count == 0) {
                return;
            }
            if (count <= maxEdges) {
                // All edges fit; upload all edges and draw all verts as a fan
                fTarget->setVertexSourceToArray(layout, base, count);
                fTarget->setEdgeAAData(&edges[0], count);
                fTarget->drawNonIndexed(kTriangleFan_PrimitiveType, 0, count);
            } else {
                // Upload "maxEdges" edges and verts at a time, and draw as
                // separate fans
                for (size_t i = 0; i < count - 2; i += maxEdges - 2) {
                    edges[i] = edges[0];
                    base[i] = base[0];
                    int size = GR_CT_MIN(count - i, maxEdges);
                    fTarget->setVertexSourceToArray(layout, &base[i], size);
                    fTarget->setEdgeAAData(&edges[i], size);
                    fTarget->drawNonIndexed(kTriangleFan_PrimitiveType, 0, size);
                }
            }
            fTarget->setEdgeAAData(NULL, 0);
        } else {
            fTarget->setVertexSourceToArray(layout, base, count);
            fTarget->drawNonIndexed(kTriangleFan_PrimitiveType, 0, count);
        }
        return;
    }

    if (fAntiAlias) {
        // Run the tesselator once to get the boundaries.
        GrBoundaryTess btess(count, fill_type_to_glu_winding_rule(fFill));
        btess.addVertices(base, subpathVertCount, subpathCnt);

        GrMatrix inverse, matrix = fTarget->getViewMatrix();
        if (!fTarget->getViewInverse(&inverse)) {
            return;
        }

        if (btess.vertices().count() > USHRT_MAX) {
            return;
        }

        // Inflate the boundary, and run the tesselator again to generate
        // interior polys.
        const GrPointArray& contourPoints = btess.contourPoints();
        const GrIndexArray& contours = btess.contours();
        GrEdgePolygonTess ptess(contourPoints.count(), GLU_TESS_WINDING_NONZERO, matrix);

        size_t i = 0;
        Sk_gluTessBeginPolygon(ptess.tess(), &ptess);
        for (int contour = 0; contour < contours.count(); ++contour) {
            int count = contours[contour];
            GrEdgeArray edges;
            int newCount = computeEdgesAndIntersect(matrix, inverse, &btess.contourPoints()[i], count, &edges, 1.0f);
            Sk_gluTessBeginContour(ptess.tess());
            for (int j = 0; j < newCount; j++) {
                ptess.addVertex(contourPoints[i + j], ptess.vertices().count());
            }
            i += count;
            Sk_gluTessEndContour(ptess.tess());
        }

        Sk_gluTessEndPolygon(ptess.tess());

        if (ptess.vertices().count() > USHRT_MAX) {
            return;
        }

        // Draw the resulting polys and upload their edge data.
        fTarget->enableState(GrDrawTarget::kEdgeAAConcave_StateBit);
        const GrPointArray& vertices = ptess.vertices();
        const GrIndexArray& indices = ptess.indices();
        const GrDrawState::Edge* edges = ptess.edges();
        GR_DEBUGASSERT(indices.count() % 3 == 0);
        for (int i = 0; i < indices.count(); i += 3) {
            GrPoint tri_verts[3];
            int index0 = indices[i];
            int index1 = indices[i + 1];
            int index2 = indices[i + 2];
            tri_verts[0] = vertices[index0];
            tri_verts[1] = vertices[index1];
            tri_verts[2] = vertices[index2];
            GrDrawState::Edge tri_edges[6];
            int t = 0;
            const GrDrawState::Edge& edge0 = edges[index0 * 2];
            const GrDrawState::Edge& edge1 = edges[index0 * 2 + 1];
            const GrDrawState::Edge& edge2 = edges[index1 * 2];
            const GrDrawState::Edge& edge3 = edges[index1 * 2 + 1];
            const GrDrawState::Edge& edge4 = edges[index2 * 2];
            const GrDrawState::Edge& edge5 = edges[index2 * 2 + 1];
            if (validEdge(edge0) && validEdge(edge1)) {
                tri_edges[t++] = edge0;
                tri_edges[t++] = edge1;
            }
            if (validEdge(edge2) && validEdge(edge3)) {
                tri_edges[t++] = edge2;
                tri_edges[t++] = edge3;
            }
            if (validEdge(edge4) && validEdge(edge5)) {
                tri_edges[t++] = edge4;
                tri_edges[t++] = edge5;
            }
            fTarget->setEdgeAAData(&tri_edges[0], t);
            fTarget->setVertexSourceToArray(layout, &tri_verts[0], 3);
            fTarget->drawNonIndexed(kTriangles_PrimitiveType, 0, 3);
        }
        fTarget->setEdgeAAData(NULL, 0);
        fTarget->disableState(GrDrawTarget::kEdgeAAConcave_StateBit);
        return;
    }

    GrPolygonTess ptess(count, fill_type_to_glu_winding_rule(fFill));
    ptess.addVertices(base, subpathVertCount, subpathCnt);
    const GrPointArray& vertices = ptess.vertices();
    const GrIndexArray& indices = ptess.indices();
    if (indices.count() > 0) {
        fTarget->setVertexSourceToArray(layout, vertices.begin(), vertices.count());
        fTarget->setIndexSourceToArray(indices.begin(), indices.count());
        fTarget->drawIndexed(kTriangles_PrimitiveType,
                            0,
                            0,
                            vertices.count(),
                            indices.count());
    }
}

bool GrTesselatedPathRenderer::canDrawPath(const GrDrawTarget::Caps& caps,
                                           const SkPath& path,
                                           GrPathFill fill,
                                           bool antiAlias) const {
    return kHairLine_PathFill != fill;
}

void GrTesselatedPathRenderer::drawPathToStencil() {
    GrAlwaysAssert(!"multipass stencil should not be needed");
}

