// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkPDFSubsetFont.h"

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "SkTo.h"
#include "SkTemplates.h"

#include "hb.h"
#include "hb-subset.h"

template <class T, void(*P)(T*)> using resource = std::unique_ptr<T, SkFunctionWrapper<void, T, P>>;
using HBBlob = resource<hb_blob_t, hb_blob_destroy>;
using HBFace = resource<hb_face_t, hb_face_destroy>;
using HBSubsetInput = resource<hb_subset_input_t, hb_subset_input_destroy>;
using HBSet = resource<hb_set_t, hb_set_destroy>;

static HBBlob to_blob(sk_sp<SkData> data) {
    return HBBlob(hb_blob_create((char*)data->data(), SkToUInt(data->size()),
                                 HB_MEMORY_MODE_READONLY,
                                 data.release(), [](void* p){ ((SkData*)p)->unref(); }));
}

static sk_sp<SkData> to_data(HBBlob blob) {
    if (!blob) {
        return nullptr;
    }
    unsigned int length;
    const char* data = hb_blob_get_data(blob.get(), &length);
    if (!data || !length) {
        return nullptr;
    }
    return SkData::MakeWithProc(data, SkToSizeT(length),
                                [](const void*, void* ctx) { hb_blob_destroy((hb_blob_t*)ctx); },
                                blob.release());
}

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              const char*,
                              int ttcIndex) {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        return nullptr;
    }
#endif
    if (!fontData) {
        return nullptr;
    }
    HBFace face(hb_face_create(to_blob(std::move(fontData)).get(), ttcIndex));
    SkASSERT(face);

    HBSubsetInput input(hb_subset_input_create_or_fail());
    SkASSERT(input);
    if (!face || !input) {
        return nullptr;
    }
    hb_set_t* glyphs = hb_subset_input_glyph_set(input.get());
    hb_set_add(glyphs, 0);
    glyphUsage.getSetValues([&glyphs](unsigned gid) { hb_set_add(glyphs, gid);});

    hb_subset_input_set_retain_gids(input.get(), true);
    hb_subset_input_set_drop_hints(input.get(), true);
    hb_subset_input_set_drop_layout(input.get(), true);
    HBFace subset(hb_subset(face.get(), input.get()));
    HBBlob result(hb_face_reference_blob(subset.get()));
    return to_data(std::move(result));
}

#elif defined(SK_PDF_USE_SFNTLY)

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

