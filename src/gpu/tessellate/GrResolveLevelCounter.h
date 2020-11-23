/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResolveLevelCounter_DEFINED
#define GrResolveLevelCounter_DEFINED

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

// This class helps bin instances by log2 "resolveLevel" when we don't use hardware tessellation. It
// is composed of simple counters that track how many instances we intend to draw at each
// resolveLevel, and how many resolveLevels there are that have at least one instances.
class GrResolveLevelCounter {
public:
    void reset() {
        memset(fInstanceCounts, 0, sizeof(fInstanceCounts));
        SkDEBUGCODE(fHasCalledReset = true;)
    }

    void reset(const SkPath& path, const SkMatrix& viewMatrix, float intolerance) {
        this->reset();
        GrVectorXform xform(viewMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kConic:
                    // We use the same quadratic formula for conics, ignoring w. This appears to be
                    // an upper bound on what the actual number of subdivisions would have been.
                    [[fallthrough]];
                case SkPathVerb::kQuad:
                    this->countInstance(GrWangsFormula::quadratic_log2(intolerance, pts, xform));
                    break;
                case SkPathVerb::kCubic:
                    this->countInstance(GrWangsFormula::cubic_log2(intolerance, pts, xform));
                    break;
                default:
                    break;
            }
        }
    }

    void countInstance(int resolveLevel) {
        SkASSERT(fHasCalledReset);
        SkASSERT(resolveLevel >= 0);
        if (resolveLevel == 0) {
            // Instances with 2^0=1 segments are empty (zero area). We ignore them completely.
            return;
        }
        resolveLevel = std::min(resolveLevel, GrTessellationPathRenderer::kMaxResolveLevel);
        if (!fInstanceCounts[resolveLevel]++) {
            ++fTotalIndirectDrawCount;
        }
        ++fTotalInstanceCount;
    }

    int operator[](int resolveLevel) const {
        SkASSERT(fHasCalledReset);
        SkASSERT(resolveLevel > 0);  // Empty instances with 2^0=1 segments do not need to be drawn.
        SkASSERT(resolveLevel <= GrTessellationPathRenderer::kMaxResolveLevel);
        return fInstanceCounts[resolveLevel];
    }
    int totalInstanceCount() const { return fTotalInstanceCount; }
    int totalIndirectDrawCount() const { return fTotalIndirectDrawCount; }

private:
    SkDEBUGCODE(bool fHasCalledReset = false;)
    int fInstanceCounts[GrTessellationPathRenderer::kMaxResolveLevel + 1];
    int fTotalInstanceCount = 0;
    int fTotalIndirectDrawCount = 0;
};

#endif
