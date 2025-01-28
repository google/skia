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
#include "src/core/SkDrawBase.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSurfacePriv.h"

class SkMatrix;
class SkPaint;
class SkPixmap;

class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {}
    SkAutoBlitterChoose(const SkDrawBase& draw,
                        const SkMatrix* ctm,
                        const SkPaint& paint,
                        SkDrawCoverage drawCoverage = SkDrawCoverage::kNo) {
        this->choose(draw, ctm, paint, drawCoverage);
    }

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    SkBlitter* choose(const SkDrawBase& draw,
                      const SkMatrix* ctm,
                      const SkPaint& paint,
                      SkDrawCoverage drawCoverage = SkDrawCoverage::kNo) {
        SkASSERT(!fBlitter);
        fBlitter = draw.fBlitterChooser(draw.fDst,
                                        ctm ? *ctm : *draw.fCTM,
                                        paint,
                                        &fAlloc,
                                        drawCoverage,
                                        draw.fRC->clipShader(),
                                        SkSurfacePropsCopyOrDefault(draw.fProps));
        return fBlitter;
    }

private:
    // Owned by fAlloc, which will handle the delete.
    SkBlitter* fBlitter = nullptr;

    // This was determined experimentally by adding logging to SkSTArenaAlloc's destructor
    // to see what the biggest size observed was while doing some browsing on Chromium.
    // It's a bit tricky to determine this value statically, as the SkRasterPipelineBuilder
    // uses the allocator for several things, as do the shaders which make use of the legacy
    // shader context. In other cases it's easier because the allocator only has the blitter
    // itself and one could do a static_assert using sizeof().
    static constexpr size_t kStackMemory = 2736;
    SkSTArenaAlloc<kStackMemory> fAlloc;
};

#endif
