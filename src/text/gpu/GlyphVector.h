/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_GlyphVector_DEFINED
#define sktext_gpu_GlyphVector_DEFINED

#include <variant>
#include "include/core/SkSpan.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkStrikeForGPU.h"
#include "src/gpu/AtlasTypes.h"
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
// -- StrikeRef ------------------------------------------------------------------------------------
// Hold a ref to either a RemoteStrike or an SkStrike. Use either to flatten a descriptor, but
// when MakeFromBuffer runs look up the SkStrike associated with the descriptor.
class StrikeRef {
public:
    StrikeRef() = delete;
    StrikeRef(sk_sp<SkStrike>&& strike);
    StrikeRef(StrikeForGPU* strike);
    StrikeRef(const StrikeRef&) = delete;
    const StrikeRef& operator=(const StrikeRef&) = delete;
    StrikeRef(StrikeRef&&);
    StrikeRef& operator=(StrikeRef&&);

    // Flatten a descriptor into the buffer.
    void flatten(SkWriteBuffer& buffer) const;

    // Unflatten a descriptor, and create a StrikeRef holding an sk_sp<SkStrike>. The client is
    // used to do SkTypeFace id translation if passed in.
    static std::optional<StrikeRef> MakeFromBuffer(SkReadBuffer& buffer,
                                                   const SkStrikeClient* client);

    // getStrikeAndSetToNullptr can only be used when holding an SkStrike. This will only return
    // the SkStrike the first time, and will return nullptr on all future calls. Once this is
    // called, flatten can not be called.
    sk_sp<SkStrike> getStrikeAndSetToNullptr();

private:
    friend class StrikeRefTestingPeer;
    // A StrikeRef can hold a pointer from a RemoteStrike which is of type SkStrikeForGPU,
    // or it can hold an actual ref to an actual SkStrike.
    std::variant<std::monostate, StrikeForGPU*, sk_sp<SkStrike>> fStrike;
};

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

    GlyphVector(StrikeRef&& strikeRef, SkSpan<Variant> glyphs);

    static GlyphVector Make(
            sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc);
    static GlyphVector Make(
            StrikeForGPU* strike, SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc);

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
    static Variant* MakeGlyphs(SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc);

    StrikeRef fStrikeRef;
    SkSpan<Variant> fGlyphs;
    sk_sp<TextStrike> fTextStrike{nullptr};
    uint64_t fAtlasGeneration{skgpu::AtlasGenerationCounter::kInvalidGeneration};
    skgpu::BulkUsePlotUpdater fBulkUseUpdater;
};
}  // namespace sktext::gpu
#endif  // sktext_gpu_GlyphVector_DEFINED
