/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_PackedGPUGlyphID_DEFINED
#define sktext_gpu_PackedGPUGlyphID_DEFINED

#include "src/core/SkGlyph.h"
#include "src/gpu/MaskFormat.h"

namespace sktext::gpu {

// PackedGPUGlyphID represents an SkPackedGlyphID and the assumptions around how the glyph mask
// will be used on the GPU:
//  1. The mask format
//  2. What the mask data stores, e.g. coverage or distances
//  3. The amount of padding around the glyph that can be safely sampled
struct PackedGPUGlyphID {
    // `padding` must be 0, 1, or 2 and should be determined by the SubRun choice, it should never
    // be a parameter controlled by SubRun data.
    constexpr PackedGPUGlyphID(SkPackedGlyphID id,
                               skgpu::MaskFormat format,
                               int padding,
                               bool isSDF)
            : fID(id.value() | PackGPUData(format, padding, isSDF)) {}

    bool operator==(const PackedGPUGlyphID& that) const { return fID == that.fID; }
    bool operator!=(const PackedGPUGlyphID& that) const { return !(*this == that); }

    explicit constexpr operator SkPackedGlyphID() const { return this->packedGlyphID(); }

    // NOTE: SkPackedGlyphID's raw uint32_t constructor masks off bits it doesn't know about.
    constexpr SkPackedGlyphID packedGlyphID() const { return SkPackedGlyphID(fID); }

    constexpr skgpu::MaskFormat maskFormat() const {
        return static_cast<skgpu::MaskFormat>((fID >> kMaskFormatOffset) & kMaskFormatMask);
    }

    constexpr int padding() const {
        return static_cast<int>((fID >> kPaddingOffset) & kPaddingMask);
    }

    constexpr bool isSDF() const {
        return SkToBool((fID >> kSDFOffset) & kSDFMask);
    }

    // This hash incorporates the GPU metadata so it will not necessarily hash to the same value as
    // what SkPackedGlyphID hashes to by itself.
    uint32_t hash() const {
        return SkChecksum::CheapMix(fID);
    }

private:
    // SkPackedGlyphID is 20 bits, so we can use 5 of the spare to store the GPU metadata that
    // determines the full contents of a glyph entry in the atlas.
    enum {
        // Bit counts
        kMaskFormatBits = 2u,
        kPaddingBits = 2u, // only need 0, 1, or 2 pixels of padding currently
        kSDFBits = 1u,

        // Bit offsets
        kMaskFormatOffset = SkPackedGlyphID::kEndData,
        kPaddingOffset = kMaskFormatOffset + kMaskFormatBits,
        kSDFOffset = kPaddingOffset + kPaddingBits,
        kEndGPUData = kSDFOffset + kSDFBits,

        // Masks
        kMaskFormatMask = (1u << kMaskFormatBits) - 1,
        kPaddingMask = (1u << kPaddingBits) - 1,
        kSDFMask = (1u << kSDFBits) - 1,
    };

    static_assert(skgpu::kMaskFormatCount <= (1 << kMaskFormatBits));
    static_assert(kEndGPUData <= 32); // Must still fit within a uint32_t

    static constexpr uint32_t PackGPUData(skgpu::MaskFormat format, int padding, bool isSDF) {
        SkASSERT((uint32_t) format <= (1u << kMaskFormatBits));
        SkASSERT(0 <= padding && padding <= (1 << kPaddingBits));
        return ((uint32_t) format << kMaskFormatOffset) |
               ((uint32_t) padding << kPaddingOffset) |
               ((uint32_t) isSDF << kSDFOffset);
    }

    uint32_t fID;
};

} // namespace sktext::gpu

#endif  // sktext_gpu_PackedGPUGlyphID_DEFINED
