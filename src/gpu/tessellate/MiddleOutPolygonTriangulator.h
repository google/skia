/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_MiddleOutPolygonTriangulator_DEFINED
#define skgpu_tessellate_MiddleOutPolygonTriangulator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPathPriv.h"
#include <tuple>

namespace skgpu {

// This class generates a middle-out triangulation of a polygon. Conceptually, middle-out emits one
// large triangle with vertices on both endpoints and a middle point, then recurses on both sides of
// the new triangle. i.e.:
//
//     void emit_middle_out_triangulation(int startIdx, int endIdx) {
//         if (startIdx + 1 == endIdx) {
//             return;
//         }
//         int middleIdx = startIdx + SkNextPow2(endIdx - startIdx) / 2;
//
//         // Recurse on the left half.
//         emit_middle_out_triangulation(startIdx, middleIdx);
//
//         // Emit a large triangle with vertices on both endpoints and a middle point.
//         emit_triangle(vertices[startIdx], vertices[middleIdx], vertices[endIdx - 1]);
//
//         // Recurse on the right half.
//         emit_middle_out_triangulation(middleIdx, endIdx);
//     }
//
// Middle-out produces drastically less work for the rasterizer as compared a linear triangle strip
// or fan.
//
// This class is designed to not know or store all the vertices in the polygon at once. The caller
// pushes each vertex in linear order (perhaps while parsing a path), then rather than relying on
// recursion, we manipulate an O(log N) stack to determine the correct middle-out triangulation.
class MiddleOutPolygonTriangulator {
private:
    // Internal representation of how we store vertices on our stack.
    struct StackVertex {
        SkPoint fPoint;
        // How many polygon vertices away is this vertex from the previous vertex on the stack?
        // i.e., the ith stack element's vertex index in the original polygon is:
        //
        //     fVertexStack[i].fVertexIdxDelta + fVertexStack[i - 1].fVertexIdxDelta + ... +
        //     fVertexStack[1].fVertexIdxDelta.
        //
        // NOTE: fVertexStack[0].fVertexIdxDelta always == 0.
        int fVertexIdxDelta;
    };

public:
    // RAII. This class is designed to first allow the caller to iterate the triangles that will be
    // popped off our stack, and then (during the destructor) it actually pops the finished vertices
    // and pushes a new one. Example usage:
    //
    //   for (auto [p0, p1, p2] : middleOut.pushVertex(pt)) {
    //       vertexWriter << p0 << p1 << p2;
    //   }
    //
    // The above code iterates over the triangles being popped, and then once iteration is finished,
    // the PoppedTriangleStack is destroyed, triggering the pending stack update.
    class PoppedTriangleStack {
    public:
        PoppedTriangleStack(MiddleOutPolygonTriangulator* middleOut,
                            SkPoint lastPoint,
                            StackVertex* end,
                            StackVertex* newTopVertex,
                            StackVertex newTopValue)
            : fMiddleOut(middleOut)
            , fLastPoint(lastPoint)
            , fEnd(end)
            , fNewTopVertex(newTopVertex)
            , fNewTopValue(newTopValue) {
        }

        PoppedTriangleStack(PoppedTriangleStack&& that) {
            memcpy(this, &that, sizeof(*this));
            that.fMiddleOut = nullptr;  // Don't do a stack update during our destructor.
        }

        ~PoppedTriangleStack() {
            if (fMiddleOut) {
                fMiddleOut->fTop = fNewTopVertex;
                *fNewTopVertex = fNewTopValue;
            }
        }

        struct Iter {
            bool operator!=(const Iter& iter) { return fVertex != iter.fVertex; }
            void operator++() { --fVertex; }
            std::tuple<SkPoint, SkPoint, SkPoint> operator*() {
                return {fVertex[-1].fPoint, fVertex[0].fPoint, fLastPoint};
            }
            StackVertex* fVertex;
            SkPoint fLastPoint;
        };

        Iter begin() const { return {fMiddleOut ? fMiddleOut->fTop : fEnd, fLastPoint}; }
        Iter end() const { return {fEnd, fLastPoint}; }

    private:
        MiddleOutPolygonTriangulator* fMiddleOut;
        SkPoint fLastPoint;
        StackVertex* fEnd;
        StackVertex* fNewTopVertex;
        StackVertex fNewTopValue;
    };

    // maxPushVertexCalls is an upper bound on the number of times the caller will call
    // pushVertex(). The caller must not call it more times than this. (Beware of int overflow.)
    MiddleOutPolygonTriangulator(int maxPushVertexCalls, SkPoint startPoint = {0,0}) {
        SkASSERT(maxPushVertexCalls >= 0);
        // Determine the deepest our stack can ever go.
        int maxStackDepth = SkNextLog2(maxPushVertexCalls) + 1;
        if (maxStackDepth > kStackPreallocCount) {
            fVertexStack.reset(maxStackDepth);
        }
        SkDEBUGCODE(fStackAllocCount = maxStackDepth;)
        // The stack will always contain a starting point. This is an implicit moveTo(0, 0)
        // initially, but will be overridden if moveTo() gets called before adding geometry.
        fVertexStack[0] = {startPoint, 0};
        fTop = fVertexStack;
    }

