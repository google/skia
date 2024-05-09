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

#if HB_VERSION_ATLEAST(10, 0, 0)
unsigned int skhb_get_table_tags_func(const hb_face_t *face,
                                      unsigned int start_offset,
                                      unsigned int *table_count, /* IN/OUT */
                                      hb_tag_t *table_tags /* OUT */,
                                      void *user_data)
{
    SkTypeface& typeface = *reinterpret_cast<SkTypeface*>(user_data);
    int numTables = typeface.countTables();
    if (numTables <= 0 || static_cast<unsigned int>(numTables) <= start_offset) {
        *table_count = 0;
        return 0;
    }
    if (!table_count || *table_count == 0) {
        return numTables;
    }

    std::unique_ptr<SkFontTableTag[]> allTableTags = std::make_unique<SkFontTableTag[]>(numTables);
    int numTableTags = typeface.readTableTags({allTableTags.get(), numTables});
    if (numTableTags != numTables) {
        *table_count = 0;
        return 0;
    }

    static_assert(sizeof(*table_tags) == sizeof(allTableTags[0]), "");
    *table_count = std::min(*table_count, static_cast<unsigned int>(numTableTags) - start_offset);
    memcpy(table_tags, allTableTags.get() + start_offset, *table_count * sizeof(*table_tags));
    return numTableTags;
}
#endif

hb_blob_t* skhb_get_table(hb_face_t* face, hb_tag_t tag, void* user_data) {
    SkTypeface& typeface = *reinterpret_cast<SkTypeface*>(user_data);

    auto data = typeface.copyTableData(tag);
    if (!data) {
        return nullptr;
    }
    SkData* rawData = data.release();
    return hb_blob_create(reinterpret_cast<char*>(rawData->writable_data()), rawData->size(),
                          HB_MEMORY_MODE_READONLY, rawData,
                          [](void* p) { SkSafeUnref(((SkData*)p)); });
}

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

HBFace stream_to_face(std::unique_ptr<SkStreamAsset> asset, int index) {
    HBFace face;
    HBBlob blob(stream_to_blob(std::move(asset)));
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
    return face;
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
    HBFace face;
    if (!SkPDFCanSubsetTableBasedFonts()) {
        int index = 0;
        std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openStream(&index);
        if (typefaceAsset) {
            face = stream_to_face(std::move(typefaceAsset), index);
        }
    } else {
        int index = 0;
        std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openExistingStream(&index);
        if (typefaceAsset && typefaceAsset->getMemoryBase()) {
            face = stream_to_face(std::move(typefaceAsset), index);
        }
        if (!face) {
            // Use hb_face_create_for_tables to try creating a working hb_face.
            // Using this creates empty subsets in HarfBuzz until 4.4.0.
            face.reset(hb_face_create_for_tables(
                skhb_get_table,
                const_cast<SkTypeface*>(SkRef(&typeface)),
                [](void* user_data){ SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data)); }));
            hb_face_set_index(face.get(), (unsigned)index);
#if HB_VERSION_ATLEAST(10, 0, 0)
            hb_face_set_get_table_tags_func(
                face.get(), skhb_get_table_tags_func,
                const_cast<SkTypeface*>(SkRef(&typeface)),
                [](void* user_data){ SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data)); });
#endif
            // Check the number of glyphs as a basic sanitization step.
            if (face && hb_face_get_glyph_count(face.get()) == 0) {
                face.reset();
            }
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

bool SkPDFCanSubsetTableBasedFonts() {
    // See https://github.com/harfbuzz/harfbuzz/issues/3609
    // See https://github.com/harfbuzz/harfbuzz/issues/1697
    // This file requires HarfBuzz 3.0 or later (when the public subsetter API shipped).
    // The default tables to search for subsetting were added in HarfBuzz 4.4.0.
    // The full fix (hb_face_set_get_table_tags_func) is in HarfBuzz 10.0.0.
    return hb_version_atleast(4, 4, 0);
}

#else

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface&, const SkPDFGlyphUse&) {
    return nullptr;
}

bool SkPDFCanSubsetTableBasedFonts() {
    return false;
}

#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)
