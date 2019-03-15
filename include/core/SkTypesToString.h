// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkTypesToString_DEFINED
#define SkTypesToString_DEFINED

#include "SkFilterQuality.h"
#include "SkImageInfo.h"
#include "SkPaint.h"

SK_API const char* SkColorTypeToString(SkColorType v);
SK_API const char* SkAlphaTypeToString(SkAlphaType v);
SK_API const char* SkPaintCapToString(SkPaint::Cap v);
SK_API const char* SkPaintJoinToString(SkPaint::Join v);
SK_API const char* SkPaintStyleToString(SkPaint::Style v);
SK_API const char* SkFilterQualityToString(SkFilterQuality v);

#endif  // SkTypesToString_DEFINED
