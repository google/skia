/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoBlitterChoose_DEFINED
#define SkAutoBlitterChoose_DEFINED

#include "include/private/SkMacros.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"

class SkMatrix;
class SkPaint;
class SkPixmap;

class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {}
    SkAutoBlitterChoose(const SkDraw& draw, const SkMatrixProvider* matrixProvider,
                        const SkPaint& paint, bool drawCoverage = false) {
        this->choose(draw, matrixProvider, paint, drawCoverage);
    }

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    SkBlitter* choose(const SkDraw& draw, const SkMatrixProvider* matrixProvider,
                      const SkPaint& paint, bool drawCoverage = false) {
        SkASSERT(!fBlitter);
        if (!matrixProvider) {
            matrixProvider = draw.fMatrixProvider;
        }
        fBlitter = SkBlitter::Choose(draw.fDst, *matrixProvider, paint, &fAlloc, drawCoverage,
                                     draw.fRC->clipShader());
        return fBlitter;
    }

private:
    // Owned by fAlloc, which will handle the delete.
    SkBlitter* fBlitter = nullptr;

    SkSTArenaAlloc<kSkBlitterContextSize> fAlloc;
};

#endif
