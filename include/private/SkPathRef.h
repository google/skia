/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRef_DEFINED
#define SkPathRef_DEFINED

#include "include/core/SkArc.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h" // IWYU pragma: keep
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>

class SkMatrix;

/*
 *  These "info" structs are used to return identifying information, when a path
 *  is queried if it is a special "shape". (e.g. isOval(), isRRect())
 */

struct SkPathRectInfo {
    SkRect          fRect;
    SkPathDirection fDirection;
    uint8_t         fStartIndex;
};

struct SkPathOvalInfo {
    SkRect          fBounds;
    SkPathDirection fDirection;
    uint8_t         fStartIndex;
};

struct SkPathRRectInfo {
    SkRRect         fRRect;
    SkPathDirection fDirection;
    uint8_t         fStartIndex;
};

/*
 *  Paths can be tagged with a "Type" -- the IsAType
 *  This signals that it was built from a high-level shape: oval, rrect, arc, wedge.
 *  We try to retain this tag, but still build the explicitly line/quad/conic/cubic
 *  structure need to represent that shape. Thus a user of path can always just look
 *  at the points and verbs, and draw it correctly.
 *
 *  The GPU backend sometimes will sniff the path for this tag/type, and may have a
 *  more optimal way to draw the shape if they know its "really" an oval or whatever.
 *
 *  Path's can also identify as a "rect" -- but we don't store any special tag for this.
 *
 *  Here are the special "types" we have APIs for (e.g. isRRect()) and what we store:
 *
 *      kGeneral  : no identifying shape, no extra data
 *      (Rect)    : no tag, but isRect() will examing the points/verbs, and try to
 *                  deduce that it represents a rect.
 *      kOval     : the path bounds is also the oval's bounds -- we store the direction
 *                  and start_index (important for dashing). see SkPathMakers.h
 *      kRRect    : same as kOval for implicit bounds, direction and start_index.
 *                  Note: we don't store its radii -- we deduce those when isRRect() is
 *                        called, by examining the points/verbs.
 */
enum class SkPathIsAType : uint8_t {
    kGeneral,
    kOval,
    kRRect,
};

struct SkPathIsAData {
    uint8_t         fStartIndex;
    SkPathDirection fDirection;
};

#endif
