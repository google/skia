/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrBatchTarget.h"
#include "GrBatchTest.h"
#include "GrContext.h"
#include "GrDrawTarget.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrTestUtils.h"
#include "GrVertexBuffer.h"
#include "SkColorPriv.h"
#include "batches/GrAAFillRectBatch.h"
#include "batches/GrAAStrokeRectBatch.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

///////////////////////////////////////////////////////////////////////////////

void GrAARectRenderer::GeometryFillAARect(GrDrawTarget* target,
                                          const GrPipelineBuilder& pipelineBuilder,
                                          GrColor color,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkRect& devRect) {
    GrAAFillRectBatch::Geometry geometry;
    geometry.fRect = rect;
    geometry.fViewMatrix = viewMatrix;
    geometry.fDevRect = devRect;
    geometry.fColor = color;


    SkAutoTUnref<GrBatch> batch(GrAAFillRectBatch::Create(geometry));
    target->drawBatch(pipelineBuilder, batch);
}

void GrAARectRenderer::StrokeAARect(GrDrawTarget* target,
                                    const GrPipelineBuilder& pipelineBuilder,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    const SkStrokeRec& stroke) {
    SkVector devStrokeSize;
    SkScalar width = stroke.getWidth();
    if (width > 0) {
        devStrokeSize.set(width, width);
        viewMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    bool miterStroke = true;
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((width > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                        stroke.getMiter() < SK_ScalarSqrt2)) {
        miterStroke = false;
    }

    if (spare <= 0 && miterStroke) {
        FillAARect(target, pipelineBuilder, color, viewMatrix, devOutside, devOutside);
        return;
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    SkRect devOutsideAssist(devRect);

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outer of the rect. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside.inset(0, ry);
        devOutsideAssist.outset(0, ry);
    }

    GeometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                         devOutsideAssist, devInside, miterStroke);
}

void GrAARectRenderer::GeometryStrokeAARect(GrDrawTarget* target,
                                            const GrPipelineBuilder& pipelineBuilder,
                                            GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& devOutside,
                                            const SkRect& devOutsideAssist,
                                            const SkRect& devInside,
                                            bool miterStroke) {
    GrAAStrokeRectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fDevOutside = devOutside;
    geometry.fDevOutsideAssist = devOutsideAssist;
    geometry.fDevInside = devInside;
    geometry.fMiterStroke = miterStroke;

    SkAutoTUnref<GrBatch> batch(GrAAStrokeRectBatch::Create(geometry, viewMatrix));
    target->drawBatch(pipelineBuilder, batch);
}

void GrAARectRenderer::FillAANestedRects(GrDrawTarget* target,
                                         const GrPipelineBuilder& pipelineBuilder,
                                         GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    viewMatrix.mapRect(&devInside, rects[1]);

    if (devInside.isEmpty()) {
        FillAARect(target, pipelineBuilder, color, viewMatrix, devOutside, devOutside);
        return;
    }

    GeometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                         devOutside, devInside, true);
}
