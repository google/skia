/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED

#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorSimd {
public:
    // Type aliases for convenient handling of the simd registers. In addition to SIMD, we use SWAR
    // to pack subsample winding 4 to 1 u32, each subsample being alloted 1 byte of memory, e.g.
    // -128 / 127 of winding. So a quality of 8 subsamples per pixel (MSAAx8) requires u32x2
    // (SwarPixel) or u8x8 (PixelBytes). Note, currently, this class is hardcoded to MSAAx8.
    // Although it would be fairly straight forward to make this class handle a variable subsample
    // count, we do not anticipate running above MSAAx8 on the CPU, due to a lack of throughput.
    // (Many devices do not 256-wide SIMD, doubling the subsample count doubles memory requirements,
    // etc.)
    static_assert(Strip::kNumSubSamples == 8);
    using SwarPixel = skvx::Vec<2, uint32_t>;
    using PixelBytes = skvx::Vec<8, uint8_t>;

    StripProcessorSimd(SkTDArray<Strip>* stripBuf,
                       SkTDArray<uint8_t>* alphaBuf,
                       bool isInverse,
                       const Polyline& polyline,
                       const SkTDArray<uint8_t>& maskLut,
                       int32_t initialAlphaIdx
#if defined(GPU_TEST_UTILS)
                       , MsaaExactMaskObserver observer
#endif
                       )
            : fCoarseWinding(0)
            , fStripBuf(stripBuf)
            , fAlphaBuf(alphaBuf)
            , fIsInverse(isInverse)
            , fPolyline(polyline)
            , fMaskLut(maskLut)
            , fLocalAlphaIdx(initialAlphaIdx)
#if defined(GPU_TEST_UTILS)
            , fObserver(observer)
