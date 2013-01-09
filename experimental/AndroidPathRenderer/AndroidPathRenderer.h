/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ANDROID_HWUI_PATH_RENDERER_H
#define ANDROID_HWUI_PATH_RENDERER_H

#include <SkTArray.h>

#include "Vertex.h"

class SkMatrix;

namespace android {
namespace uirenderer {

class VertexBuffer {
public:
    VertexBuffer():
        mBuffer(0),
        mSize(0),
        mCleanupMethod(0)
    {}

    ~VertexBuffer() {
        if (mCleanupMethod)
            mCleanupMethod(mBuffer);
    }

    template <class TYPE>
    TYPE* alloc(int size) {
        mSize = size;
        mBuffer = (void*)new TYPE[size];
        mCleanupMethod = &(cleanup<TYPE>);

        return (TYPE*)mBuffer;
    }

    void* getBuffer() { return mBuffer; }
    unsigned int getSize() { return mSize; }

private:
    template <class TYPE>
    static void cleanup(void* buffer) {
        delete[] (TYPE*)buffer;
    }

    void* mBuffer;
    unsigned int mSize;
    void (*mCleanupMethod)(void*);
};

class PathRenderer {
public:
    static SkRect ComputePathBounds(const SkPath& path, const SkPaint* paint);

    static void ConvexPathVertices(const SkPath& path, const SkStrokeRec& stroke, bool isAA,
            const SkMatrix* transform, VertexBuffer* vertexBuffer);

private:
    static bool ConvexPathPerimeterVertices(const SkPath &path, bool forceClose,
        float sqrInvScaleX, float sqrInvScaleY, SkTArray<Vertex, true>* outputVertices);

/*
  endpoints a & b,
  control c
 */
    static void RecursiveQuadraticBezierVertices(
            float ax, float ay,
            float bx, float by,
            float cx, float cy,
            float sqrInvScaleX, float sqrInvScaleY,
            SkTArray<Vertex, true>* outputVertices);

/*
  endpoints p1, p2
  control c1, c2
 */
    static void RecursiveCubicBezierVertices(
            float p1x, float p1y,
            float c1x, float c1y,
            float p2x, float p2y,
            float c2x, float c2y,
            float sqrInvScaleX, float sqrInvScaleY,
            SkTArray<Vertex, true>* outputVertices);
};

}; // namespace uirenderer
}; // namespace android

#endif // ANDROID_HWUI_PATH_RENDERER_H
