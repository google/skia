/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAnalyticRectOp_DEFINED
#define GrAnalyticRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrLegacyMeshDrawOp;
class SkMatrix;
struct SkRect;

/*
 * This class wraps helper functions that draw rects analytically. Used when a shader requires a
 * distance vector.
 *
 * @param color        the shape's color
 * @param viewMatrix   the shape's local matrix
 * @param rect         the shape in source space
 * @param croppedRect  the shape in device space, clipped to the device's bounds
 * @param bounds       the axis aligned bounds of the shape in device space
 */
class GrAnalyticRectOp {
public:
    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                                    const SkMatrix& viewMatrix,
                                                    const SkRect& rect,
                                                    const SkRect& croppedRect,
                                                    const SkRect& bounds);
};

#endif  // GrAnalyticRectOp_DEFINED
