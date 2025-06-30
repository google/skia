/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathUtils_DEFINED
#define SkPathUtils_DEFINED

#include "include/core/SkScalar.h"  // IWYU pragma: keep
#include "include/core/SkTypes.h"

class SkMatrix;
class SkPaint;
class SkPath;
class SkPathBuilder;
struct SkRect;

namespace skpathutils {

/* Returns the filled equivalent of the stroked path.
 *
 *  @param src       SkPath read to create a filled version
 *  @param paint     uses settings for stroke cap, width, miter, join, and patheffect.
 *  @param dst       results are written to this builder.
 *  @param cullRect  optional limit passed to SkPathEffect
 *  @param ctm       matrix to take into acount for increased precision (if it scales up).
 *  @return          true if the result can be filled, or false if it is a hairline (to be stroked).
 */
SK_API bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPathBuilder* dst,
                              const SkRect* cullRect, const SkMatrix& ctm);

SK_API bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPathBuilder* dst);

SK_API SkPath FillPathWithPaint(const SkPath& src, const SkPaint& paint, bool* isFill = nullptr);

#ifdef SK_SUPPORT_MUTABLE_PATHEFFECT
SK_API bool FillPathWithPaint(const SkPath &src, const SkPaint &paint, SkPath *dst,
                              const SkRect *cullRect, SkScalar resScale = 1);

SK_API bool FillPathWithPaint(const SkPath &src, const SkPaint &paint, SkPath *dst,
                              const SkRect *cullRect, const SkMatrix &ctm);

SK_API bool FillPathWithPaint(const SkPath &src, const SkPaint &paint, SkPath *dst);
#endif

}

#endif
