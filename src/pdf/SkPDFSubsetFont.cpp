// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFSubsetFont.h"

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/pdf/SkPDFGlyphUse.h"

#include "hb.h"  // NO_G3_REWRITE
#include "hb-subset.h"  // NO_G3_REWRITE

#include <cstddef>
#include <memory>
#include <utility>

namespace {

using HBBlob = std::unique_ptr<hb_blob_t, SkFunctionObject<hb_blob_destroy>>;
using HBFace = std::unique_ptr<hb_face_t, SkFunctionObject<hb_face_destroy>>;
using HBSubsetInput = std::unique_ptr<hb_subset_input_t, SkFunctionObject<hb_subset_input_destroy>>;
using HBSet = std::unique_ptr<hb_set_t, SkFunctionObject<hb_set_destroy>>;

HBBlob stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    HBBlob blob;
    if (const void* base = asset->getMemoryBase()) {
        blob.reset(hb_blob_create(const_cast<char*>(static_cast<const char*>(base)), SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, asset.release(),
                                  [](void* p) { delete (SkStreamAsset*)p; }));
    } else {
        void* ptr = size ? sk_malloc_throw(size) : nullptr;
        asset->read(ptr, size);
        blob.reset(hb_blob_create((char*)ptr, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, ptr, sk_free));
    }
    SkASSERT(blob);
    hb_blob_make_immutable(blob.get());
    return blob;
}

sk_sp<SkData> to_data(HBBlob blob) {
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

HBFace make_subset(hb_subset_input_t* input, hb_face_t* face, bool retainZeroGlyph) {
    // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
    // If it isn't known if a font is 'tricky', retain the hints.
    unsigned int flags = HB_SUBSET_FLAGS_RETAIN_GIDS;
    if (retainZeroGlyph) {
        flags |= HB_SUBSET_FLAGS_NOTDEF_OUTLINE;
    }
    hb_subset_input_set_flags(input, flags);
    return HBFace(hb_subset_or_fail(face, input));
}

sk_sp<SkData> subset_harfbuzz(const SkTypeface& typeface, const SkPDFGlyphUse& glyphUsage) {
    int index = 0;
    std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openStream(&index);
    HBFace face;
    HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
    // hb_face_create always succeeds. Check that the format is minimally recognized first.
    // See https://github.com/harfbuzz/harfbuzz/issues/248
    unsigned int num_hb_faces = hb_face_count(blob.get());
    if (0 < num_hb_faces && (unsigned)index < num_hb_faces) {
        face.reset(hb_face_create(blob.get(), (unsigned)index));
        // Check the number of glyphs as a basic sanitization step.
        if (face && hb_face_get_glyph_count(face.get()) == 0) {
            face.reset();
        }
    }

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

}  // namespace

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface, const SkPDFGlyphUse& glyphUsage) {
    return subset_harfbuzz(typeface, glyphUsage);
}

#else

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface&, const SkPDFGlyphUse&) {
    return nullptr;
}

#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)
