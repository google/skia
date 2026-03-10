/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GlyphData_DEFINED
#define skgpu_graphite_GlyphData_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkGlyph.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/text/TextStrike.h"

#include <cstdint>
#include <tuple>

namespace sktext::gpu {
class GlyphVector;
class StrikeCache;
class VertexFiller;
}

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::graphite {
class DrawWriter;
class Recorder;
class TextStrike;

struct GlyphEntryKey {
    explicit GlyphEntryKey(SkPackedGlyphID id, MaskFormat format) : fPackedID(id), fFormat(format) {}

    const SkPackedGlyphID fPackedID;
    MaskFormat fFormat;

    bool operator==(const GlyphEntryKey& that) const {
        return fPackedID == that.fPackedID && fFormat == that.fFormat;
    }
    bool operator!=(const GlyphEntryKey& that) const {
        return !(*this == that);
    }

    uint32_t hash() const {
        return fPackedID.hash();
    }
};

/**
 * Graphite-specific glyph type with atlas location information.
 */
struct GlyphEntry {
    explicit GlyphEntry(SkPackedGlyphID id, MaskFormat format) : fGlyphEntryKey(id, format) {}

    const GlyphEntryKey fGlyphEntryKey;
    DrawAtlas::AtlasLocator fAtlasLocator;
};

/** Adapts GlyphEntry* to conform to GlyphVector's GlyphType requirements. */
class Glyph final {
    GlyphEntry* fEntry;

    Glyph() = delete;
    Glyph(const Glyph&) = delete;
    Glyph(Glyph&&) = default;
    Glyph& operator=(const Glyph&) = delete;
    Glyph& operator=(Glyph&&) = delete;

public:
    explicit Glyph(GlyphEntry* entry) : fEntry{entry} { SkASSERT(entry); }
    SkPackedGlyphID packedID() const { return fEntry->fGlyphEntryKey.fPackedID; }
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

    GlyphData(sk_sp<TextStrike>);

    ~GlyphData();

    Glyph makeGlyphFromID(SkPackedGlyphID, MaskFormat);

    // Regenerate atlas entries for glyphs in range [begin, end).
    // Returns {success, glyphs_placed_in_atlas}.
    std::tuple<bool, int> regenerateAtlas(int begin,
                                          int end,
                                          sktext::gpu::GlyphVector& glyphVector,
                                          MaskFormat maskFormat,
                                          int srcPadding,
                                          Recorder* recorder);

    void fillInstanceData(const sktext::gpu::VertexFiller&,
                          SkSpan<const Glyph> glyphs,
                          DrawWriter* dw,
                          int offset,
                          int count,
                          unsigned short flags,
                          uint32_t ssboIndex,
                          SkScalar depth) const;

private:
    GlyphData(const GlyphData&) = delete;
    GlyphData(GlyphData&&) = delete;
    GlyphData& operator=(const GlyphData&) = delete;
    GlyphData& operator=(GlyphData&&) = delete;

    sk_sp<TextStrike> fTextStrike;
    uint64_t fAtlasGeneration{DrawAtlas::GenerationCounter::kInvalidGeneration};
    DrawAtlas::BulkUsePlotUpdater fBulkUseUpdater;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_GlyphData_DEFINED
