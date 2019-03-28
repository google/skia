// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkFontSubsetter_DEFINED
#define SkFontSubsetter_DEFINED

#include "SkTypes.h"

class SkData;
template <typename T> class sk_sp;

using SkFontSubsetter = sk_sp<SkData> (*)(sk_sp<SkData> fontData,
                                          const SkGlyphID* glyphUsage,
                                          int glyphUsageCount,
                                          const char* fontName,
                                          int ttcIndex);

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)
sk_sp<SkData> SkHarfbuzzFontSubset(sk_sp<SkData>, const SkGlyphID*, int, const char*, int);
#endif

#if defined(SK_PDF_USE_SFNTLY)
sk_sp<SkData> SkSfntlyFontSubset(sk_sp<SkData>, const SkGlyphID*, int, const char*, int);
#endif

#endif  // SkFontSubsetter_DEFINED
