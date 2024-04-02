/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/tessellate/FixedCountBufferUtils.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/BufferWriter.h"

#include <array>
#include <utility>

using namespace skia_private;

namespace skgpu::tess {

namespace {

void write_curve_index_buffer_base_index(VertexWriter vertexWriter,
                                         size_t bufferSize,
                                         uint16_t baseIndex) {
    int triangleCount = bufferSize / (sizeof(uint16_t) * 3);
    SkASSERT(triangleCount >= 1);
    TArray<std::array<uint16_t, 3>> indexData(triangleCount);

    // Connect the vertices with a middle-out triangulation. Refer to InitFixedCountVertexBuffer()
    // for the exact vertex ordering.
    //
    // Resolve level 1 is just a single triangle at T=[0, 1/2, 1].
    const auto* neighborInLastResolveLevel = &indexData.push_back({baseIndex,
                                                                   (uint16_t)(baseIndex + 2),
                                                                   (uint16_t)(baseIndex + 1)});

    // Resolve levels 2..maxResolveLevel
    int maxResolveLevel = SkPrevLog2(triangleCount + 1);
    uint16_t nextIndex = baseIndex + 3;
    SkASSERT(NumCurveTrianglesAtResolveLevel(maxResolveLevel) == triangleCount);
    for (int resolveLevel = 2; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        SkDEBUGCODE(auto* firstTriangleInCurrentResolveLevel = indexData.end());
        int numOuterTrianglelsInResolveLevel = 1 << (resolveLevel - 1);
        SkASSERT(numOuterTrianglelsInResolveLevel % 2 == 0);
        int numTrianglePairsInResolveLevel = numOuterTrianglelsInResolveLevel >> 1;
        for (int i = 0; i < numTrianglePairsInResolveLevel; ++i) {
            // First triangle shares the left edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[0],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[1]});
            // Second triangle shares the right edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[1],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[2]});
            ++neighborInLastResolveLevel;
        }
        SkASSERT(neighborInLastResolveLevel == firstTriangleInCurrentResolveLevel);
    }
    SkASSERT(indexData.size() == triangleCount);
    SkASSERT(nextIndex == baseIndex + triangleCount + 2);

    vertexWriter << VertexWriter::Array(indexData.data(), indexData.size());
}

}  // namespace

void FixedCountCurves::WriteVertexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(SkPoint) * 2);
    int vertexCount = bufferSize / sizeof(SkPoint);
    SkASSERT(vertexCount > 3);
    SkDEBUGCODE(auto end = vertexWriter.mark(vertexCount * sizeof(SkPoint));)

    // Lay out the vertices in "middle-out" order:
    //
    // T= 0/1, 1/1,              ; resolveLevel=0
    //    1/2,                   ; resolveLevel=1  (0/2 and 2/2 are already in resolveLevel 0)
    //    1/4, 3/4,              ; resolveLevel=2  (2/4 is already in resolveLevel 1)
    //    1/8, 3/8, 5/8, 7/8,    ; resolveLevel=3  (2/8 and 6/8 are already in resolveLevel 2)
    //    ...                    ; resolveLevel=...
    //
    // Resolve level 0 is just the beginning and ending vertices.
    vertexWriter << (float)0/*resolveLevel*/ << (float)0/*idx*/;
    vertexWriter << (float)0/*resolveLevel*/ << (float)1/*idx*/;

    // Resolve levels 1..kMaxResolveLevel.
    int maxResolveLevel = SkPrevLog2(vertexCount - 1);
    SkASSERT((1 << maxResolveLevel) + 1 == vertexCount);
    for (int resolveLevel = 1; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        int numSegmentsInResolveLevel = 1 << resolveLevel;
        // Write out the odd vertices in this resolveLevel. The even vertices were already written
        // out in previous resolveLevels and will be indexed from there.
        for (int i = 1; i < numSegmentsInResolveLevel; i += 2) {
            vertexWriter << (float)resolveLevel << (float)i;
        }
    }

    SkASSERT(vertexWriter.mark() == end);
}

void FixedCountCurves::WriteIndexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
   write_curve_index_buffer_base_index(std::move(vertexWriter), bufferSize, /*baseIndex=*/0);
}

void FixedCountWedges::WriteVertexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(SkPoint));

    // Start out with the fan point. A negative resolve level indicates the fan point.
    vertexWriter << -1.f/*resolveLevel*/ << -1.f/*idx*/;

    // The rest is the same as for curves.
    FixedCountCurves::WriteVertexBuffer(std::move(vertexWriter), bufferSize - sizeof(SkPoint));
}

void FixedCountWedges::WriteIndexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(uint16_t) * 3);

    // Start out with the fan triangle.
    vertexWriter << (uint16_t)0 << (uint16_t)1 << (uint16_t)2;

    // The rest is the same as for curves, with a baseIndex of 1.
    write_curve_index_buffer_base_index(std::move(vertexWriter),
                                        bufferSize - sizeof(uint16_t) * 3,
                                        /*baseIndex=*/1);
}

void FixedCountStrokes::WriteVertexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    int edgeCount = bufferSize / (sizeof(float) * 2);
    for (int i = 0; i < edgeCount; ++i) {
        vertexWriter << (float)i << (float)-i;
    }
}

}  // namespace skgpu::tess
