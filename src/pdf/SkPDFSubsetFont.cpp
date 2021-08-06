// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFSubsetFont.h"

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/utils/SkCallableTraits.h"

#include "hb.h"
#include "hb-subset.h"

template <class T, void(*P)(T*)> using resource =
    std::unique_ptr<T, SkFunctionWrapper<std::remove_pointer_t<decltype(P)>, P>>;
using HBBlob = resource<hb_blob_t, &hb_blob_destroy>;
using HBFace = resource<hb_face_t, &hb_face_destroy>;
using HBSubsetInput = resource<hb_subset_input_t, &hb_subset_input_destroy>;
using HBSet = resource<hb_set_t, &hb_set_destroy>;

static HBBlob to_blob(sk_sp<SkData> data) {
    using blob_size_t = SkCallableTraits<decltype(hb_blob_create)>::argument<1>::type;
    if (!SkTFitsIn<blob_size_t>(data->size())) {
        return nullptr;
    }
    const char* blobData = static_cast<const char*>(data->data());
    blob_size_t blobSize = SkTo<blob_size_t>(data->size());
    return HBBlob(hb_blob_create(blobData, blobSize,
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

template<typename...> using void_t = void;
template<typename T, typename = void>
struct SkPDFHarfBuzzSubset {
    // This is the HarfBuzz 3.0 interface.
    // hb_subset_flags_t does not exist in 2.0. It isn't dependent on T, so inline the value of
    // HB_SUBSET_FLAGS_RETAIN_GIDS until 2.0 is no longer supported.
    static HBFace Make(T input, hb_face_t* face) {
        // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
        // If it isn't known if a font is 'tricky', retain the hints.
        hb_subset_input_set_flags(input, 2/*HB_SUBSET_FLAGS_RETAIN_GIDS*/);
        return HBFace(hb_subset_or_fail(face, input));
    }
};
template<typename T>
struct SkPDFHarfBuzzSubset<T, void_t<
    decltype(hb_subset_input_set_retain_gids(std::declval<T>(), std::declval<bool>())),
    decltype(hb_subset_input_set_drop_hints(std::declval<T>(), std::declval<bool>())),
    decltype(hb_subset(std::declval<hb_face_t*>(), std::declval<T>()))
    >>
{
    // This is the HarfBuzz 2.0 (non-public) interface, used if it exists.
    // This code should be removed as soon as all users are migrated to the newer API.
    static HBFace Make(T input, hb_face_t* face) {
        hb_subset_input_set_retain_gids(input, true);
        // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
        // If it isn't known if a font is 'tricky', retain the hints.
        hb_subset_input_set_drop_hints(input, false);
        return HBFace(hb_subset(face, input));
    }
};

static sk_sp<SkData> subset_harfbuzz(sk_sp<SkData> fontData,
                                     const SkPDFGlyphUse& glyphUsage,
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
    glyphUsage.getSetValues([&glyphs](unsigned gid) { hb_set_add(glyphs, gid);});

    HBFace subset = SkPDFHarfBuzzSubset<hb_subset_input_t*>::Make(input.get(), face.get());
    if (!subset) {
        return nullptr;
    }
    HBBlob result(hb_face_reference_blob(subset.get()));
    return to_data(std::move(result));
}

#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_PDF_USE_SFNTLY)

#include "sample/chromium/font_subsetter.h"
#include <vector>

static sk_sp<SkData> subset_sfntly(sk_sp<SkData> fontData,
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

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_PDF_USE_SFNTLY) && defined(SK_PDF_USE_HARFBUZZ_SUBSET)

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter subsetter,
                              const char* fontName,
                              int ttcIndex) {
    switch (subsetter) {
        case SkPDF::Metadata::kHarfbuzz_Subsetter:
            return subset_harfbuzz(std::move(fontData), glyphUsage, ttcIndex);
        case SkPDF::Metadata::kSfntly_Subsetter:
            return subset_sfntly(std::move(fontData), glyphUsage, fontName, ttcIndex);
    }
    return nullptr;
}

#elif defined(SK_PDF_USE_SFNTLY)

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter,
                              const char* fontName,
                              int ttcIndex) {
    return subset_sfntly(std::move(fontData), glyphUsage, fontName, ttcIndex);
}

#elif defined(SK_PDF_USE_HARFBUZZ_SUBSET)

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter,
                              const char*,
                              int ttcIndex) {
    return subset_harfbuzz(std::move(fontData), glyphUsage, ttcIndex);
}

#else

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData>, const SkPDFGlyphUse&, SkPDF::Metadata::Subsetter,
                              const char*, int) {
    return nullptr;
}
#endif  // defined(SK_PDF_USE_SFNTLY)
