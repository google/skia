// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFSubsetFont_DEFINED
#define SkPDFSubsetFont_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/docs/SkPDFDocument.h"

class SkData;
class SkPDFGlyphUse;
class SkTypeface;

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface, const SkPDFGlyphUse& glyphUsage);

#endif  // SkPDFSubsetFont_DEFINED
