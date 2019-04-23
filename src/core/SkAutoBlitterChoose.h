/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoBlitterChoose_DEFINED
#define SkAutoBlitterChoose_DEFINED

#include "include/private/SkArenaAlloc.h"
#include "include/private/SkMacros.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkDraw.h"

class SkMatrix;
class SkPaint;
class SkPixmap;

class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {}
    SkAutoBlitterChoose(const SkDraw& draw, const SkMatrix* matrix, const SkPaint& paint,
                        bool drawCoverage = false) {
        this->choose(draw, matrix, paint, drawCoverage);
    }

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    SkBlitter* choose(const SkDraw& draw, const SkMatrix* matrix, const SkPaint& paint,
                      bool drawCoverage = false) {
        SkASSERT(!fBlitter);
        if (!matrix) {
            matrix = draw.fMatrix;
        }
        fBlitter = SkBlitter::Choose(draw.fDst, *matrix, paint, &fAlloc, drawCoverage);

        if (draw.fCoverage) {
            // hmm, why can't choose ignore the paint if drawCoverage is true?
            SkBlitter* coverageBlitter = SkBlitter::Choose(*draw.fCoverage, *matrix, SkPaint(),
                                                           &fAlloc, true);
            fBlitter = fAlloc.make<SkPairBlitter>(fBlitter, coverageBlitter);
        }
        return fBlitter;
    }

private:
    // Owned by fAlloc, which will handle the delete.
    SkBlitter* fBlitter = nullptr;

    SkSTArenaAlloc<kSkBlitterContextSize> fAlloc;
};
#define SkAutoBlitterChoose(...) SK_REQUIRE_LOCAL_VAR(SkAutoBlitterChoose)

#endif
