/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_GlyphVector_DEFINED
#define sktext_gpu_GlyphVector_DEFINED
#include "include/core/SkSpan.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/SubRunAllocator.h"

class SkStrikeClient;
#if SK_SUPPORT_GPU
class GrMeshDrawTarget;
#endif
#if defined(SK_GRAPHITE_ENABLED)
namespace skgpu::graphite { class Recorder; }
#endif

namespace sktext::gpu {

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of Glyphs until the code is running on the GPU
// in single threaded mode. The GlyphVector is created in a multi-threaded environment, but the
// StrikeCache is only single threaded (and must be single threaded because of the atlas).
class GlyphVector {
public:
    union Variant {
        // Initially, filled with packed id, but changed to Glyph* in the onPrepare stage.
        SkPackedGlyphID packedGlyphID;
        Glyph* glyph;
        // Add ctors to help SkArenaAlloc create arrays.
        Variant() : glyph{nullptr} {}
        Variant(SkPackedGlyphID id) : packedGlyphID{id} {}
    };

    GlyphVector(SkStrikePromise&& strikePromise, SkSpan<Variant> glyphs);

    static GlyphVector Make(
            SkStrikePromise&& promise, SkSpan<SkPackedGlyphID> glyphs, SubRunAllocator* alloc);

    SkSpan<const Glyph*> glyphs() const;

    static std::optional<GlyphVector> MakeFromBuffer(SkReadBuffer& buffer,
                                                     const SkStrikeClient* strikeClient,
                                                     SubRunAllocator* alloc);
    void flatten(SkWriteBuffer& buffer) const;

    // This doesn't need to include sizeof(GlyphVector) because this is embedded in each of
    // the sub runs.
    int unflattenSize() const { return GlyphVectorSize(fGlyphs.size()); }

    void packedGlyphIDToGlyph(StrikeCache* cache);

#if SK_SUPPORT_GPU
    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            skgpu::MaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawTarget*);
#endif

#if defined(SK_GRAPHITE_ENABLED)
    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            skgpu::MaskFormat maskFormat,
            int srcPadding,
            skgpu::graphite::Recorder*);
#endif

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    friend class GlyphVectorTestingPeer;
    static Variant* MakeGlyphs(SkSpan<SkPackedGlyphID> glyphs, SubRunAllocator* alloc);

    SkStrikePromise fStrikePromise;
    SkSpan<Variant> fGlyphs;
    sk_sp<TextStrike> fTextStrike{nullptr};
    uint64_t fAtlasGeneration{skgpu::AtlasGenerationCounter::kInvalidGeneration};
    skgpu::BulkUsePlotUpdater fBulkUseUpdater;
};
}  // namespace sktext::gpu
#endif  // sktext_gpu_GlyphVector_DEFINED
