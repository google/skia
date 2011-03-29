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

#include <internal_glu.h>

struct PolygonData {
    PolygonData(GrTDArray<GrPoint>* vertices, GrTDArray<short>* indices)
      : fVertices(vertices)
      , fIndices(indices)
    {
    }
    GrTDArray<GrPoint>* fVertices;
    GrTDArray<short>* fIndices;
};

static void beginData(GLenum type, void* data)
{
    GR_DEBUGASSERT(type == GL_TRIANGLES);
}

static void edgeFlagData(GLboolean flag, void* data)
{
}

static void vertexData(void* vertexData, void* data)
{
    short* end = static_cast<PolygonData*>(data)->fIndices->append();
    *end = reinterpret_cast<long>(vertexData);
}

static void endData(void* data)
{
}

static void combineData(GLdouble coords[3], void* vertexData[4],
                                 GLfloat weight[4], void **outData, void* data)
{
    PolygonData* polygonData = static_cast<PolygonData*>(data);
    int index = polygonData->fVertices->count();
    *polygonData->fVertices->append() = GrPoint(static_cast<float>(coords[0]),
                                                 static_cast<float>(coords[1]));
    *outData = reinterpret_cast<void*>(index);
}

typedef void (*TESSCB)();

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

void GrTesselatedPathRenderer::drawPath(GrDrawTarget* target,
                                        GrDrawTarget::StageBitfield stages,
                                        GrPathIter* path,
                                        GrPathFill fill,
                                        const GrPoint* translate) {
    GrDrawTarget::AutoStateRestore asr(target);
    bool colorWritesWereDisabled = target->isColorWriteDisabled();
    // face culling doesn't make sense here
    GrAssert(GrDrawTarget::kBoth_DrawFace == target->getDrawFace());

    GrMatrix viewM = target->getViewMatrix();
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    bool useStretch = stretch > 0;
    GrScalar tol = GrPathUtils::gTolerance;

    if (!useStretch) {
        // TODO: deal with perspective in some better way.
        tol /= 10;
    } else {
        GrScalar sinv = GR_Scalar1 / stretch;
        tol = GrMul(tol, sinv);
    }
    GrScalar tolSqd = GrMul(tol, tol);

    path->rewind();

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
    GrPoint* base = new GrPoint[maxPts];
    GrPoint* vert = base;
    GrPoint* subpathBase = base;

    GrAutoSTMalloc<8, uint16_t> subpathVertCount(subpathCnt);

    path->rewind();

    GrPoint pts[4];

    bool first = true;
    int subpath = 0;

    for (;;) {
        GrPathCmd cmd = path->next(pts);
        switch (cmd) {
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
        *vert++ = GrPoint(bounds.fLeft, bounds.fTop);
        *vert++ = GrPoint(bounds.fLeft, bounds.fBottom);
        *vert++ = GrPoint(bounds.fRight, bounds.fBottom);
        *vert++ = GrPoint(bounds.fRight, bounds.fTop);
        subpathVertCount[subpath++] = 4;
    }

    GrAssert(subpath == subpathCnt);
    GrAssert((vert - base) <= maxPts);

    size_t count = vert - base;

    if (subpathCnt == 1 && !inverted && path->convexHint() == kConvex_ConvexHint) {
        target->setVertexSourceToArray(layout, base, count);
        target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, count);
        delete[] base;
        return;
    }

    // FIXME:  This copy could be removed if we had (templated?) versions of
    // generate_*_point above that wrote directly into doubles.
    double* inVertices = new double[count * 3];
    for (size_t i = 0; i < count; ++i) {
        inVertices[i * 3]     = base[i].fX;
        inVertices[i * 3 + 1] = base[i].fY;
        inVertices[i * 3 + 2] = 1.0;
    }

    GLUtesselator* tess = internal_gluNewTess();
    unsigned windingRule = fill_type_to_glu_winding_rule(fill);
    internal_gluTessProperty(tess, GLU_TESS_WINDING_RULE, windingRule);
    internal_gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (TESSCB) &beginData);
    internal_gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (TESSCB) &vertexData);
    internal_gluTessCallback(tess, GLU_TESS_END_DATA, (TESSCB) &endData);
    internal_gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (TESSCB) &edgeFlagData);
    internal_gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (TESSCB) &combineData);
    GrTDArray<short> indices;
    GrTDArray<GrPoint> vertices;
    PolygonData data(&vertices, &indices);

    internal_gluTessBeginPolygon(tess, &data);
    size_t i = 0;
    for (int sp = 0; sp < subpathCnt; ++sp) {
        internal_gluTessBeginContour(tess);
        int start = i;
        int end = start + subpathVertCount[sp];
        for (; i < end; ++i) {
            double* inVertex = &inVertices[i * 3];
            *vertices.append() = GrPoint(inVertex[0], inVertex[1]);
            internal_gluTessVertex(tess, inVertex, reinterpret_cast<void*>(i));
        }
        internal_gluTessEndContour(tess);
    }

    internal_gluTessEndPolygon(tess);
    internal_gluDeleteTess(tess);

    if (indices.count() > 0) {
        target->setVertexSourceToArray(layout, vertices.begin(), vertices.count());
        target->setIndexSourceToArray(indices.begin(), indices.count());
        target->drawIndexed(kTriangles_PrimitiveType,
                            0,
                            0,
                            vertices.count(),
                            indices.count());
    }
    delete[] inVertices;
    delete[] base;
}

bool GrTesselatedPathRenderer::canDrawPath(const GrDrawTarget* target,
                                           GrPathIter* path,
                                           GrPathFill fill) const {
    return kHairLine_PathFill != fill;
}

void GrTesselatedPathRenderer::drawPathToStencil(GrDrawTarget* target,
                                                 GrPathIter* path,
                                                 GrPathFill fill,
                                                 const GrPoint* translate) {
    GrAlwaysAssert(!"multipass stencil should not be needed");
}
