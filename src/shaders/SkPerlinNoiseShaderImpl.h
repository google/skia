/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPerlinNoiseShaderImpl_DEFINED
#define SkPerlinNoiseShaderImpl_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkOnce.h"
#include "src/shaders/SkShaderBase.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>

class SkReadBuffer;
enum class SkPerlinNoiseShaderType;
struct SkStageRec;
class SkWriteBuffer;

class SkPerlinNoiseShader : public SkShaderBase {
private:
    static constexpr int kBlockSize = 256;
    static constexpr int kBlockMask = kBlockSize - 1;
    static constexpr int kPerlinNoise = 4096;
    static constexpr int kRandMaximum = SK_MaxS32;  // 2**31 - 1

public:
    struct StitchData {
        StitchData() = default;

        StitchData(SkScalar w, SkScalar h)
                : fWidth(std::min(SkScalarRoundToInt(w), SK_MaxS32 - kPerlinNoise))
                , fWrapX(kPerlinNoise + fWidth)
                , fHeight(std::min(SkScalarRoundToInt(h), SK_MaxS32 - kPerlinNoise))
                , fWrapY(kPerlinNoise + fHeight) {}

        bool operator==(const StitchData& other) const {
            return fWidth == other.fWidth && fWrapX == other.fWrapX && fHeight == other.fHeight &&
                   fWrapY == other.fWrapY;
        }

        int fWidth = 0;  // How much to subtract to wrap for stitching.
        int fWrapX = 0;  // Minimum value to wrap.
        int fHeight = 0;
        int fWrapY = 0;
    };

    struct PaintingData {
        PaintingData(const SkISize& tileSize,
                     SkScalar seed,
                     SkScalar baseFrequencyX,
                     SkScalar baseFrequencyY) {
            fBaseFrequency.set(baseFrequencyX, baseFrequencyY);
            fTileSize.set(SkScalarRoundToInt(tileSize.fWidth),
                          SkScalarRoundToInt(tileSize.fHeight));
            this->init(seed);
            if (!fTileSize.isEmpty()) {
                this->stitch();
            }
        }

