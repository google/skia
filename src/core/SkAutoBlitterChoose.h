/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoBlitterChoose_DEFINED
#define SkAutoBlitterChoose_DEFINED

#include "SkArenaAlloc.h"
#include "SkBlitter.h"

class SkMatrix;
class SkPaint;
class SkPixmap;

class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {
        fBlitter = nullptr;
    }
    SkAutoBlitterChoose(const SkPixmap& dst, const SkMatrix& matrix,
                        const SkPaint& paint, bool drawCoverage = false) {
        fBlitter = SkBlitter::Choose(dst, matrix, paint, &fAlloc, drawCoverage);
    }

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    void choose(const SkPixmap& dst, const SkMatrix& matrix,
                const SkPaint& paint, bool drawCoverage = false) {
        SkASSERT(!fBlitter);
        fBlitter = SkBlitter::Choose(dst, matrix, paint, &fAlloc, drawCoverage);
    }

private:
    // Owned by fAlloc, which will handle the delete.
    SkBlitter*          fBlitter;

    SkSTArenaAlloc<kSkBlitterContextSize> fAlloc;
};
#define SkAutoBlitterChoose(...) SK_REQUIRE_LOCAL_VAR(SkAutoBlitterChoose)

#endif
