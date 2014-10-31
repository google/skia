
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAndroidPathRenderer.h"
#include "AndroidPathRenderer.h"
#include "Vertex.h"

GrAndroidPathRenderer::GrAndroidPathRenderer() {
}

bool GrAndroidPathRenderer::canDrawPath(const SkPath& path,
                                        const SkStrokeRec& stroke,
                                        const GrDrawTarget* target,
                                        bool antiAlias) const {
    return ((stroke.isFillStyle() || stroke.getStyle() == SkStrokeRec::kStroke_Style)
             && !path.isInverseFillType() && path.isConvex());
}

struct ColorVertex {
    SkPoint pos;
    GrColor color;
};

bool GrAndroidPathRenderer::onDrawPath(const SkPath& origPath,
                                       const SkStrokeRec& stroke,
                                       GrDrawTarget* target,
                                       bool antiAlias) {

    // generate verts using Android algorithm
    android::uirenderer::VertexBuffer vertices;
    android::uirenderer::PathRenderer::ConvexPathVertices(origPath, stroke, antiAlias, NULL,
                                                          &vertices);

    // set vertex attributes depending on anti-alias
    GrDrawState* drawState = target->drawState();
    if (antiAlias) {
        // position + coverage
        GrVertexAttrib attribs[] = {
            GrVertexAttrib(kVec2f_GrVertexAttribType, 0),
            GrVertexAttrib(kVec4ub_GrVertexAttribType, sizeof(GrPoint))
        };
        drawState->setVertexAttribs(attribs, SK_ARRAY_COUNT(attribs));
        drawState->setAttribIndex(GrDrawState::kPosition_AttribIndex, 0);
        drawState->setAttribIndex(GrDrawState::kCoverage_AttribIndex, 1);
        drawState->setAttribBindings(GrDrawState::kCoverage_AttribBindingsBit);
    } else {
        drawState->setDefaultVertexAttribs();
    }

    // allocate our vert buffer
    int vertCount = vertices.getSize();
    GrDrawTarget::AutoReleaseGeometry geo(target, vertCount, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    // copy android verts to our vertex buffer
    if (antiAlias) {
        SkASSERT(sizeof(ColorVertex) == drawState->getVertexSize());
        ColorVertex* outVert = reinterpret_cast<ColorVertex*>(geo.vertices());
        android::uirenderer::AlphaVertex* inVert =
            reinterpret_cast<android::uirenderer::AlphaVertex*>(vertices.getBuffer());

        for (int i = 0; i < vertCount; ++i) {
            // copy vertex position
            outVert->pos.set(inVert->position[0], inVert->position[1]);
            // copy alpha
            int coverage = static_cast<int>(inVert->alpha * 0xff);
            outVert->color = GrColorPackRGBA(coverage, coverage, coverage, coverage);
            ++outVert;
            ++inVert;
        }
    } else {
       size_t vsize = drawState->getVertexSize();
       size_t copySize = vsize*vertCount;
       memcpy(geo.vertices(), vertices.getBuffer(), copySize);
    }

    // render it
    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, vertCount);

    return true;
}