    // Returns an RAII object that first allows the caller to iterate the triangles we will pop,
    // pops those triangles, and finally pushes 'pt' onto the vertex stack.
    SK_WARN_UNUSED_RESULT PoppedTriangleStack pushVertex(SkPoint pt) {
        // Our topology wants triangles that have the same vertexIdxDelta on both sides:
        // e.g., a run of 9 points should be triangulated as:
        //
        //    [0, 1, 2], [2, 3, 4], [4, 5, 6], [6, 7, 8]  // vertexIdxDelta == 1
        //    [0, 2, 4], [4, 6, 8]  // vertexIdxDelta == 2
        //    [0, 4, 8]  // vertexIdxDelta == 4
        //
        // Find as many new triangles as we can pop off the stack that have equal-delta sides. (This
        // is a stack-based implementation of the recursive example method from the class comment.)
        StackVertex* endVertex = fTop;
        int vertexIdxDelta = 1;
        while (endVertex->fVertexIdxDelta == vertexIdxDelta) {
            --endVertex;
            vertexIdxDelta *= 2;
        }

        // Once the above triangles are popped, push 'pt' to the top of the stack.
        StackVertex* newTopVertex = endVertex + 1;
        StackVertex newTopValue = {pt, vertexIdxDelta};
        SkASSERT(newTopVertex < fVertexStack + fStackAllocCount); // Is fStackAllocCount enough?

        return PoppedTriangleStack(this, pt, endVertex, newTopVertex, newTopValue);
    }

    // Returns an RAII object that first allows the caller to iterate the remaining triangles, then
    // resets the vertex stack with newStartPoint.
    SK_WARN_UNUSED_RESULT PoppedTriangleStack closeAndMove(SkPoint newStartPoint) {
        // Add an implicit line back to the starting point.
        SkPoint startPt = fVertexStack[0].fPoint;

        // Triangulate the rest of the polygon. Since we simply have to finish now, we can't be
        // picky anymore about getting a pure middle-out topology.
        StackVertex* endVertex = std::min(fTop, fVertexStack + 1);

        // Once every remaining triangle is popped, reset the vertex stack with newStartPoint.
        StackVertex* newTopVertex = fVertexStack;
        StackVertex newTopValue = {newStartPoint, 0};

        return PoppedTriangleStack(this, startPt, endVertex, newTopVertex, newTopValue);
    }

    // Returns an RAII object that first allows the caller to iterate the remaining triangles, then
    // resets the vertex stack with the same starting point as it had before.
    SK_WARN_UNUSED_RESULT PoppedTriangleStack close() {
        return this->closeAndMove(fVertexStack[0].fPoint);
    }

private:
    constexpr static int kStackPreallocCount = 32;
    SkAutoSTMalloc<kStackPreallocCount, StackVertex> fVertexStack;
    SkDEBUGCODE(int fStackAllocCount;)
    StackVertex* fTop;
};

// This is a helper class that transforms and pushes a path's inner fan vertices onto a
// MiddleOutPolygonTriangulator. Example usage:
//
//   for (PathMiddleOutFanIter it(pathMatrix, path); !it.done();) {
//       for (auto [p0, p1, p2] : it.nextStack()) {
//           vertexWriter << p0 << p1 << p2;
//       }
//   }
//
class PathMiddleOutFanIter {
public:
    PathMiddleOutFanIter(const SkPath& path) : fMiddleOut(path.countVerbs()) {
        SkPathPriv::Iterate it(path);
        fPathIter = it.begin();
        fPathEnd = it.end();
    }

    bool done() const { return fDone; }

    MiddleOutPolygonTriangulator::PoppedTriangleStack nextStack() {
        SkASSERT(!fDone);
        if (fPathIter == fPathEnd) {
            fDone = true;
            return fMiddleOut.close();
        }
        switch (auto [verb, pts, w] = *fPathIter++; verb) {
            SkPoint pt;
            case SkPathVerb::kMove:
                return fMiddleOut.closeAndMove(pts[0]);
            case SkPathVerb::kLine:
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                pt = pts[SkPathPriv::PtsInIter((unsigned)verb) - 1];
                return fMiddleOut.pushVertex(pt);
            case SkPathVerb::kClose:
                return fMiddleOut.close();
        }
        SkUNREACHABLE;
    }

private:
    MiddleOutPolygonTriangulator fMiddleOut;
    SkPathPriv::RangeIter fPathIter;
    SkPathPriv::RangeIter fPathEnd;
    bool fDone = false;
};

}  // namespace skgpu

#endif  // skgpu_tessellate_MiddleOutPolygonTriangulator_DEFINED
