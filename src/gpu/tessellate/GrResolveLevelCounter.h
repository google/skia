/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResolveLevelCounter_DEFINED
#define GrResolveLevelCounter_DEFINED

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

// This class is composed of simple counters for tracking how many curves we intend to draw at each
// resolveLevel, and how many resolve levels there are that have at least one curve. This is for
// when we don't use hardware tessellation -- we bin curves by resolveLevel and issue an indirect
// draw to render them all.
class GrResolveLevelCounter {
public:
    GrResolveLevelCounter(int maxInstanceCount) {
        memset(fInstanceCounts, 0, sizeof(fInstanceCounts));
    }

    bool countBezier(int resolveLevel) {
        if (resolveLevel == 0) {
            return false;
        }
        resolveLevel = std::min(resolveLevel, GrMiddleOutCubicShader::kMaxResolveLevel);
        if (!fInstanceCounts[resolveLevel]++) {
            ++fTotalCurveDrawIndirectCount;
        }
        ++fTotalCurveInstanceCount;
        return true;
    }

    void countAllBeziersInPath(float intolerance, const SkPath& path, const SkMatrix& viewMatrix) {
        GrVectorXform xform(viewMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kQuad:
                    this->countBezier(GrWangsFormula::quadratic_log2(intolerance, pts, xform));
                    break;
                case SkPathVerb::kCubic:
                    this->countBezier(GrWangsFormula::cubic_log2(intolerance, pts, xform));
                    break;
                default:
                    break;
            }
        }
    }

    int operator[](int resolveLevel) const {
        SkASSERT(resolveLevel > 0);  // Use triangleInstanceCount() instead.
        SkASSERT(resolveLevel <= GrMiddleOutCubicShader::kMaxResolveLevel);
        return fInstanceCounts[resolveLevel];
    }
    int totalCurveInstanceCount() const { return fTotalCurveInstanceCount; }
    int totalCurveDrawIndirectCount() const { return fTotalCurveDrawIndirectCount; }

    // For performance reasons we can often express triangles as an indirect cubic draw and sneak
    // them in alongside the other indirect draws. This class provides a convenient location for
    // the caller to store how many triangles it wishes to draw this way.
    //
    // NOTE: This does *not* imply that flat cubics with resolveLevel == 0 turn into triangles; we
    // don't draw flat cubics at all, so element 0 in the fInstanceCounts array is otherwise unused.
    void setTriangleInstanceCount(int count) { fInstanceCounts[0] = count; }
    int triangleInstanceCount() const { return fInstanceCounts[0]; }

private:
    int fInstanceCounts[GrMiddleOutCubicShader::kMaxResolveLevel + 1];
    int fTotalCurveInstanceCount = 0;
    int fTotalCurveDrawIndirectCount = 0;
};

#endif