#endif
    {
        this->clearWinding(kInitialWinding);
    }

    SK_ALWAYS_INLINE void clearWinding(uint8_t val) {
        std::memset(fSubsampleWinding, val, sizeof(fSubsampleWinding));
    }

    SK_ALWAYS_INLINE void clearWithCoarseWinding() {
        uint8_t windingByte;
        if constexpr (kIsWinding) {
            // Cast to i8 to sign extend before casting to u8, then apply SWAR bias to map -127/128
            // to 0/255.
            windingByte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(fCoarseWinding));
        } else {
            windingByte = (fCoarseWinding & 1) ? 1 : 0;
        }
        this->clearWinding(windingByte);
    }

    SK_ALWAYS_INLINE void clearWindingForNewRow() { this->clearWinding(kInitialWinding); }

    SK_ALWAYS_INLINE static bool ShouldFill(int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    }

    SK_ALWAYS_INLINE int32_t coarseWinding() const { return fCoarseWinding; }
    SK_ALWAYS_INLINE void setCoarseWinding(int32_t val) { fCoarseWinding = val; }
    SK_ALWAYS_INLINE int32_t localAlphaIdx() const { return fLocalAlphaIdx; }

    // Convert the winding to alpha in row sized chunks. Technically, processChunk could be renamed
    // to processRow, but it is intended to be flexible, so that if the tile width were to exceed
    // the simd width, the row could be proccessed in serial chunks.
    SK_ALWAYS_INLINE void resolveWindingToAlpha() {
        uint8_t* tileAlphaBase = reserveAlphaBuffer();
        for (int32_t row = 0; row < kTileHeight; ++row) {
            if constexpr (kTileWidth % 8 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 8) {
                    this->processChunk<8>(row, column, tileAlphaBase);
                    tileAlphaBase += 8;
                }
            } else if constexpr (kTileWidth % 4 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 4) {
                    this->processChunk<4>(row, column, tileAlphaBase);
                    tileAlphaBase += 4;
                }
            }
        }
        fLocalAlphaIdx += kTilePixelCount;
    }

    SK_ALWAYS_INLINE void rasterizeLineToTile(const Tile& tile, std::array<SkPoint, 2> tileBounds) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalYDir = line.p1.fY >= line.p0.fY;
        if (canonicalYDir) {
            this->rasterizeLineToTileImpl</*kCanonicalYDir=*/true>(tile, tileBounds, line);
        } else {
            this->rasterizeLineToTileImpl</*kCanonicalYDir=*/false>(tile, tileBounds, line);
        }
    }

    template <bool kCanonicalYDir>
    SK_ALWAYS_INLINE void rasterizeLineToTileImpl(const Tile& tile,
                                                  std::array<SkPoint, 2> tileBounds,
                                                  const Line& line) {
        bool canonicalXDir = line.p1.fX >= line.p0.fX;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (kCanonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        // TODO (thomsmit): remove once culling lands
        // Cull lines that exist entirely to the left of the tile.
        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float invDx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float invDy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * invDy;
        std::array<float, 4> derivs = {dx, dy, invDx, invDy};

        auto [clippedLine, topIsOnLeftEdge, botIsOnLeftEdge] =
                Tile::ClipToTile<kTileWidth, kTileHeight>(line,
                                                          tileBounds,
                                                          derivs,
                                                          tile.intersectionMask(),
                                                          canonicalXDir,
                                                          kCanonicalYDir);
        SkPoint pTop = clippedLine.p0;
        SkPoint pBot = clippedLine.p1;

        if (tile.hasLeftIntersection()) {
            float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
            this->fillLeft(yEdge, canonicalXDir);
        }

        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));
        if (startY < endY) {
            bool isOnlyRow = (startY == endY - 1);
            auto rowInt = FindRowIntersections(pTop, pBot, dxdy, startY, endY);
            LineStepParams stepParams = this->computeLineStepParams(pTop, pBot, dx, dy);

            // First, and possibly only row
            {
                float pTopY = pTop.fY;
                float pBotY = isOnlyRow ? pBot.fY : static_cast<float>(startY + 1);

                bool crossedTop = (pTopY == std::floor(pTopY));
                bool defaultInvert = !crossedTop && stepParams.fSortedXDir;

                uint8_t startMaskVal = 0xff;
                bool startInvert = topIsOnLeftEdge && !crossedTop;
                if (!topIsOnLeftEdge) {
                    startMaskVal = GetTruncationMask</*kIsStart=*/true>(pTopY,
                                                                        static_cast<float>(startY));
                }

                uint8_t endMaskVal = 0xff;
                if (isOnlyRow && !botIsOnLeftEdge) {
                    endMaskVal = GetTruncationMask</*kIsStart=*/false>(pBotY,
                                                                       static_cast<float>(startY));
                }

                bool leftInvert = stepParams.fSortedXDir ? startInvert : defaultInvert;
                bool rightInvert = stepParams.fSortedXDir ? defaultInvert : startInvert;

                uint8_t leftMask = stepParams.fSortedXDir ? startMaskVal : endMaskVal;
                uint8_t rightMask = stepParams.fSortedXDir ? endMaskVal : startMaskVal;
                this->processRowSpan<kCanonicalYDir>(startY, leftMask, rightMask, leftInvert,
                                                     defaultInvert, rightInvert, crossedTop, rowInt,
                                                     stepParams);
            }

            // Middle rows
            for (int32_t row = startY + 1; row < endY - 1; ++row) {
                this->processRowSpan<kCanonicalYDir>(row, /*leftMask=*/0xff, /*rightMask=*/0xff,
                                                     /*leftInvert=*/false, /*midInvert=*/false,
                                                     /*rightInvert=*/false, /*crossedTop=*/true,
                                                     rowInt, stepParams);
            }

            // Bottom row, if it exists
            if (!isOnlyRow) {
                int32_t lastY = endY - 1;
                uint8_t endMaskLast = 0xff;
                if (!botIsOnLeftEdge) {
                    endMaskLast = GetTruncationMask</*kIsStart=*/false>(pBot.fY,
                                                                        static_cast<float>(lastY));
                }

                uint8_t leftMask = stepParams.fSortedXDir ? 0xff : endMaskLast;
                uint8_t rightMask = stepParams.fSortedXDir ? endMaskLast : 0xff;
                this->processRowSpan<kCanonicalYDir>(lastY, leftMask, rightMask,
                                                     /*leftInvert=*/false, /*midInvert=*/false,
                                                     /*rightInvert=*/false, /*crossedTop=*/true,
                                                     rowInt, stepParams);
            }
        }
    }

