// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkFontSubsetter.h"

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#if defined(SK_USING_THIRD_PARTY_ICU)
#include "SkLoadICU.h"
#endif

#include "SkData.h"
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

sk_sp<SkData> SkHarfbuzzFontSubset(sk_sp<SkData> fontData,
                                   const SkGlyphID* glyphUsage,
                                   int glyphUsageCount,
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
    for (int i = 0; i < glyphUsageCount; ++i) {
        hb_set_add(glyphs, glyphUsage[i]);
    }

    hb_subset_input_set_retain_gids(input.get(), true);
    hb_subset_input_set_drop_hints(input.get(), true);
    hb_subset_input_set_drop_layout(input.get(), true);
    HBFace subset(hb_subset(face.get(), input.get()));
    HBBlob result(hb_face_reference_blob(subset.get()));
    return to_data(std::move(result));
}

#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)
