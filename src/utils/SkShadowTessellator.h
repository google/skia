/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowTessellator_DEFINED
#define SkShadowTessellator_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkRefCnt.h"

class SkMatrix;
class SkPath;
class SkVertices;

namespace SkShadowTessellator {
/**
 * This function generates an ambient shadow mesh for a path by walking the path, outsetting by
 * the radius, and setting inner and outer colors to umbraColor and penumbraColor, respectively.
 * If transparent is true, then the center of the ambient shadow will be filled in.
 */
sk_sp<SkVertices> MakeAmbient(const SkPath& path, const SkMatrix& ctm, SkScalar radius,
                              SkColor umbraColor, SkColor penumbraColor, bool transparent);

/**
 * This function generates a spot shadow mesh for a path by walking the transformed path,
 * further transforming by the scale and translation, and outsetting and insetting by a radius.
 * The center will be clipped against the original path unless transparent is true.
 */
sk_sp<SkVertices> MakeSpot(const SkPath& path, const SkMatrix& ctm, SkScalar scale,
                           const SkVector& translate, SkScalar radius, SkColor umbraColor,
                           SkColor penumbraColor, bool transparent);
}

#endif
