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
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrSubRunAllocator.h"
#include "src/gpu/ganesh/text/GrStrikeCache.h"
#include "src/text/gpu/Glyph.h"

class SkStrikeClient;

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of sktext::gpu::Glyphs until the code is running
// on the GPU in single threaded mode. The GlyphVector is created in a multi-threaded environment,
// but the GrStrikeCache is only single threaded (and must be single threaded because of the atlas).
class GrGlyphVector {
public:
    union Variant {
        // Initially, filled with packed id, but changed to Glyph* in the onPrepare stage.
        SkPackedGlyphID packedGlyphID;
        sktext::gpu::Glyph* glyph;
        // Add ctors to help SkArenaAlloc create arrays.
        Variant() : glyph{nullptr} {}
        Variant(SkPackedGlyphID id) : packedGlyphID{id} {}
    };

    GrGlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs);

    static GrGlyphVector Make(
            sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc);
    SkSpan<const sktext::gpu::Glyph*> glyphs() const;

    static std::optional<GrGlyphVector> MakeFromBuffer(SkReadBuffer& buffer,
                                                       const SkStrikeClient* strikeClient,
                                                       GrSubRunAllocator* alloc);
    void flatten(SkWriteBuffer& buffer);

    // This doesn't need to include sizeof(GrGlyphVector) because this is embedded in each of
    // the sub runs.
    int unflattenSize() const { return GlyphVectorSize(fGlyphs.size()); }

    void packedGlyphIDToGlyph(GrStrikeCache* cache);

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            skgpu::MaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawTarget *);

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    friend class TestingPeer;
    sk_sp<SkStrike> fStrike;
    SkSpan<Variant> fGlyphs;
    sk_sp<GrTextStrike> fGrStrike{nullptr};
    uint64_t fAtlasGeneration{skgpu::AtlasGenerationCounter::kInvalidGeneration};
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
};
#endif  // GrGlyphVector_DEFINED
