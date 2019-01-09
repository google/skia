// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFSubsetFont_DEFINED
#define SkPDFSubsetFont_DEFINED

#include "SkData.h"
#include "SkPDFGlyphUse.h"

#ifdef SK_PDF_USE_SFNTLY
#define SK_PDF_SUBSET_SUPPORTED

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              const char* fontName,
                              int ttcIndex);
#endif

#endif  // SkPDFSubsetFont_DEFINED
