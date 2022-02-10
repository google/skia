/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGlyphVector_DEFINED
#define GrGlyphVector_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/gpu/GrGlyph.h"
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrSubRunAllocator.h"
#include "src/gpu/text/GrStrikeCache.h"

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of GrGlyphs until the code is running on the
// GPU in single threaded mode. The GlyphVector is created in a multi-threaded environment, but
// the GrStrikeCache is only single threaded (and must be single threaded because of the atlas).
class GrGlyphVector {
public:
    union Variant {
        // Initially, filled with packed id, but changed to GrGlyph* in the onPrepare stage.
        SkPackedGlyphID packedGlyphID;
        GrGlyph* grGlyph;
        // Add ctors to help SkArenaAlloc create arrays.
        Variant() : grGlyph{nullptr} {}
        Variant(SkPackedGlyphID id) : packedGlyphID{id} {}
    };

    GrGlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs);

    static GrGlyphVector Make(
            sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc);
    SkSpan<const GrGlyph*> glyphs() const;

    void packedGlyphIDToGrGlyph(GrStrikeCache* cache);

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            GrMaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawTarget *,
            bool bilerpPadding = false);

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    sk_sp<SkStrike> fStrike;
    SkSpan<Variant> fGlyphs;
    sk_sp<GrTextStrike> fGrStrike{nullptr};
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
};
#endif  // GrGlyphVector_DEFINED
