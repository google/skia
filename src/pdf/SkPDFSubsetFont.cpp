// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFSubsetFont.h"

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/utils/SkCallableTraits.h"

#include "hb.h"  // NO_G3_REWRITE
#include "hb-subset.h"  // NO_G3_REWRITE

using HBBlob = std::unique_ptr<hb_blob_t, SkFunctionObject<hb_blob_destroy>>;
using HBFace = std::unique_ptr<hb_face_t, SkFunctionObject<hb_face_destroy>>;
using HBSubsetInput = std::unique_ptr<hb_subset_input_t, SkFunctionObject<hb_subset_input_destroy>>;
using HBSet = std::unique_ptr<hb_set_t, SkFunctionObject<hb_set_destroy>>;

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

static HBFace make_subset(hb_subset_input_t* input, hb_face_t* face, bool retainZeroGlyph) {
    // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
    // If it isn't known if a font is 'tricky', retain the hints.
    unsigned int flags = HB_SUBSET_FLAGS_RETAIN_GIDS;
    if (retainZeroGlyph) {
        flags |= HB_SUBSET_FLAGS_NOTDEF_OUTLINE;
    }
    hb_subset_input_set_flags(input, flags);
    return HBFace(hb_subset_or_fail(face, input));
}

static sk_sp<SkData> subset_harfbuzz(sk_sp<SkData> fontData,
                                     const SkPDFGlyphUse& glyphUsage,
                                     int ttcIndex) {
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

    HBFace subset = make_subset(input.get(), face.get(), glyphUsage.has(0));
    if (!subset) {
        return nullptr;
    }
    HBBlob result(hb_face_reference_blob(subset.get()));
    return to_data(std::move(result));
}

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData> fontData,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter,
                              int ttcIndex) {
    return subset_harfbuzz(std::move(fontData), glyphUsage, ttcIndex);
}

#else

sk_sp<SkData> SkPDFSubsetFont(sk_sp<SkData>, const SkPDFGlyphUse&, SkPDF::Metadata::Subsetter, int){
    return nullptr;
}
#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)
