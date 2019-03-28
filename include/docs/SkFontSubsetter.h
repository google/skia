// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkFontSubsetter_DEFINED
#define SkFontSubsetter_DEFINED

#include "SkTypes.h"

class SkData;
template <typename T> class sk_sp;

class SkGlyphSet {
public:
    virtual ~SkGlyphSet() {}
    virtual void getSetValues(void* context, void (*callback)(void* context, SkGlyphID)) const = 0;
};

using SkFontSubsetter = sk_sp<SkData> (*)(sk_sp<SkData> fontData,
                                          const SkGlyphSet& glyphUsage,
                                          const char* fontName,
                                          int ttcIndex);

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)
SK_API sk_sp<SkData> SkHarfbuzzFontSubset(sk_sp<SkData>, const SkGlyphSet&, const char*, int);
#endif

#if defined(SK_PDF_USE_SFNTLY)
SK_API sk_sp<SkData> SkSfntlyFontSubset(sk_sp<SkData>, const SkGlyphSet&, const char*, int);
#endif

#endif  // SkFontSubsetter_DEFINED
