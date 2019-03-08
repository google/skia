// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPenPathEffect_DEFINED
#define SkPenPathEffect_DEFINED

#include "SkPathEffect.h"

/** return a path effect that strokes with an elipse specified by a matrix
   [[a,b],[c,d]]. `SkMakePenPathEffect(W, 0, 0, W)` strokes with a width of
   `W`.  Ignores SkPaint StrokeWidth parameter.  Has no effect if SkPaint
   style is FillStyle.
*/
sk_sp<SkPathEffect> SkMakePenPathEffect(SkScalar a, SkScalar b, SkScalar c, SkScalar d);

#endif  // SkPenPathEffect_DEFINED
