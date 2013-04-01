/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrRefCnt.h"
#include "GrGpu.h"

SK_DEFINE_INST_COUNT(GrAARectRenderer)

namespace {

static void aa_rect_attributes(bool useCoverage, const GrVertexAttrib** attribs, int* count) {
    static const GrVertexAttrib kCoverageAttribs[] = {
        {kVec2f_GrVertexAttribType,  0, kPosition_GrVertexAttribBinding},
        {kVec4ub_GrVertexAttribType, sizeof(GrPoint), kCoverage_GrVertexAttribBinding},
    };
    static const GrVertexAttrib kColorAttribs[] = {
        {kVec2f_GrVertexAttribType,  0, kPosition_GrVertexAttribBinding},
        {kVec4ub_GrVertexAttribType, sizeof(GrPoint), kColor_GrVertexAttribBinding},
    };
    *attribs = useCoverage ? kCoverageAttribs : kColorAttribs;
    *count = 2;
}

static void set_inset_fan(GrPoint* pts, size_t stride,
                          const GrRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

};

void GrAARectRenderer::reset() {
    GrSafeSetNull(fAAFillRectIndexBuffer);
    GrSafeSetNull(fAAStrokeRectIndexBuffer);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

static const int kIndicesPerAAFillRect = GR_ARRAY_COUNT(gFillAARectIdx);
static const int kVertsPerAAFillRect = 8;
static const int kNumAAFillRectsInIndexBuffer = 256;

GrIndexBuffer* GrAARectRenderer::aaFillRectIndexBuffer(GrGpu* gpu) {
    static const size_t kAAFillRectIndexBufferSize = kIndicesPerAAFillRect *
                                                     sizeof(uint16_t) *
                                                     kNumAAFillRectsInIndexBuffer;

    if (NULL == fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = gpu->createIndexBuffer(kAAFillRectIndexBufferSize, false);
        if (NULL != fAAFillRectIndexBuffer) {
            uint16_t* data = (uint16_t*) fAAFillRectIndexBuffer->lock();
            bool useTempData = (NULL == data);
            if (useTempData) {
                data = SkNEW_ARRAY(uint16_t, kNumAAFillRectsInIndexBuffer * kIndicesPerAAFillRect);
            }
            for (int i = 0; i < kNumAAFillRectsInIndexBuffer; ++i) {
                // Each AA filled rect is drawn with 8 vertices and 10 triangles (8 around
                // the inner rect (for AA) and 2 for the inner rect.
                int baseIdx = i * kIndicesPerAAFillRect;
                uint16_t baseVert = (uint16_t)(i * kVertsPerAAFillRect);
                for (int j = 0; j < kIndicesPerAAFillRect; ++j) {
                    data[baseIdx+j] = baseVert + gFillAARectIdx[j];
                }
            }
            if (useTempData) {
                if (!fAAFillRectIndexBuffer->updateData(data, kAAFillRectIndexBufferSize)) {
                    GrCrash("Can't get AA Fill Rect indices into buffer!");
                }
                SkDELETE_ARRAY(data);
            } else {
                fAAFillRectIndexBuffer->unlock();
            }
        }
    }

    return fAAFillRectIndexBuffer;
}

static const uint16_t gStrokeAARectIdx[] = {
    0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
    1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
    2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
    3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

    0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
    1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
    2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
    3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

    0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
    1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
    2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
    3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
};

int GrAARectRenderer::aaStrokeRectIndexCount() {
    return GR_ARRAY_COUNT(gStrokeAARectIdx);
}

GrIndexBuffer* GrAARectRenderer::aaStrokeRectIndexBuffer(GrGpu* gpu) {
    if (NULL == fAAStrokeRectIndexBuffer) {
        fAAStrokeRectIndexBuffer =
                  gpu->createIndexBuffer(sizeof(gStrokeAARectIdx), false);
        if (NULL != fAAStrokeRectIndexBuffer) {
#if GR_DEBUG
            bool updated =
#endif
            fAAStrokeRectIndexBuffer->updateData(gStrokeAARectIdx,
                                                 sizeof(gStrokeAARectIdx));
            GR_DEBUGASSERT(updated);
        }
    }
    return fAAStrokeRectIndexBuffer;
}

void GrAARectRenderer::fillAARect(GrGpu* gpu,
                                  GrDrawTarget* target,
                                  const GrRect& devRect,
                                  bool useVertexCoverage) {
    GrDrawState* drawState = target->drawState();

    const GrVertexAttrib* attribs;
    int attribCount;
    aa_rect_attributes(useVertexCoverage, &attribs, &attribCount);
    drawState->setVertexAttribs(attribs, attribCount);

    GrDrawTarget::AutoReleaseGeometry geo(target, 8, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    GrIndexBuffer* indexBuffer = this->aaFillRectIndexBuffer(gpu);
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());
    size_t vsize = drawState->getVertexSize();
    GrAssert(sizeof(GrPoint) + sizeof(GrColor) == vsize);

    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);

    set_inset_fan(fan0Pos, vsize, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
    set_inset_fan(fan1Pos, vsize, devRect,  SK_ScalarHalf,  SK_ScalarHalf);

    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = 0xffffffff;
    } else {
        innerColor = target->getDrawState().getColor();
    }

    verts += 4 * vsize;
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1,
                                 kVertsPerAAFillRect,
                                 kIndicesPerAAFillRect);
}

void GrAARectRenderer::strokeAARect(GrGpu* gpu,
                                    GrDrawTarget* target,
                                    const GrRect& devRect,
                                    const GrVec& devStrokeSize,
                                    bool useVertexCoverage) {
    GrDrawState* drawState = target->drawState();

    const SkScalar& dx = devStrokeSize.fX;
    const SkScalar& dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = GrMin(w, h);
    }

    if (spare <= 0) {
        GrRect r(devRect);
        r.inset(-rx, -ry);
        this->fillAARect(gpu, target, r, useVertexCoverage);
        return;
    }

    const GrVertexAttrib* attribs;
    int attribCount;
    aa_rect_attributes(useVertexCoverage, &attribs, &attribCount);
    drawState->setVertexAttribs(attribs, attribCount);

    GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer(gpu);
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());
    size_t vsize = drawState->getVertexSize();
    GrAssert(sizeof(GrPoint) + sizeof(GrColor) == vsize);

    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.
    // The following pointers refer to the four rects, from outermost to innermost.
    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);
    GrPoint* fan2Pos = reinterpret_cast<GrPoint*>(verts + 8 * vsize);
    GrPoint* fan3Pos = reinterpret_cast<GrPoint*>(verts + 12 * vsize);

    set_inset_fan(fan0Pos, vsize, devRect,
                  -rx - SK_ScalarHalf, -ry - SK_ScalarHalf);
    set_inset_fan(fan1Pos, vsize, devRect,
                  -rx + SK_ScalarHalf, -ry + SK_ScalarHalf);
    set_inset_fan(fan2Pos, vsize, devRect,
                  rx - SK_ScalarHalf,  ry - SK_ScalarHalf);
    set_inset_fan(fan3Pos, vsize, devRect,
                  rx + SK_ScalarHalf,  ry + SK_ScalarHalf);

    // The outermost rect has 0 coverage
    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    // The inner two rects have full coverage
    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = 0xffffffff;
    } else {
        innerColor = target->getDrawState().getColor();
    }
    verts += 4 * vsize;
    for (int i = 0; i < 8; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
    }

    // The innermost rect has full coverage
    verts += 8 * vsize;
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexed(kTriangles_GrPrimitiveType,
                        0, 0, 16, aaStrokeRectIndexCount());
}
