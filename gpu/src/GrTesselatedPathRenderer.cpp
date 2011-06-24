/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "GrTesselatedPathRenderer.h"

#include "GrMemory.h"
#include "GrPathUtils.h"
#include "GrPoint.h"
#include "GrTDArray.h"

#include <limits.h>
#include <sk_glu.h>

typedef GrTDArray<GrDrawTarget::Edge> GrEdgeArray;
typedef GrTDArray<GrPoint> GrPointArray;
typedef GrTDArray<uint16_t> GrIndexArray;
typedef void (*TESSCB)();

// limit the allowable vertex range to approximately half of the representable
// IEEE exponent in order to avoid overflow when doing multiplies between
// vertex components,
const float kMaxVertexValue = 1e18;

static inline GrDrawTarget::Edge computeEdge(const GrPoint& p,
                                             const GrPoint& q,
                                             float sign) {
    GrVec tangent = GrVec::Make(p.fY - q.fY, q.fX - p.fX);
    float scale = sign / tangent.length();
    float cross2 = p.fX * q.fY - q.fX * p.fY;
    return GrDrawTarget::Edge(tangent.fX * scale,
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
    const GrDrawTarget::Edge* edges() const { return fEdges; }
private:
    void addEdge(int index0, int index1) {
        GrPoint p = fVertices[index0];
        GrPoint q = fVertices[index1];
        fMatrix.mapPoints(&p, 1);
        fMatrix.mapPoints(&q, 1);
        p = sanitizePoint(p);
        q = sanitizePoint(q);
        if (p == q) return;
        GrDrawTarget::Edge edge = computeEdge(p, q, 1.0f);
        fEdges[index0 * 2 + 1] = edge;
        fEdges[index1 * 2] = edge;
    }
    virtual void begin(GLenum type) {
        GR_DEBUGASSERT(type == GL_TRIANGLES);
        int count = fVertices.count() * 2;
        fEdges = new GrDrawTarget::Edge[count];
        memset(fEdges, 0, count * sizeof(GrDrawTarget::Edge));
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
    GrDrawTarget::Edge* fEdges;
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

static bool parallel(const GrDrawTarget::Edge& a, const GrDrawTarget::Edge& b) {
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

static bool validEdge(const GrDrawTarget::Edge& edge) {
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
        GrDrawTarget::Edge edge = computeEdge(p, q, sign);
        edge.fZ += 0.5f;    // Offset by half a pixel along the tangent.
        *edges->append() = edge;
        p = q;
    }
    int count = edges->count();
    if (count == 0) {
        return 0;
    }
    GrDrawTarget::Edge prev_edge = edges->at(0);
    for (int i = 0; i < count; ++i) {
        GrDrawTarget::Edge edge = edges->at(i < count - 1 ? i + 1 : 0);
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

void GrTesselatedPathRenderer::drawPath(GrDrawTarget* target,
                                        GrDrawTarget::StageBitfield stages,
                                        const GrPath& path,
                                        GrPathFill fill,
                                        const GrPoint* translate) {
    GrDrawTarget::AutoStateRestore asr(target);
    // face culling doesn't make sense here
    GrAssert(GrDrawTarget::kBoth_DrawFace == target->getDrawFace());

    GrMatrix viewM = target->getViewMatrix();
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    bool useStretch = stretch > 0;
    GrScalar tol = fCurveTolerance;

    if (!useStretch) {
        // TODO: deal with perspective in some better way.
        tol /= 10;
    } else {
        tol = GrScalarDiv(tol, stretch);
    }
    GrScalar tolSqd = GrMul(tol, tol);

    int subpathCnt;
    int maxPts = GrPathUtils::worstCasePointCount(path, &subpathCnt, tol);

    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
        if ((1 << s) & stages) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }

    bool inverted = IsFillInverted(fill);
    if (inverted) {
        maxPts += 4;
        subpathCnt++;
    }
    if (maxPts > USHRT_MAX) {
        return;
    }
    GrAutoSTMalloc<8, GrPoint> baseMem(maxPts);
    GrPoint* base = (GrPoint*) baseMem;
    GrPoint* vert = base;
    GrPoint* subpathBase = base;

    GrAutoSTMalloc<8, uint16_t> subpathVertCount(subpathCnt);

    GrPoint pts[4];
    SkPath::Iter iter(path, false);

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
    if (translate) {
        for (int i = 0; i < vert - base; i++) {
            base[i].offset(translate->fX, translate->fY);
        }
    }

    if (inverted) {
        GrRect bounds;
        GrAssert(NULL != target->getRenderTarget());
        bounds.setLTRB(0, 0,
                       GrIntToScalar(target->getRenderTarget()->width()),
                       GrIntToScalar(target->getRenderTarget()->height()));
        GrMatrix vmi;
        if (target->getViewInverse(&vmi)) {
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

    if (subpathCnt == 1 && !inverted && path.isConvex()) {
        if (target->isAntialiasState()) {
            GrEdgeArray edges;
            GrMatrix inverse, matrix = target->getViewMatrix();
            target->getViewInverse(&inverse);

            count = computeEdgesAndIntersect(matrix, inverse, base, count, &edges, 0.0f);
            size_t maxEdges = target->getMaxEdges();
            if (count == 0) {
                return;
            }
            if (count <= maxEdges) {
                // All edges fit; upload all edges and draw all verts as a fan
                target->setVertexSourceToArray(layout, base, count);
                target->setEdgeAAData(&edges[0], count);
                target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, count);
            } else {
                // Upload "maxEdges" edges and verts at a time, and draw as
                // separate fans
                for (size_t i = 0; i < count - 2; i += maxEdges - 2) {
                    edges[i] = edges[0];
                    base[i] = base[0];
                    int size = GR_CT_MIN(count - i, maxEdges);
                    target->setVertexSourceToArray(layout, &base[i], size);
                    target->setEdgeAAData(&edges[i], size);
                    target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, size);
                }
            }
            target->setEdgeAAData(NULL, 0);
        } else {
            target->setVertexSourceToArray(layout, base, count);
            target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, count);
        }
        return;
    }

    if (target->isAntialiasState()) {
        // Run the tesselator once to get the boundaries.
        GrBoundaryTess btess(count, fill_type_to_glu_winding_rule(fill));
        btess.addVertices(base, subpathVertCount, subpathCnt);

        GrMatrix inverse, matrix = target->getViewMatrix();
        if (!target->getViewInverse(&inverse)) {
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
        target->enableState(GrDrawTarget::kEdgeAAConcave_StateBit);
        const GrPointArray& vertices = ptess.vertices();
        const GrIndexArray& indices = ptess.indices();
        const GrDrawTarget::Edge* edges = ptess.edges();
        GR_DEBUGASSERT(indices.count() % 3 == 0);
        for (int i = 0; i < indices.count(); i += 3) {
            GrPoint tri_verts[3];
            int index0 = indices[i];
            int index1 = indices[i + 1];
            int index2 = indices[i + 2];
            tri_verts[0] = vertices[index0];
            tri_verts[1] = vertices[index1];
            tri_verts[2] = vertices[index2];
            GrDrawTarget::Edge tri_edges[6];
            int t = 0;
            const GrDrawTarget::Edge& edge0 = edges[index0 * 2];
            const GrDrawTarget::Edge& edge1 = edges[index0 * 2 + 1];
            const GrDrawTarget::Edge& edge2 = edges[index1 * 2];
            const GrDrawTarget::Edge& edge3 = edges[index1 * 2 + 1];
            const GrDrawTarget::Edge& edge4 = edges[index2 * 2];
            const GrDrawTarget::Edge& edge5 = edges[index2 * 2 + 1];
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
            target->setEdgeAAData(&tri_edges[0], t);
            target->setVertexSourceToArray(layout, &tri_verts[0], 3);
            target->drawNonIndexed(kTriangles_PrimitiveType, 0, 3);
        }
        target->setEdgeAAData(NULL, 0);
        target->disableState(GrDrawTarget::kEdgeAAConcave_StateBit);
        return;
    }

    GrPolygonTess ptess(count, fill_type_to_glu_winding_rule(fill));
    ptess.addVertices(base, subpathVertCount, subpathCnt);
    const GrPointArray& vertices = ptess.vertices();
    const GrIndexArray& indices = ptess.indices();
    if (indices.count() > 0) {
        target->setVertexSourceToArray(layout, vertices.begin(), vertices.count());
        target->setIndexSourceToArray(indices.begin(), indices.count());
        target->drawIndexed(kTriangles_PrimitiveType,
                            0,
                            0,
                            vertices.count(),
                            indices.count());
    }
}

bool GrTesselatedPathRenderer::canDrawPath(const GrDrawTarget* target,
                                           const SkPath& path,
                                           GrPathFill fill) const {
    return kHairLine_PathFill != fill;
}

void GrTesselatedPathRenderer::drawPathToStencil(GrDrawTarget* target,
                                                 const SkPath& path,
                                                 GrPathFill fill,
                                                 const GrPoint* translate) {
    GrAlwaysAssert(!"multipass stencil should not be needed");
}

bool GrTesselatedPathRenderer::supportsAA(GrDrawTarget* target,
                                                  const SkPath& path,
                                                  GrPathFill fill) {
    return true;
}