private:
    static constexpr int32_t kTilePixelCount = kTileWidth * kTileHeight;
    static constexpr uint8_t kInitialWinding = kIsWinding ? 0x80 : 0;

    struct LineStepParams {
        const uint8_t* fMaskRowLut;
        int32_t fStepXFixed;
        int32_t fStepYFixed;
        int32_t fTBaseFixed;
        bool fSortedXDir;
    };

    template <int N>
    SK_ALWAYS_INLINE static void ApplyWinding(SwarPixel* target, uint32_t fillVal) {
        auto vSubsampleWinding = skvx::Vec<N, uint32_t>::Load(target);
        skvx::Vec<N, uint32_t> vFill(fillVal);
        if constexpr (kIsWinding) {
            auto windingBytes = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vSubsampleWinding);
            auto fillBytes = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vFill);
            vSubsampleWinding = sk_bit_cast<skvx::Vec<N, uint32_t>>(windingBytes + fillBytes);
        } else {
            vSubsampleWinding ^= vFill;
        }
        vSubsampleWinding.store(target);
    }

    // Evaluate the subsamples against the winding rule to determine which are "active" (covered).
    // For Even-Odd no processing is required. For the Non-Zero case, we compare subsample winding
    // against the empty SWAR mask. In SkVX, the result of !=, true, is ~0, so we mask away any
    // extraneous bits.
    template <typename VecT> static SK_ALWAYS_INLINE VecT GetActivesWide(VecT v) {
        using Vec8 = skvx::Vec<sizeof(VecT), uint8_t>;
        if constexpr (kIsWinding) {
            const Vec8 emptyBytes(0x80);
            Vec8 vecBytes = sk_bit_cast<skvx::Vec<sizeof(VecT), uint8_t>>(v);
            Vec8 cmp = (vecBytes != emptyBytes);
            return sk_bit_cast<VecT>(cmp) & VecT(0x01010101u);
        } else {
            SkASSERT(all(sk_bit_cast<Vec8>(v) <= 1));
            return v;
        }
    }

    SK_ALWAYS_INLINE uint8_t* reserveAlphaBuffer() {
        if (fAlphaBuf->size() + kTilePixelCount > fAlphaBuf->capacity()) {
            constexpr size_t kChunkSize = 4 * kTilePixelCount;
            fAlphaBuf->reserve(fAlphaBuf->capacity() + kChunkSize);
        }
        return fAlphaBuf->append(kTilePixelCount);
    }

#if defined(GPU_TEST_UTILS)
    SK_ALWAYS_INLINE void observeChunk(int32_t row, int32_t column, int32_t chunkSize) {
        for (int32_t x = column; x < column + chunkSize; ++x) {
            SwarPixel v = fSubsampleWinding[row][x];
            uint8_t exactMask = 0;
            uint32_t lo = v[0];
            uint32_t hi = v[1];
            for (int s = 0; s < 4; ++s) {
                int8_t sLo = static_cast<int8_t>(lo & 0xFF);
                int8_t sHi = static_cast<int8_t>(hi & 0xFF);
                if constexpr (kIsWinding) {
                    sLo = static_cast<int8_t>(static_cast<uint8_t>(sLo) - 0x80);
                    sHi = static_cast<int8_t>(static_cast<uint8_t>(sHi) - 0x80);
                }
                if (ShouldFill(sLo)) exactMask |= (1 << s);
                if (ShouldFill(sHi)) exactMask |= (1 << (s + 4));
                lo >>= 8;
                hi >>= 8;
            }
            if (fIsInverse) {
                exactMask = ~exactMask & ((1 << Strip::kNumSubSamples) - 1);
            }
            fObserver(exactMask);
        }
    }
