/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDashOp_DEFINED
#define GrDashOp_DEFINED

#include "GrColor.h"
#include "GrTypesPriv.h"
#include "SkPathEffect.h"

class GrLegacyMeshDrawOp;
class GrStyle;

namespace GrDashOp {
enum class AAMode {
    kNone,
    kCoverage,
    kCoverageWithMSAA,
};
static const int kAAModeCnt = static_cast<int>(AAMode::kCoverageWithMSAA) + 1;

std::unique_ptr<GrLegacyMeshDrawOp> MakeDashLineOp(GrColor, const SkMatrix& viewMatrix,
                                                   const SkPoint pts[2], AAMode,
                                                   const GrStyle& style);
bool CanDrawDashLine(const SkPoint pts[2], const GrStyle& style, const SkMatrix& viewMatrix);
}

#endif
