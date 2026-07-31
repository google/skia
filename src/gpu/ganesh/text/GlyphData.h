/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_GlyphData_DEFINED
#define skgpu_ganesh_GlyphData_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "src/core/SkColorData.h"
#include "src/core/SkGlyph.h"
#include "src/gpu/ganesh/GrAtlasTypes.h"
#include "src/gpu/ganesh/text/TextStrike.h"
#include "src/text/gpu/PackedGPUGlyphID.h"

#include <cstdint>
#include <tuple>

class GrAtlasManager;
class GrMeshDrawTarget;
class SkMatrix;

namespace sktext::gpu {
class GlyphVector;
class StrikeCache;
class VertexFiller;
}

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::ganesh {

class TextStrike;

/**
 * Ganesh-specific glyph type with atlas location information.
 */
struct GlyphEntry {
    explicit GlyphEntry(sktext::gpu::PackedGPUGlyphID key) : fKey(key) {}

    const sktext::gpu::PackedGPUGlyphID fKey;
    GrAtlasLocator fAtlasLocator;
};

/** Adapts GlyphEntry* to conform to GlyphVectors GlyphType requirements. */
class Glyph {
    GlyphEntry* fEntry;

    Glyph() = delete;
    Glyph(const Glyph&) = delete;
    Glyph(Glyph&&) = delete;
    Glyph& operator=(const Glyph&) = delete;
    Glyph& operator=(Glyph&&) = delete;

public:
    explicit Glyph(GlyphEntry* entry) : fEntry{entry} { SkASSERT(entry); }
    SkPackedGlyphID packedID() const { return fEntry->fKey.packedGlyphID(); }
    GlyphEntry& entry() const { return *fEntry; }
};

/**
 * Stored on SubRun. It maintains mapping from SubRun's packed glyph IDs to atlas locations,
 * coordinating updates to this data as the data in the atlas evolves.
 */
class GlyphData final {
public:
    static sk_sp<TextStrike> FindStrike(sktext::gpu::StrikeCache* cache, const SkStrikeSpec& spec) {
        return TextStrike::GetOrCreate(cache, spec);
    }

    // These are also the parameters that must be passed to GlyphVector::initBackendData<GlyphData>
    // after the StrikeCache (sans TextStrike, which is derived from the StrikeCache).
    GlyphData(sk_sp<TextStrike>,
              GrAtlasManager* atlasMgr,
              MaskFormat maskFormat,
              int srcPadding,
              bool isSDF);

    ~GlyphData();

    Glyph makeGlyphFromID(SkPackedGlyphID);

    // Regenerate atlas entries for glyphs in range [begin, end).
    // Returns {success, glyphs_placed_in_atlas}.
    std::tuple<bool, int> regenerateAtlas(int begin,
                                          int end,
                                          sktext::gpu::GlyphVector& glyphVector,
                                          GrMeshDrawTarget* target);

    size_t vertexStride(MaskFormat, const SkMatrix& positionMatrix) const;

    void fillVertexData(const sktext::gpu::VertexFiller& vf,
                        SkSpan<const Glyph> glyphs,
                        int offset,
                        int count,
                        const SkPMColor4f& pmColor,
                        const SkMatrix& positionMatrix,
                        SkIRect clip,
                        void* vertexBuffer);

private:
    GlyphData(const GlyphData&) = delete;
    GlyphData(GlyphData&&) = delete;
    GlyphData& operator=(const GlyphData&) = delete;
    GlyphData& operator=(GlyphData&&) = delete;

    sk_sp<TextStrike> fTextStrike{nullptr};
    uint64_t fAtlasGeneration{GrAtlasGenerationCounter::kInvalidGeneration};
    GrBulkUsePlotUpdater fBulkUseUpdater;

    MaskFormat fResolvedMaskFormat;
    int fSrcPadding;
    bool fIsSDF;
};

}  // namespace skgpu::ganesh

#endif  // skgpu_ganesh_GlyphData_DEFINED