#endif

    // Resolves the subsample windings of a chunk of pixels to their equivalent 8-bit alpha values:
    //
    //  1) GetActivesWide(): Evaluates the winding rule, returning a skvx::Vec of the same size
    //      where each byte lane contains 1 (active) or 0 (inactive).
    //  2) activeLo / activeHi: Splits the active masks into two separate 32-bit words, each
    //     containing 4 subsamples (one per byte lane).
    //  3) combinedBytes: Sums the two halves together. Each byte lane now holds a value of 0, 1,
    //     or 2.
    //  4) SWAR Horizontal Sum: Multiplying by 0x01010101u (which is 2^24 + 2^16 + 2^8 + 1)
    //     accumulates the sum of all 4 byte lanes into the highest byte. Given a 32-bit word
    //     [ A | B | C | D ], the multiplication expands across the lanes:
    //          Shifted by 0:  [ A | B | C | D ]
    //        + Shifted by 8:  [ B | C | D | 0 ]
    //        + Shifted by 16: [ C | D | 0 | 0 ]
    //        + Shifted by 24: [ D | 0 | 0 | 0 ]
    //     Summed together, the highest byte holds A + B + C + D. This can never overflow into
    //     neighboring lanes, since the maximum value per byte lane is 2.
    //  5) Shifting >> 24: Right-shifting the result isolates the final sum. This discards the lower
    //     accumulation bytes and moves the total subsample count into the lowest byte.
    //  6) Alpha Conversion: The total is multiplied by the maximum alpha value (255), summed by
    //     subsampleCount / 2 for rounding, and right-shifted by log2(subsampleCount). This keeps
    //     the entire conversion in integer math and avoids division.
    template <int kChunkSize>
    SK_ALWAYS_INLINE void processChunk(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase) {
        auto vSubsampleWinding =
                skvx::Vec<kChunkSize * 2, uint32_t>::Load(&fSubsampleWinding[row][column]);
        auto actives = GetActivesWide(vSubsampleWinding);

        skvx::Vec<kChunkSize, uint32_t> activeLo;
        skvx::Vec<kChunkSize, uint32_t> activeHi;
        if constexpr (kChunkSize == 8) {
            activeLo = skvx::shuffle<0, 2, 4, 6, 8, 10, 12, 14>(actives);
            activeHi = skvx::shuffle<1, 3, 5, 7, 9, 11, 13, 15>(actives);
        } else {
            activeLo = skvx::shuffle<0, 2, 4, 6>(actives);
            activeHi = skvx::shuffle<1, 3, 5, 7>(actives);
        }

        auto combinedBytes = activeLo + activeHi;
        skvx::Vec<kChunkSize, uint32_t> activeSamples = (combinedBytes * 0x01010101u) >> 24;
        const uint32_t invMask32 = fIsInverse ? 0xffffffff : 0;
        skvx::Vec<kChunkSize, uint32_t> alpha32 = ((activeSamples * 255 + 4) >> 3) ^ invMask32;
        skvx::cast<uint8_t>(alpha32).store(tileAlphaBase);
#if defined(GPU_TEST_UTILS)
        if (fObserver) {
            observeChunk(row, column, kChunkSize);
        }
#endif
    }

    SK_ALWAYS_INLINE static std::array<float, kTileHeight + 1> FindRowIntersections(
            SkPoint pTop, SkPoint pBot, float dxdy, int32_t startY, int32_t endY) {
        std::array<float, kTileHeight + 1> rowInt;
        skvx::float4 vPTopX(pTop.fX);
        skvx::float4 vPTopY(pTop.fY);
        skvx::float4 vDxDy(dxdy);
        skvx::float4 vBase(0.0f, 1.0f, 2.0f, 3.0f);

        static_assert((kTileHeight & 3) == 0);
        for (int32_t k = 0; k < kTileHeight; k += 4) {
            skvx::float4 vGridY = skvx::float4(static_cast<float>(k)) + vBase;
            skvx::float4 vGridX = vPTopX + (vGridY - vPTopY) * vDxDy;
            vGridX.store(rowInt.data() + k);
        }

        rowInt[startY] = pTop.fX;
        rowInt[endY] = pBot.fX;
        return rowInt;
    }

    template<bool kIsStart>
    SK_ALWAYS_INLINE static uint8_t GetTruncationMask(float p, float row) {
        uint32_t shift = static_cast<uint32_t>(std::round(8.0f * (p - static_cast<float>(row))));
        if constexpr (kIsStart) {
            return static_cast<uint8_t>(0xff << shift);
        } else {
            return static_cast<uint8_t>(~(0xff << shift));
        }
    }

    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(SkPoint pTop, SkPoint pBot,
                                                          float dx, float dy) const {
        float normalX = dy;
        float normalY = -dx;
        if (normalX < 0.0f) {
            normalX = -normalX;
            normalY = -normalY;
        }
        float D = normalX + std::abs(normalY);
        float invD = (D < Strip::kStripEpsilon) ? 0.0f : 1.0f / D;

        bool hasPositiveSlope = normalY <= 0.0f;
        float C = normalX * pTop.fX + normalY * pTop.fY;
        float s = std::abs(normalY) * invD;
        int lutRowOffset = std::clamp(
                static_cast<int>(std::floor(s * (Strip::kLutMaskHeight / 2))),
                0,
                (Strip::kLutMaskHeight / 2) - 1);
        int lutRow = hasPositiveSlope ? (lutRowOffset + Strip::kLutMaskHeight / 2) : lutRowOffset;

        // Unlike the scalar version, we simply return the raw pointer to the row in the LUT
        const uint8_t* maskRowLut = fMaskLut.data() + (lutRow * Strip::kLutMaskWidth);

        float stepX = normalX * invD;
        float stepY = normalY * invD;
        float tBase = ((hasPositiveSlope ? normalX : D) - C) * invD;

        // We use 16.16 fixed-point arithmetic to avoid floating-point math and conversions inside
        // the inner loops. A 16.16 fixed-point number uses a 32-bit integer where:
        //   - The upper 16 bits represent the integer part.
        //   - The lower 16 bits represent the fractional part.
        //
        // Encoding: value_fixed = round(x * 65536)
        // Decoding (floor): integer_part = value_fixed >> 16
        //
        // In addition to the 16.16 representation (scaled by 65536), we scale by the LUT's width
        // (64.0) so that extracting the LUT column index `u = floor(t * 64)` can be done using a
        // simple bit-shift: `u = clamp(tFixed >> 16, 0, 63)`.
        //
        // Note: We use direct static_cast conversions to int32_t instead of calling SkFloatToFixed
        // because the step and base variables are guaranteed to neither overflow or underflow, so
        // they don't require saturation checks.
        constexpr float kFixedMult = 64.0f * 65536.0f;
        int32_t stepXFixed = static_cast<int32_t>(stepX * kFixedMult);
        int32_t stepYFixed = static_cast<int32_t>(stepY * kFixedMult);
        int32_t tBaseFixed = static_cast<int32_t>(tBase * kFixedMult);

        bool sortedXDir = pTop.fX <= pBot.fX;

        return {
            maskRowLut,
            stepXFixed,
            stepYFixed,
            tBaseFixed,
            sortedXDir
        };
    }

    SK_ALWAYS_INLINE void fillLeft(float yEdge, bool canonicalXDir) {
        uint8_t fillByte;
        if constexpr (kIsWinding) {
            fillByte = canonicalXDir ? 0xFF : 1;
        } else {
            fillByte = 1;
        }

        uint32_t fill32 = fillByte * 0x01010101u;
        int32_t startY = static_cast<int32_t>(std::ceil(yEdge));
        for (int32_t row = startY; row < kTileHeight; ++row) {
            if constexpr (kTileWidth % 8 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 8) {
                    ApplyWinding<16>(&fSubsampleWinding[row][column], fill32);
                }
            } else if constexpr (kTileWidth % 4 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 4) {
                    ApplyWinding<8>(&fSubsampleWinding[row][column], fill32);
                }
            }
        }
    }

    template <bool kCanonicalYDir, bool kIsEdgePixel>
    SK_ALWAYS_INLINE void processPixel(SwarPixel* pixel,
                                       uint8_t truncationMask,
                                       const PixelBytes& pInvert,
                                       int32_t tFixed,
                                       const uint8_t* maskRowLut) {
        // Shift right by 16 to extract the integer LUT column index `u = floor(t * 64)`.
        int column = std::clamp(tFixed >> 16, 0, Strip::kLutMaskWidthExcl);
        uint8_t maskVal = maskRowLut[column];

        // Apply the truncation mask if we're one of the candidate pixels.
        if constexpr (kIsEdgePixel) {
            maskVal &= truncationMask;
        }

    /*
     * Convert 1 byte of packed coverage bits (maskVal) into 8 separate SIMD byte lanes.
     * E.g., maskVal = 0b10100011:
     *
     * 1) Splat maskVal into all 8 lanes:
     *    [   L0    |    L1    |    L2    |    L3    |    L4    |    L5    |    L6    |   L7    ]
     *    [10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011]
     *
     * 2) Bitwise AND with vBit (2^0, 2^1, ..., 2^7) to isolate bit k in lane k:
     *  & [00000001 | 00000010 | 00000100 | 00001000 | 00010000 | 00100000 | 01000000 | 10000000]
     *    ---------------------------------------------------------------------------------------
     *    [00000001 | 00000010 | 00000000 | 00000000 | 00000000 | 00100000 | 00000000 | 10000000]
     *
     * 3) In SkVX, Vector comparison of != 0 yields ~0 for true (SkVx.h:L316-L318,L357-359); this is
     *    then negated to produce the correct winding across the lanes:
     * != 0:   [  0xFF   |  0xFF   |  0x00   |  0x00   |  0x00   |  0xFF   |  0x00   |  0xFF   ]
     * Unary -:[  0x01   |  0x01   |  0x00   |  0x00   |  0x00   |  0x01   |  0x00   |  0x01   ]
     *         (+1 for covered lanes, 0x00 for uncovered)
     *
     * WARNING: relies on pcmpeqb behavior for 3! If this is not true, this will fail.
     */
        const PixelBytes vBit{1, 2, 4, 8, 16, 32, 64, 128};
        PixelBytes cmp = (PixelBytes(maskVal) & vBit) != 0;
        PixelBytes pSubsampleWinding = sk_bit_cast<PixelBytes>(*pixel);

        if constexpr (kIsWinding) {
            PixelBytes pRes = cmp - pInvert;
            if constexpr (kCanonicalYDir) {
                pSubsampleWinding -= pRes;
            } else {
                pSubsampleWinding += pRes;
            }
        } else {
            PixelBytes pRes = (cmp & 1) ^ pInvert;
            pSubsampleWinding ^= pRes;
        }

        (*pixel) = sk_bit_cast<SwarPixel>(pSubsampleWinding);
    }

    // The inversion masks could maybe be moved into templating, but for now simply expose them
    // as function arguments and rely on the compiler's DCE to optimize them.
    template <bool kCanonicalYDir>
    SK_ALWAYS_INLINE void processRowSpan(int32_t row,
                                         uint8_t leftMask, uint8_t rightMask,
                                         bool leftInvert, bool midInvert, bool rightInvert,
                                         bool crossedTop,
                                         const std::array<float, kTileHeight + 1>& rowInt,
                                         const LineStepParams& params) {
        float pTopX = rowInt[row];
        float pBotX = rowInt[row + 1];

        float xMin = std::fmin(pTopX, pBotX);
        float xMax = std::fmax(pTopX, pBotX);
        int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
        int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

        // Compute the initial translation parameter `tFixed` in 16.16 fixed-point format
        // (pre-scaled by 64 * 65536) at the starting pixel (xStart, row) of the span using:
        // tFixed = tBaseFixed + stepYFixed * row + stepXFixed * xStart
        int32_t tFixed =
                params.fTBaseFixed + (params.fStepYFixed * row) + (params.fStepXFixed * xStart);
        SwarPixel* rowSubsampleWindings = fSubsampleWinding[row];

        uint8_t invertByte = kIsWinding ? 0xff : 1;
        if (xStart == xEnd) {
            uint8_t combinedMask = leftMask & rightMask;
            processPixel<kCanonicalYDir, /*kIsEdgePixel=*/true>(
                    &rowSubsampleWindings[xStart], combinedMask, leftInvert ? invertByte : 0,
                    tFixed, params.fMaskRowLut);
            tFixed += params.fStepXFixed;
        } else {
            processPixel<kCanonicalYDir, /*kIsEdgePixel=*/true>(
                    &rowSubsampleWindings[xStart], leftMask, leftInvert ? invertByte : 0, tFixed,
                    params.fMaskRowLut);
            tFixed += params.fStepXFixed;

            for (int32_t column = xStart + 1; column < xEnd; ++column) {
                processPixel<kCanonicalYDir, /*kIsEdgePixel=*/false>(
                        &rowSubsampleWindings[column], 0, midInvert ? invertByte : 0, tFixed,
                        params.fMaskRowLut);
                tFixed += params.fStepXFixed;
            }

            processPixel<kCanonicalYDir, /*kIsEdgePixel=*/true>(
                    &rowSubsampleWindings[xEnd], rightMask, rightInvert ? invertByte : 0, tFixed,
                    params.fMaskRowLut);
            tFixed += params.fStepXFixed;
        }

        if (crossedTop) {
            uint8_t fillByte;
            if constexpr (kIsWinding) {
                fillByte = kCanonicalYDir ? 1 : 0xFF;
            } else {
                fillByte = 1;
            }

            int32_t column = xEnd + 1;
            uint32_t fill32 = fillByte * 0x01010101u;
            if constexpr (kTileWidth >= 8) {
                while (column + 8 <= kTileWidth) {
                    ApplyWinding<16>(&fSubsampleWinding[row][column], fill32);
                    column += 8;
                }
            } else if constexpr (kTileWidth >= 4) {
                while (column + 4 <= kTileWidth) {
                    ApplyWinding<8>(&fSubsampleWinding[row][column], fill32);
                    column += 4;
                }
            }

            const PixelBytes pFill(fillByte);
            for (; column < kTileWidth; ++column) {
                PixelBytes pSubsampleWinding =
                        sk_bit_cast<PixelBytes>(fSubsampleWinding[row][column]);
                if constexpr (kIsWinding) {
                    pSubsampleWinding += pFill;
                } else {
                    pSubsampleWinding ^= pFill;
                }
                fSubsampleWinding[row][column] = sk_bit_cast<SwarPixel>(pSubsampleWinding);
            }
        }
    }

    SwarPixel fSubsampleWinding[kTileHeight][kTileWidth];
    int32_t fCoarseWinding;
    SkTDArray<Strip>* fStripBuf;
    SkTDArray<uint8_t>* fAlphaBuf;
    bool fIsInverse;
    const Polyline& fPolyline;
    const SkTDArray<uint8_t>& fMaskLut;
    int32_t fLocalAlphaIdx;
#if defined(GPU_TEST_UTILS)
    MsaaExactMaskObserver fObserver;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED
