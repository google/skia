/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define LOG_TAG "PathRenderer"
#define LOG_NDEBUG 1
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#define VERTEX_DEBUG 0

#include <SkPath.h>
#include <SkStrokeRec.h>

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include <SkTypes.h>
#include <SkTraceEvent.h>
#include <SkMatrix.h>
#include <SkPoint.h>

#ifdef VERBOSE
#define ALOGV SkDebugf
#else
#define ALOGV(x, ...)
#endif

#include "AndroidPathRenderer.h"
#include "Vertex.h"

namespace android {
namespace uirenderer {

#define THRESHOLD 0.5f

SkRect PathRenderer::ComputePathBounds(const SkPath& path, const SkPaint* paint) {
    SkRect bounds = path.getBounds();
    if (paint->getStyle() != SkPaint::kFill_Style) {
        float outset = paint->getStrokeWidth() * 0.5f;
        bounds.outset(outset, outset);
    }
    return bounds;
}

inline void computeInverseScales(const SkMatrix* transform, float &inverseScaleX, float& inverseScaleY) {
    if (transform && transform->getType() & (SkMatrix::kScale_Mask|SkMatrix::kAffine_Mask|SkMatrix::kPerspective_Mask)) {
        float m00 = transform->getScaleX();
        float m01 = transform->getSkewY();
        float m10 = transform->getSkewX();
        float m11 = transform->getScaleY();
        float scaleX = sk_float_sqrt(m00 * m00 + m01 * m01);
        float scaleY = sk_float_sqrt(m10 * m10 + m11 * m11);
        inverseScaleX = (scaleX != 0) ? (1.0f / scaleX) : 1.0f;
        inverseScaleY = (scaleY != 0) ? (1.0f / scaleY) : 1.0f;
    } else {
        inverseScaleX = 1.0f;
        inverseScaleY = 1.0f;
    }
}

inline void copyVertex(Vertex* destPtr, const Vertex* srcPtr) {
    Vertex::set(destPtr, srcPtr->position[0], srcPtr->position[1]);
}

inline void copyAlphaVertex(AlphaVertex* destPtr, const AlphaVertex* srcPtr) {
    AlphaVertex::set(destPtr, srcPtr->position[0], srcPtr->position[1], srcPtr->alpha);
}

/**
 * Produces a pseudo-normal for a vertex, given the normals of the two incoming lines. If the offset
 * from each vertex in a perimeter is calculated, the resultant lines connecting the offset vertices
 * will be offset by 1.0
 *
 * Note that we can't add and normalize the two vectors, that would result in a rectangle having an
 * offset of (sqrt(2)/2, sqrt(2)/2) at each corner, instead of (1, 1)
 *
 * NOTE: assumes angles between normals 90 degrees or less
 */
inline SkVector totalOffsetFromNormals(const SkVector& normalA, const SkVector& normalB) {
    SkVector pseudoNormal = normalA + normalB;
    pseudoNormal.scale(1.0f / (1.0f + sk_float_abs(normalA.dot(normalB))));
    return pseudoNormal;
}

inline void scaleOffsetForStrokeWidth(SkVector& offset, float halfStrokeWidth,
        float inverseScaleX, float inverseScaleY) {
    if (halfStrokeWidth == 0.0f) {
        // hairline - compensate for scale
        offset.fX *= 0.5f * inverseScaleX;
        offset.fY *= 0.5f * inverseScaleY;
    } else {
        offset.scale(halfStrokeWidth);
    }
}

static void getFillVerticesFromPerimeter(const SkTArray<Vertex, true>& perimeter, VertexBuffer* vertexBuffer) {
    Vertex* buffer = vertexBuffer->alloc<Vertex>(perimeter.count());

    int currentIndex = 0;
    // zig zag between all previous points on the inside of the hull to create a
    // triangle strip that fills the hull
    int srcAindex = 0;
    int srcBindex = perimeter.count() - 1;
    while (srcAindex <= srcBindex) {
        copyVertex(&buffer[currentIndex++], &perimeter[srcAindex]);
        if (srcAindex == srcBindex) break;
        copyVertex(&buffer[currentIndex++], &perimeter[srcBindex]);
        srcAindex++;
        srcBindex--;
    }
}

static void getStrokeVerticesFromPerimeter(const SkTArray<Vertex, true>& perimeter, float halfStrokeWidth,
        VertexBuffer* vertexBuffer, float inverseScaleX, float inverseScaleY) {
    Vertex* buffer = vertexBuffer->alloc<Vertex>(perimeter.count() * 2 + 2);

    int currentIndex = 0;
    const Vertex* last = &(perimeter[perimeter.count() - 1]);
    const Vertex* current = &(perimeter[0]);
    SkVector lastNormal;
    lastNormal.set(current->position[1] - last->position[1],
                   last->position[0] - current->position[0]);
    lastNormal.normalize();
    for (int i = 0; i < perimeter.count(); i++) {
        const Vertex* next = &(perimeter[i + 1 >= perimeter.count() ? 0 : i + 1]);
        SkVector nextNormal;
        nextNormal.set(next->position[1] - current->position[1],
                       current->position[0] - next->position[0]);
        nextNormal.normalize();

        SkVector totalOffset = totalOffsetFromNormals(lastNormal, nextNormal);
        scaleOffsetForStrokeWidth(totalOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);

        Vertex::set(&buffer[currentIndex++],
                current->position[0] + totalOffset.fX,
                current->position[1] + totalOffset.fY);

        Vertex::set(&buffer[currentIndex++],
                current->position[0] - totalOffset.fX,
                current->position[1] - totalOffset.fY);

        last = current;
        current = next;
        lastNormal = nextNormal;
    }

    // wrap around to beginning
    copyVertex(&buffer[currentIndex++], &buffer[0]);
    copyVertex(&buffer[currentIndex++], &buffer[1]);
}

static void getStrokeVerticesFromUnclosedVertices(const SkTArray<Vertex, true>& vertices, float halfStrokeWidth,
        VertexBuffer* vertexBuffer, float inverseScaleX, float inverseScaleY) {
    Vertex* buffer = vertexBuffer->alloc<Vertex>(vertices.count() * 2);

    int currentIndex = 0;
    const Vertex* current = &(vertices[0]);
    SkVector lastNormal;
    for (int i = 0; i < vertices.count() - 1; i++) {
        const Vertex* next = &(vertices[i + 1]);
        SkVector nextNormal;
        nextNormal.set(next->position[1] - current->position[1],
                       current->position[0] - next->position[0]);
        nextNormal.normalize();

        SkVector totalOffset;
        if (i == 0) {
            totalOffset = nextNormal;
        } else {
            totalOffset = totalOffsetFromNormals(lastNormal, nextNormal);
        }
        scaleOffsetForStrokeWidth(totalOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);

        Vertex::set(&buffer[currentIndex++],
                current->position[0] + totalOffset.fX,
                current->position[1] + totalOffset.fY);

        Vertex::set(&buffer[currentIndex++],
                current->position[0] - totalOffset.fX,
                current->position[1] - totalOffset.fY);

        current = next;
        lastNormal = nextNormal;
    }

    SkVector totalOffset = lastNormal;
    scaleOffsetForStrokeWidth(totalOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);

    Vertex::set(&buffer[currentIndex++],
            current->position[0] + totalOffset.fX,
            current->position[1] + totalOffset.fY);
    Vertex::set(&buffer[currentIndex++],
            current->position[0] - totalOffset.fX,
            current->position[1] - totalOffset.fY);
#if VERTEX_DEBUG
    for (unsigned int i = 0; i < vertexBuffer.getSize(); i++) {
        SkDebugf("point at %f %f", buffer[i].position[0], buffer[i].position[1]);
    }
#endif
}

static void getFillVerticesFromPerimeterAA(const SkTArray<Vertex, true>& perimeter, VertexBuffer* vertexBuffer,
         float inverseScaleX, float inverseScaleY) {
    AlphaVertex* buffer = vertexBuffer->alloc<AlphaVertex>(perimeter.count() * 3 + 2);

    // generate alpha points - fill Alpha vertex gaps in between each point with
    // alpha 0 vertex, offset by a scaled normal.
    int currentIndex = 0;
    const Vertex* last = &(perimeter[perimeter.count() - 1]);
    const Vertex* current = &(perimeter[0]);
    SkVector lastNormal;
    lastNormal.set(current->position[1] - last->position[1],
                   last->position[0] - current->position[0]);
    lastNormal.normalize();
    for (int i = 0; i < perimeter.count(); i++) {
        const Vertex* next = &(perimeter[i + 1 >= perimeter.count() ? 0 : i + 1]);
        SkVector nextNormal;
        nextNormal.set(next->position[1] - current->position[1],
                       current->position[0] - next->position[0]);
        nextNormal.normalize();

        // AA point offset from original point is that point's normal, such that each side is offset
        // by .5 pixels
        SkVector totalOffset = totalOffsetFromNormals(lastNormal, nextNormal);
        totalOffset.fX *= 0.5f * inverseScaleX;
        totalOffset.fY *= 0.5f * inverseScaleY;

        AlphaVertex::set(&buffer[currentIndex++],
                current->position[0] + totalOffset.fX,
                current->position[1] + totalOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[currentIndex++],
                current->position[0] - totalOffset.fX,
                current->position[1] - totalOffset.fY,
                1.0f);

        last = current;
        current = next;
        lastNormal = nextNormal;
    }

    // wrap around to beginning
    copyAlphaVertex(&buffer[currentIndex++], &buffer[0]);
    copyAlphaVertex(&buffer[currentIndex++], &buffer[1]);

    // zig zag between all previous points on the inside of the hull to create a
    // triangle strip that fills the hull, repeating the first inner point to
    // create degenerate tris to start inside path
    int srcAindex = 0;
    int srcBindex = perimeter.count() - 1;
    while (srcAindex <= srcBindex) {
        copyAlphaVertex(&buffer[currentIndex++], &buffer[srcAindex * 2 + 1]);
        if (srcAindex == srcBindex) break;
        copyAlphaVertex(&buffer[currentIndex++], &buffer[srcBindex * 2 + 1]);
        srcAindex++;
        srcBindex--;
    }

#if VERTEX_DEBUG
    for (unsigned int i = 0; i < vertexBuffer.getSize(); i++) {
        SkDebugf("point at %f %f, alpha %f", buffer[i].position[0], buffer[i].position[1], buffer[i].alpha);
    }
#endif
}


static void getStrokeVerticesFromUnclosedVerticesAA(const SkTArray<Vertex, true>& vertices, float halfStrokeWidth,
        VertexBuffer* vertexBuffer, float inverseScaleX, float inverseScaleY) {
    AlphaVertex* buffer = vertexBuffer->alloc<AlphaVertex>(6 * vertices.count() + 2);

    // avoid lines smaller than hairline since they break triangle based sampling. instead reducing
    // alpha value (TODO: support different X/Y scale)
    float maxAlpha = 1.0f;
    if (halfStrokeWidth != 0 && inverseScaleX == inverseScaleY &&
            halfStrokeWidth * inverseScaleX < 0.5f) {
        maxAlpha *= (2 * halfStrokeWidth) / inverseScaleX;
        halfStrokeWidth = 0.0f;
    }

    // there is no outer/inner here, using them for consistency with below approach
    int offset = 2 * (vertices.count() - 2);
    int currentAAOuterIndex = 2;
    int currentAAInnerIndex = 2 * offset + 5; // reversed
    int currentStrokeIndex = currentAAInnerIndex + 7;

    const Vertex* last = &(vertices[0]);
    const Vertex* current = &(vertices[1]);
    SkVector lastNormal;
    lastNormal.set(current->position[1] - last->position[1],
                   last->position[0] - current->position[0]);
    lastNormal.normalize();

    {
        // start cap
        SkVector totalOffset = lastNormal;
        SkVector AAOffset = totalOffset;
        AAOffset.fX *= 0.5f * inverseScaleX;
        AAOffset.fY *= 0.5f * inverseScaleY;

        SkVector innerOffset = totalOffset;
        scaleOffsetForStrokeWidth(innerOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);
        SkVector outerOffset = innerOffset + AAOffset;
        innerOffset -= AAOffset;

        // TODO: support square cap by changing this offset to incorporate halfStrokeWidth
        SkVector capAAOffset;
        capAAOffset.set(AAOffset.fY, -AAOffset.fX);
        AlphaVertex::set(&buffer[0],
                last->position[0] + outerOffset.fX + capAAOffset.fX,
                last->position[1] + outerOffset.fY + capAAOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[1],
                last->position[0] + innerOffset.fX - capAAOffset.fX,
                last->position[1] + innerOffset.fY - capAAOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[2 * offset + 6],
                last->position[0] - outerOffset.fX + capAAOffset.fX,
                last->position[1] - outerOffset.fY + capAAOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[2 * offset + 7],
                last->position[0] - innerOffset.fX - capAAOffset.fX,
                last->position[1] - innerOffset.fY - capAAOffset.fY,
                maxAlpha);
        copyAlphaVertex(&buffer[2 * offset + 8], &buffer[0]);
        copyAlphaVertex(&buffer[2 * offset + 9], &buffer[1]);
        copyAlphaVertex(&buffer[2 * offset + 10], &buffer[1]); // degenerate tris (the only two!)
        copyAlphaVertex(&buffer[2 * offset + 11], &buffer[2 * offset + 7]);
    }

    for (int i = 1; i < vertices.count() - 1; i++) {
        const Vertex* next = &(vertices[i + 1]);
        SkVector nextNormal;
        nextNormal.set(next->position[1] - current->position[1],
                       current->position[0] - next->position[0]);
        nextNormal.normalize();

        SkVector totalOffset = totalOffsetFromNormals(lastNormal, nextNormal);
        SkVector AAOffset = totalOffset;
        AAOffset.fX *= 0.5f * inverseScaleX;
        AAOffset.fY *= 0.5f * inverseScaleY;

        SkVector innerOffset = totalOffset;
        scaleOffsetForStrokeWidth(innerOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);
        SkVector outerOffset = innerOffset + AAOffset;
        innerOffset -= AAOffset;

        AlphaVertex::set(&buffer[currentAAOuterIndex++],
                current->position[0] + outerOffset.fX,
                current->position[1] + outerOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[currentAAOuterIndex++],
                current->position[0] + innerOffset.fX,
                current->position[1] + innerOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[currentStrokeIndex++],
                current->position[0] + innerOffset.fX,
                current->position[1] + innerOffset.fY,
                maxAlpha);
        AlphaVertex::set(&buffer[currentStrokeIndex++],
                current->position[0] - innerOffset.fX,
                current->position[1] - innerOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[currentAAInnerIndex--],
                current->position[0] - innerOffset.fX,
                current->position[1] - innerOffset.fY,
                maxAlpha);
        AlphaVertex::set(&buffer[currentAAInnerIndex--],
                current->position[0] - outerOffset.fX,
                current->position[1] - outerOffset.fY,
                0.0f);

        last = current;
        current = next;
        lastNormal = nextNormal;
    }

    {
        // end cap
        SkVector totalOffset = lastNormal;
        SkVector AAOffset = totalOffset;
        AAOffset.fX *= 0.5f * inverseScaleX;
        AAOffset.fY *= 0.5f * inverseScaleY;

        SkVector innerOffset = totalOffset;
        scaleOffsetForStrokeWidth(innerOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);
        SkVector outerOffset = innerOffset + AAOffset;
        innerOffset -= AAOffset;

        // TODO: support square cap by changing this offset to incorporate halfStrokeWidth
        SkVector capAAOffset;
        capAAOffset.set(-AAOffset.fY, AAOffset.fX);

        AlphaVertex::set(&buffer[offset + 2],
                current->position[0] + outerOffset.fX + capAAOffset.fX,
                current->position[1] + outerOffset.fY + capAAOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[offset + 3],
                current->position[0] + innerOffset.fX - capAAOffset.fX,
                current->position[1] + innerOffset.fY - capAAOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[offset + 4],
                current->position[0] - outerOffset.fX + capAAOffset.fX,
                current->position[1] - outerOffset.fY + capAAOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[offset + 5],
                current->position[0] - innerOffset.fX - capAAOffset.fX,
                current->position[1] - innerOffset.fY - capAAOffset.fY,
                maxAlpha);

        copyAlphaVertex(&buffer[vertexBuffer->getSize() - 2], &buffer[offset + 3]);
        copyAlphaVertex(&buffer[vertexBuffer->getSize() - 1], &buffer[offset + 5]);
    }

#if VERTEX_DEBUG
    for (unsigned int i = 0; i < vertexBuffer.getSize(); i++) {
        SkDebugf("point at %f %f, alpha %f", buffer[i].position[0], buffer[i].position[1], buffer[i].alpha);
    }
#endif
}


static void getStrokeVerticesFromPerimeterAA(const SkTArray<Vertex, true>& perimeter, float halfStrokeWidth,
        VertexBuffer* vertexBuffer, float inverseScaleX, float inverseScaleY) {
    AlphaVertex* buffer = vertexBuffer->alloc<AlphaVertex>(6 * perimeter.count() + 8);

    // avoid lines smaller than hairline since they break triangle based sampling. instead reducing
    // alpha value (TODO: support different X/Y scale)
    float maxAlpha = 1.0f;
    if (halfStrokeWidth != 0 && inverseScaleX == inverseScaleY &&
            halfStrokeWidth * inverseScaleX < 0.5f) {
        maxAlpha *= (2 * halfStrokeWidth) / inverseScaleX;
        halfStrokeWidth = 0.0f;
    }

    int offset = 2 * perimeter.count() + 3;
    int currentAAOuterIndex = 0;
    int currentStrokeIndex = offset;
    int currentAAInnerIndex = offset * 2;

    const Vertex* last = &(perimeter[perimeter.count() - 1]);
    const Vertex* current = &(perimeter[0]);
    SkVector lastNormal;
    lastNormal.set(current->position[1] - last->position[1],
                   last->position[0] - current->position[0]);
    lastNormal.normalize();
    for (int i = 0; i < perimeter.count(); i++) {
        const Vertex* next = &(perimeter[i + 1 >= perimeter.count() ? 0 : i + 1]);
        SkVector nextNormal;
        nextNormal.set(next->position[1] - current->position[1],
                       current->position[0] - next->position[0]);
        nextNormal.normalize();

        SkVector totalOffset = totalOffsetFromNormals(lastNormal, nextNormal);
        SkVector AAOffset = totalOffset;
        AAOffset.fX *= 0.5f * inverseScaleX;
        AAOffset.fY *= 0.5f * inverseScaleY;

        SkVector innerOffset = totalOffset;
        scaleOffsetForStrokeWidth(innerOffset, halfStrokeWidth, inverseScaleX, inverseScaleY);
        SkVector outerOffset = innerOffset + AAOffset;
        innerOffset -= AAOffset;

        AlphaVertex::set(&buffer[currentAAOuterIndex++],
                current->position[0] + outerOffset.fX,
                current->position[1] + outerOffset.fY,
                0.0f);
        AlphaVertex::set(&buffer[currentAAOuterIndex++],
                current->position[0] + innerOffset.fX,
                current->position[1] + innerOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[currentStrokeIndex++],
                current->position[0] + innerOffset.fX,
                current->position[1] + innerOffset.fY,
                maxAlpha);
        AlphaVertex::set(&buffer[currentStrokeIndex++],
                current->position[0] - innerOffset.fX,
                current->position[1] - innerOffset.fY,
                maxAlpha);

        AlphaVertex::set(&buffer[currentAAInnerIndex++],
                current->position[0] - innerOffset.fX,
                current->position[1] - innerOffset.fY,
                maxAlpha);
        AlphaVertex::set(&buffer[currentAAInnerIndex++],
                current->position[0] - outerOffset.fX,
                current->position[1] - outerOffset.fY,
                0.0f);

        last = current;
        current = next;
        lastNormal = nextNormal;
    }

    // wrap each strip around to beginning, creating degenerate tris to bridge strips
    copyAlphaVertex(&buffer[currentAAOuterIndex++], &buffer[0]);
    copyAlphaVertex(&buffer[currentAAOuterIndex++], &buffer[1]);
    copyAlphaVertex(&buffer[currentAAOuterIndex++], &buffer[1]);

    copyAlphaVertex(&buffer[currentStrokeIndex++], &buffer[offset]);
    copyAlphaVertex(&buffer[currentStrokeIndex++], &buffer[offset + 1]);
    copyAlphaVertex(&buffer[currentStrokeIndex++], &buffer[offset + 1]);

    copyAlphaVertex(&buffer[currentAAInnerIndex++], &buffer[2 * offset]);
    copyAlphaVertex(&buffer[currentAAInnerIndex++], &buffer[2 * offset + 1]);
    // don't need to create last degenerate tri

#if VERTEX_DEBUG
    for (unsigned int i = 0; i < vertexBuffer.getSize(); i++) {
        SkDebugf("point at %f %f, alpha %f", buffer[i].position[0], buffer[i].position[1], buffer[i].alpha);
    }
#endif
}

void PathRenderer::ConvexPathVertices(const SkPath &path, const SkStrokeRec& stroke, bool isAA,
        const SkMatrix* transform, VertexBuffer* vertexBuffer) {

    SkStrokeRec::Style style = stroke.getStyle();

    float inverseScaleX, inverseScaleY;
    computeInverseScales(transform, inverseScaleX, inverseScaleY);

    SkTArray<Vertex, true> tempVertices;
    float threshInvScaleX = inverseScaleX;
    float threshInvScaleY = inverseScaleY;
    if (style == SkStrokeRec::kStroke_Style) {
        // alter the bezier recursion threshold values we calculate in order to compensate for
        // expansion done after the path vertices are found
        SkRect bounds = path.getBounds();
        if (!bounds.isEmpty()) {
            threshInvScaleX *= bounds.width() / (bounds.width() + stroke.getWidth());
            threshInvScaleY *= bounds.height() / (bounds.height() + stroke.getWidth());
        }
    }

    // force close if we're filling the path, since fill path expects closed perimeter.
    bool forceClose = style != SkStrokeRec::kStroke_Style;
    bool wasClosed = ConvexPathPerimeterVertices(path, forceClose, threshInvScaleX * threshInvScaleX,
            threshInvScaleY * threshInvScaleY, &tempVertices);

    if (!tempVertices.count()) {
        // path was empty, return without allocating vertex buffer
        return;
    }

#if VERTEX_DEBUG
    for (unsigned int i = 0; i < tempVertices.count(); i++) {
        SkDebugf("orig path: point at %f %f", tempVertices[i].position[0], tempVertices[i].position[1]);
    }
#endif

    if (style == SkStrokeRec::kStroke_Style) {
        float halfStrokeWidth = stroke.getWidth() * 0.5f;
        if (!isAA) {
            if (wasClosed) {
                getStrokeVerticesFromPerimeter(tempVertices, halfStrokeWidth, vertexBuffer,
                        inverseScaleX, inverseScaleY);
            } else {
                getStrokeVerticesFromUnclosedVertices(tempVertices, halfStrokeWidth, vertexBuffer,
                        inverseScaleX, inverseScaleY);
            }

        } else {
            if (wasClosed) {
                getStrokeVerticesFromPerimeterAA(tempVertices, halfStrokeWidth, vertexBuffer,
                        inverseScaleX, inverseScaleY);
            } else {
                getStrokeVerticesFromUnclosedVerticesAA(tempVertices, halfStrokeWidth, vertexBuffer,
                        inverseScaleX, inverseScaleY);
            }
        }
    } else {
        // For kStrokeAndFill style, the path should be adjusted externally, as it will be treated as a fill here.
        if (!isAA) {
            getFillVerticesFromPerimeter(tempVertices, vertexBuffer);
        } else {
            getFillVerticesFromPerimeterAA(tempVertices, vertexBuffer, inverseScaleX, inverseScaleY);
        }
    }
}


static void pushToVector(SkTArray<Vertex, true>* vertices, float x, float y) {
    // TODO: make this not yuck
    vertices->push_back();
    Vertex* newVertex = &((*vertices)[vertices->count() - 1]);
    Vertex::set(newVertex, x, y);
}

bool PathRenderer::ConvexPathPerimeterVertices(const SkPath& path, bool forceClose,
        float sqrInvScaleX, float sqrInvScaleY, SkTArray<Vertex, true>* outputVertices) {


    // TODO: to support joins other than sharp miter, join vertices should be labelled in the
    // perimeter, or resolved into more vertices. Reconsider forceClose-ing in that case.
    SkPath::Iter iter(path, forceClose);
    SkPoint pts[4];
    SkPath::Verb v;

    while (SkPath::kDone_Verb != (v = iter.next(pts))) {
            switch (v) {
                case SkPath::kMove_Verb:
                    pushToVector(outputVertices, pts[0].x(), pts[0].y());
                    ALOGV("Move to pos %f %f", pts[0].x(), pts[0].y());
                    break;
                case SkPath::kClose_Verb:
                    ALOGV("Close at pos %f %f", pts[0].x(), pts[0].y());
                    break;
                case SkPath::kLine_Verb:
                    ALOGV("kLine_Verb %f %f -> %f %f",
                            pts[0].x(), pts[0].y(),
                            pts[1].x(), pts[1].y());

                    pushToVector(outputVertices, pts[1].x(), pts[1].y());
                    break;
                case SkPath::kQuad_Verb:
                    ALOGV("kQuad_Verb");
                    RecursiveQuadraticBezierVertices(
                            pts[0].x(), pts[0].y(),
                            pts[2].x(), pts[2].y(),
                            pts[1].x(), pts[1].y(),
                            sqrInvScaleX, sqrInvScaleY, outputVertices);
                    break;
                case SkPath::kCubic_Verb:
                    ALOGV("kCubic_Verb");
                    RecursiveCubicBezierVertices(
                            pts[0].x(), pts[0].y(),
                            pts[1].x(), pts[1].y(),
                            pts[3].x(), pts[3].y(),
                            pts[2].x(), pts[2].y(),
                        sqrInvScaleX, sqrInvScaleY, outputVertices);
                    break;
                default:
                    break;
            }
    }

    int size = outputVertices->count();
    if (size >= 2 && (*outputVertices)[0].position[0] == (*outputVertices)[size - 1].position[0] &&
            (*outputVertices)[0].position[1] == (*outputVertices)[size - 1].position[1]) {
        outputVertices->pop_back();
        return true;
    }
    return false;
}

void PathRenderer::RecursiveCubicBezierVertices(
        float p1x, float p1y, float c1x, float c1y,
        float p2x, float p2y, float c2x, float c2y,
        float sqrInvScaleX, float sqrInvScaleY, SkTArray<Vertex, true>* outputVertices) {
    float dx = p2x - p1x;
    float dy = p2y - p1y;
    float d1 = sk_float_abs((c1x - p2x) * dy - (c1y - p2y) * dx);
    float d2 = sk_float_abs((c2x - p2x) * dy - (c2y - p2y) * dx);
    float d = d1 + d2;

    // multiplying by sqrInvScaleY/X equivalent to multiplying in dimensional scale factors

    if (d * d < THRESHOLD * THRESHOLD * (dx * dx * sqrInvScaleY + dy * dy * sqrInvScaleX)) {
        // below thresh, draw line by adding endpoint
        pushToVector(outputVertices, p2x, p2y);
    } else {
        float p1c1x = (p1x + c1x) * 0.5f;
        float p1c1y = (p1y + c1y) * 0.5f;
        float p2c2x = (p2x + c2x) * 0.5f;
        float p2c2y = (p2y + c2y) * 0.5f;

        float c1c2x = (c1x + c2x) * 0.5f;
        float c1c2y = (c1y + c2y) * 0.5f;

        float p1c1c2x = (p1c1x + c1c2x) * 0.5f;
        float p1c1c2y = (p1c1y + c1c2y) * 0.5f;

        float p2c1c2x = (p2c2x + c1c2x) * 0.5f;
        float p2c1c2y = (p2c2y + c1c2y) * 0.5f;

        float mx = (p1c1c2x + p2c1c2x) * 0.5f;
        float my = (p1c1c2y + p2c1c2y) * 0.5f;

        RecursiveCubicBezierVertices(
                p1x, p1y, p1c1x, p1c1y,
                mx, my, p1c1c2x, p1c1c2y,
                sqrInvScaleX, sqrInvScaleY, outputVertices);
        RecursiveCubicBezierVertices(
                mx, my, p2c1c2x, p2c1c2y,
                p2x, p2y, p2c2x, p2c2y,
                sqrInvScaleX, sqrInvScaleY, outputVertices);
    }
}

void PathRenderer::RecursiveQuadraticBezierVertices(
        float ax, float ay,
        float bx, float by,
        float cx, float cy,
        float sqrInvScaleX, float sqrInvScaleY, SkTArray<Vertex, true>* outputVertices) {
    float dx = bx - ax;
    float dy = by - ay;
    float d = (cx - bx) * dy - (cy - by) * dx;

    if (d * d < THRESHOLD * THRESHOLD * (dx * dx * sqrInvScaleY + dy * dy * sqrInvScaleX)) {
        // below thresh, draw line by adding endpoint
        pushToVector(outputVertices, bx, by);
    } else {
        float acx = (ax + cx) * 0.5f;
        float bcx = (bx + cx) * 0.5f;
        float acy = (ay + cy) * 0.5f;
        float bcy = (by + cy) * 0.5f;

        // midpoint
        float mx = (acx + bcx) * 0.5f;
        float my = (acy + bcy) * 0.5f;

        RecursiveQuadraticBezierVertices(ax, ay, mx, my, acx, acy,
                sqrInvScaleX, sqrInvScaleY, outputVertices);
        RecursiveQuadraticBezierVertices(mx, my, bx, by, bcx, bcy,
                sqrInvScaleX, sqrInvScaleY, outputVertices);
    }
}

}; // namespace uirenderer
}; // namespace android
