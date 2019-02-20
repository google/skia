// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkPDFSubsetFont.h"

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "SkTo.h"

#include "hb.h"
#include "hb-subset.h"

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              const char*,
                              int ttcIndex) {
    if (!fontData) { return nullptr; }

    /* Creating a face */
    hb_blob_t* srcBlob = hb_blob_create((char*)fontData->data(), SkToUInt(fontData->size()),
                                        HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    hb_face_t* face = hb_face_create(srcBlob, ttcIndex);
    hb_blob_destroy(srcBlob);

    /* Add your codepoints here and subset */
    hb_set_t* glyphs = hb_set_create();

    hb_set_add(glyphs, 0);
    glyphUsage.getSetValues([glyphs](unsigned gid) { hb_set_add(glyphs, gid);});

    hb_subset_input_t* input = hb_subset_input_create_or_fail();
    hb_set_t* input_glyphs = hb_subset_input_glyph_set(input);
    hb_set_union(input_glyphs, glyphs);
    hb_subset_input_set_drop_hints(input, true);
    hb_subset_input_set_drop_layout(input, true);
    hb_face_t* subset = hb_subset(face, input);

    /* Clean up */
    hb_set_destroy(glyphs);
    hb_subset_input_destroy(input);

    /* Get result blob */
    hb_blob_t* resultBlob = hb_face_reference_blob(subset);
    hb_face_destroy(subset);
    unsigned int length;
    const char* data = hb_blob_get_data(resultBlob, &length);

    return SkData::MakeWithProc(data, SkToSizeT(length),
                                [](const void*, void* ctx) { hb_blob_destroy((hb_blob_t*)ctx); },
                                resultBlob);
}

#elif defined(SK_PDF_USE_SFNTLY)

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#include "sample/chromium/font_subsetter.h"
#include <vector>

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              const char* fontName,
                              int ttcIndex) {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        return nullptr;
    }
#endif
    // Generate glyph id array in format needed by sfntly.
    // TODO(halcanary): sfntly should take a more compact format.
    std::vector<unsigned> subset;
    if (!glyphUsage.has(0)) {
        subset.push_back(0);  // Always include glyph 0.
    }
    glyphUsage.getSetValues([&subset](unsigned v) { subset.push_back(v); });

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
#else
    (void)fontName;
    int subsetFontSize = SfntlyWrapper::SubsetFont(ttcIndex,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.data(),
                                                   subset.size(),
                                                   &subsetFont);
#endif
    SkASSERT(subsetFontSize > 0 || subsetFont == nullptr);
    if (subsetFontSize < 1 || subsetFont == nullptr) {
        return nullptr;
    }
    return SkData::MakeWithProc(subsetFont, subsetFontSize,
                                [](const void* p, void*) { delete[] (unsigned char*)p; },
                                nullptr);
}

#else

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData>, const SkPDFGlyphUse&, const char*, int) {
    return nullptr;
}
#endif  // defined(SK_PDF_USE_SFNTLY)

