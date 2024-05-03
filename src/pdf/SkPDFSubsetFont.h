// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFSubsetFont_DEFINED
#define SkPDFSubsetFont_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/docs/SkPDFDocument.h"

class SkData;
class SkPDFGlyphUse;

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter subsetter,
                              int ttcIndex);

#endif  // SkPDFSubsetFont_DEFINED
