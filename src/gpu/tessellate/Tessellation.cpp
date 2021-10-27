/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/Tessellation.h"

#include "include/core/SkPath.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

namespace skgpu {

VertexWriter WritePathMiddleOutInnerFan(VertexWriter&& vertexWriter,
                                        int pad32Count,
                                        uint32_t pad32Value,
                                        const SkMatrix& matrix,
                                        const SkPath& path,
                                        int* numTrianglesWritten) {
    MiddleOutPolygonTriangulator middleOut(std::move(vertexWriter),
                                           pad32Count,
                                           pad32Value,
                                           path.countVerbs());
    PathXform pathXform(matrix);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            SkPoint pt;
            case SkPathVerb::kMove:
                middleOut.closeAndMove(pathXform.mapPoint(pts[0]));
                break;
            case SkPathVerb::kLine:
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                pt = pts[SkPathPriv::PtsInIter((unsigned)verb) - 1];
                middleOut.pushVertex(pathXform.mapPoint(pt));
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
    *numTrianglesWritten = middleOut.close();
    return middleOut.detachVertexWriter();
}

}  // namespace skgpu
