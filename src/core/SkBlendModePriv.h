/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModePriv_DEFINED
#define SkBlendModePriv_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "src/core/SkColorData.h"

class SkRasterPipeline;
class SkPaint;

/**
 *  Sentinel value for SkBlendMode enum.
 *
 *  Will never be a valid enum value, but will be storable in a byte.
 */
constexpr uint8_t kCustom_SkBlendMode = 0xFF;

bool SkBlendMode_SupportsCoverageAsAlpha(SkBlendMode);

static inline bool SkBlendMode_CaresAboutRBOrder(SkBlendMode mode) {
    return (mode > SkBlendMode::kLastSeparableMode);
}

bool SkBlendMode_ShouldPreScaleCoverage(SkBlendMode, bool rgb_coverage);
void SkBlendMode_AppendStages(SkBlendMode, SkRasterPipeline*);

SkPMColor4f SkBlendMode_Apply(SkBlendMode, const SkPMColor4f& src, const SkPMColor4f& dst);

enum class SkBlendFastPath {
    kNormal,      // draw normally
    kSrcOver,     //< draw as if in srcover mode
    kSkipDrawing  //< draw nothing
};

/**
 *  Given a paint, determine whether the paint's blend mode can be
 *  replaced with kSrcOver or not drawn at all. This can inform drawing optimizations.
 */
SkBlendFastPath CheckFastPath(const SkPaint&, bool dstIsOpaque);

#endif
