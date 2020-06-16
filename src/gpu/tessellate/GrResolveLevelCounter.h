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

// This class helps bin cubics by log2 "resolveLevel" when we don't use hardware tessellation. It is
// composed of simple counters that track how many cubics we intend to draw at each resolveLevel,
// and how many resolveLevels there are that have at least one cubic.
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
                case SkPathVerb::kQuad:
                    // Quadratics get converted to cubics before rendering.
                    this->countCubic(GrWangsFormula::quadratic_log2(intolerance, pts, xform));
                    break;
                case SkPathVerb::kCubic:
                    this->countCubic(GrWangsFormula::cubic_log2(intolerance, pts, xform));
                    break;
                default:
                    break;
            }
        }
    }

    void countCubic(int resolveLevel) {
        SkASSERT(fHasCalledReset);
        SkASSERT(resolveLevel >= 0);
        if (resolveLevel == 0) {
            // Cubics with 2^0=1 segments are empty (zero area). We ignore them completely.
            return;
        }
        resolveLevel = std::min(resolveLevel, GrTessellationPathRenderer::kMaxResolveLevel);
        if (!fInstanceCounts[resolveLevel]++) {
            ++fTotalCubicIndirectDrawCount;
        }
        ++fTotalCubicInstanceCount;
    }

    int operator[](int resolveLevel) const {
        SkASSERT(fHasCalledReset);
        SkASSERT(resolveLevel > 0);  // Empty cubics with 2^0=1 segments do not need to be drawn.
        SkASSERT(resolveLevel <= GrTessellationPathRenderer::kMaxResolveLevel);
        return fInstanceCounts[resolveLevel];
    }
    int totalCubicInstanceCount() const { return fTotalCubicInstanceCount; }
    int totalCubicIndirectDrawCount() const { return fTotalCubicIndirectDrawCount; }

private:
    SkDEBUGCODE(bool fHasCalledReset = false;)
    int fInstanceCounts[GrTessellationPathRenderer::kMaxResolveLevel + 1];
    int fTotalCubicInstanceCount = 0;
    int fTotalCubicIndirectDrawCount = 0;
};

#endif
