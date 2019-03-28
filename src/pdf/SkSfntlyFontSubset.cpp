// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkFontSubsetter.h"

#if defined(SK_PDF_USE_SFNTLY)

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#include "SkData.h"

#include "sample/chromium/font_subsetter.h"
#include <vector>

sk_sp<SkData> SkSfntlyFontSubset(sk_sp<SkData> fontData,
                                 const SkGlyphID* glyphUsage,
                                 int glyphUsageCount,
                                 const char* fontName,
                                 int ttcIndex) {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        return nullptr;
    }
#endif
    // Generate glyph id array in format needed by sfntly.
    std::vector<unsigned> subset(glyphUsageCount);
    for (int i = 0; i < glyphUsageCount; ++i) {
         subset[i] = glyphUsage[i];
    }

    unsigned char* subsetFont{nullptr};
#if defined(SK_BUILD_FOR_GOOGLE3)
    // TODO(halcanary): update SK_BUILD_FOR_GOOGLE3 to newest version of Sfntly.
    (void)ttcIndex;
    int subsetFontSize = SfntlyWrapper::SubsetFont(fontName,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.data(),
                                                   subset.size(),
                                                   &subsetFont);
#else  // defined(SK_BUILD_FOR_GOOGLE3)
    (void)fontName;
    int subsetFontSize = SfntlyWrapper::SubsetFont(ttcIndex,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.data(),
                                                   subset.size(),
                                                   &subsetFont);
#endif  // defined(SK_BUILD_FOR_GOOGLE3)
    SkASSERT(subsetFontSize > 0 || subsetFont == nullptr);
    if (subsetFontSize < 1 || subsetFont == nullptr) {
        return nullptr;
    }
    return SkData::MakeWithProc(subsetFont, subsetFontSize,
                                [](const void* p, void*) { delete[] (unsigned char*)p; },
                                nullptr);
}

#endif  // defined(SK_PDF_USE_SFNTLY)
