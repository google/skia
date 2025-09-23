/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoBlitterChoose_DEFINED
#define SkAutoBlitterChoose_DEFINED

#include "include/private/base/SkMacros.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSurfacePriv.h"

class SkMatrix;
class SkPaint;
class SkPixmap;

// This was determined experimentally by adding logging to SkSTArenaAlloc's destructor
// to see what the biggest size observed was while doing some browsing on Chromium.
// It's a bit tricky to determine this value statically, as the SkRasterPipelineBuilder
// uses the allocator for several things, as do the shaders which make use of the legacy
// shader context. In other cases it's easier because the allocator only has the blitter
// itself and one could do a static_assert using sizeof().
using SkBlitterSizedArena = SkSTArenaAlloc<2736>;

class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {}
    SkAutoBlitterChoose(const skcpu::Draw& draw,
                        const SkMatrix* ctm,
                        const SkPaint& paint,
                        const SkRect& devBounds,
                        SkDrawCoverage drawCoverage = SkDrawCoverage::kNo) {
        this->choose(draw, ctm, paint, devBounds, drawCoverage);
    }

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    SkBlitter* choose(const skcpu::Draw& draw,
                      const SkMatrix* ctm,
                      const SkPaint& paint,
                      const SkRect& devBounds,
                      SkDrawCoverage drawCoverage = SkDrawCoverage::kNo) {
        SkASSERT(!fBlitter);
        fBlitter = draw.fBlitterChooser(draw.fDst,
                                        ctm ? *ctm : *draw.fCTM,
                                        paint,
                                        &fAlloc,
                                        drawCoverage,
                                        draw.fRC->clipShader(),
                                        SkSurfacePropsCopyOrDefault(draw.fProps),
                                        devBounds);
        return fBlitter;
    }

private:
    // Owned by fAlloc, which will handle the delete.
    SkBlitter* fBlitter = nullptr;

    SkBlitterSizedArena fAlloc;
};

#endif
