/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowTessellator_DEFINED
#define SkShadowTessellator_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include <functional>  // std::function

class SkMatrix;
class SkPath;
struct SkPoint3;
class SkVertices;

namespace SkShadowTessellator {

typedef std::function<SkScalar(SkScalar, SkScalar)> HeightFunc;

/**
 * This function generates an ambient shadow mesh for a path by walking the path, outsetting by
 * the radius, and setting inner and outer colors to umbraColor and penumbraColor, respectively.
 * If transparent is true, then the center of the ambient shadow will be filled in.
 */
sk_sp<SkVertices> MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                              const SkPoint3& zPlane, bool transparent);

/**
 * This function generates a spot shadow mesh for a path by walking the transformed path,
 * further transforming by the scale and translation, and outsetting and insetting by a radius.
 * The center will be clipped against the original path unless transparent is true.
 */
sk_sp<SkVertices> MakeSpot(const SkPath& path, const SkMatrix& ctm, const SkPoint3& zPlane,
                           const SkPoint3& lightPos, SkScalar lightRadius, bool transparent,
                           bool directional);


}  // namespace SkShadowTessellator

#endif