        void generateBitmaps() {
            SkImageInfo info = SkImageInfo::MakeA8(kBlockSize, 1);
            fPermutationsBitmap.installPixels(info, fLatticeSelector, info.minRowBytes());
            fPermutationsBitmap.setImmutable();

            info = SkImageInfo::Make(kBlockSize, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            fNoiseBitmap.installPixels(info, fNoise[0][0], info.minRowBytes());
            fNoiseBitmap.setImmutable();
        }

        PaintingData(const PaintingData& that)
                : fSeed(that.fSeed)
                , fTileSize(that.fTileSize)
                , fBaseFrequency(that.fBaseFrequency)
                , fStitchDataInit(that.fStitchDataInit)
                , fPermutationsBitmap(that.fPermutationsBitmap)
                , fNoiseBitmap(that.fNoiseBitmap) {
            memcpy(fLatticeSelector, that.fLatticeSelector, sizeof(fLatticeSelector));
            memcpy(fNoise, that.fNoise, sizeof(fNoise));
        }

        int fSeed;
        uint8_t fLatticeSelector[kBlockSize];
        uint16_t fNoise[4][kBlockSize][2];
        SkISize fTileSize;
        SkVector fBaseFrequency;
        StitchData fStitchDataInit;

    private:
        SkBitmap fPermutationsBitmap;
        SkBitmap fNoiseBitmap;

        int random() {
            // See https://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
            // m = kRandMaximum, 2**31 - 1 (2147483647)
            static constexpr int kRandAmplitude = 16807;  // 7**5; primitive root of m
            static constexpr int kRandQ = 127773;         // m / a
            static constexpr int kRandR = 2836;           // m % a

            int result = kRandAmplitude * (fSeed % kRandQ) - kRandR * (fSeed / kRandQ);
            if (result <= 0) {
                result += kRandMaximum;
            }
            fSeed = result;
            return result;
        }

        // Only called once. Could be part of the constructor.
        void init(SkScalar seed) {
            // According to the SVG spec, we must truncate (not round) the seed value.
            fSeed = SkScalarTruncToInt(seed);
            // The seed value clamp to the range [1, kRandMaximum - 1].
            if (fSeed <= 0) {
                fSeed = -(fSeed % (kRandMaximum - 1)) + 1;
            }
            if (fSeed > kRandMaximum - 1) {
                fSeed = kRandMaximum - 1;
            }
            for (int channel = 0; channel < 4; ++channel) {
                for (int i = 0; i < kBlockSize; ++i) {
                    fLatticeSelector[i] = i;
                    fNoise[channel][i][0] = (random() % (2 * kBlockSize));
                    fNoise[channel][i][1] = (random() % (2 * kBlockSize));
                }
            }
            for (int i = kBlockSize - 1; i > 0; --i) {
                int k = fLatticeSelector[i];
                int j = random() % kBlockSize;
                SkASSERT(j >= 0);
                SkASSERT(j < kBlockSize);
                fLatticeSelector[i] = fLatticeSelector[j];
                fLatticeSelector[j] = k;
            }

            // Perform the permutations now
            {
                // Copy noise data
                uint16_t noise[4][kBlockSize][2];
                for (int i = 0; i < kBlockSize; ++i) {
                    for (int channel = 0; channel < 4; ++channel) {
                        for (int j = 0; j < 2; ++j) {
                            noise[channel][i][j] = fNoise[channel][i][j];
                        }
                    }
                }
                // Do permutations on noise data
                for (int i = 0; i < kBlockSize; ++i) {
                    for (int channel = 0; channel < 4; ++channel) {
                        for (int j = 0; j < 2; ++j) {
                            fNoise[channel][i][j] = noise[channel][fLatticeSelector[i]][j];
                        }
                    }
                }
            }

            // Half of the largest possible value for 16 bit unsigned int
            static constexpr SkScalar kHalfMax16bits = 32767.5f;

            // Compute gradients from permuted noise data
            static constexpr SkScalar kInvBlockSizef = 1.0 / SkIntToScalar(kBlockSize);
            for (int channel = 0; channel < 4; ++channel) {
                for (int i = 0; i < kBlockSize; ++i) {
                    SkPoint gradient =
                            SkPoint::Make((fNoise[channel][i][0] - kBlockSize) * kInvBlockSizef,
                                          (fNoise[channel][i][1] - kBlockSize) * kInvBlockSizef);
                    gradient.normalize();
                    // Put the normalized gradient back into the noise data
                    fNoise[channel][i][0] = SkScalarRoundToInt((gradient.fX + 1) * kHalfMax16bits);
                    fNoise[channel][i][1] = SkScalarRoundToInt((gradient.fY + 1) * kHalfMax16bits);
                }
            }
        }

        // Only called once. Could be part of the constructor.
        void stitch() {
            SkScalar tileWidth = SkIntToScalar(fTileSize.width());
            SkScalar tileHeight = SkIntToScalar(fTileSize.height());
            SkASSERT(tileWidth > 0 && tileHeight > 0);
            // When stitching tiled turbulence, the frequencies must be adjusted
            // so that the tile borders will be continuous.
            if (fBaseFrequency.fX) {
                SkScalar lowFrequencx =
                        SkScalarFloorToScalar(tileWidth * fBaseFrequency.fX) / tileWidth;
                SkScalar highFrequencx =
                        SkScalarCeilToScalar(tileWidth * fBaseFrequency.fX) / tileWidth;
                // BaseFrequency should be non-negative according to the standard.
                // lowFrequencx can be 0 if fBaseFrequency.fX is very small.
                if (sk_ieee_float_divide(fBaseFrequency.fX, lowFrequencx) <
                    highFrequencx / fBaseFrequency.fX) {
                    fBaseFrequency.fX = lowFrequencx;
                } else {
                    fBaseFrequency.fX = highFrequencx;
                }
            }
            if (fBaseFrequency.fY) {
                SkScalar lowFrequency =
                        SkScalarFloorToScalar(tileHeight * fBaseFrequency.fY) / tileHeight;
                SkScalar highFrequency =
                        SkScalarCeilToScalar(tileHeight * fBaseFrequency.fY) / tileHeight;
                // lowFrequency can be 0 if fBaseFrequency.fY is very small.
                if (sk_ieee_float_divide(fBaseFrequency.fY, lowFrequency) <
                    highFrequency / fBaseFrequency.fY) {
                    fBaseFrequency.fY = lowFrequency;
                } else {
                    fBaseFrequency.fY = highFrequency;
                }
            }
            fStitchDataInit =
                    StitchData(tileWidth * fBaseFrequency.fX, tileHeight * fBaseFrequency.fY);
        }

    public:
        const SkBitmap& getPermutationsBitmap() const {
            SkASSERT(!fPermutationsBitmap.drawsNothing());
            return fPermutationsBitmap;
        }
        const SkBitmap& getNoiseBitmap() const {
            SkASSERT(!fNoiseBitmap.drawsNothing());
            return fNoiseBitmap;
        }
    };  // struct PaintingData

    static const int kMaxOctaves = 255;  // numOctaves must be <= 0 and <= kMaxOctaves

    SkPerlinNoiseShader(SkPerlinNoiseShaderType type,
                        SkScalar baseFrequencyX,
                        SkScalar baseFrequencyY,
                        int numOctaves,
                        SkScalar seed,
                        const SkISize* tileSize);

    ShaderType type() const override { return ShaderType::kPerlinNoise; }

    SkPerlinNoiseShaderType noiseType() const { return fType; }
    int numOctaves() const { return fNumOctaves; }
    bool stitchTiles() const { return fStitchTiles; }
    SkISize tileSize() const { return fTileSize; }

    std::unique_ptr<PaintingData> getPaintingData() const {
        return std::make_unique<PaintingData>(fTileSize, fSeed, fBaseFrequencyX, fBaseFrequencyY);
    }

    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkPerlinNoiseShader)

    const SkPerlinNoiseShaderType fType;
    const SkScalar fBaseFrequencyX;
    const SkScalar fBaseFrequencyY;
    const int fNumOctaves;
    const SkScalar fSeed;
    const SkISize fTileSize;
    const bool fStitchTiles;

    mutable SkOnce fInitPaintingDataOnce;
    std::unique_ptr<PaintingData> fPaintingData;

    friend void SkRegisterPerlinNoiseShaderFlattenable();
};

#endif
