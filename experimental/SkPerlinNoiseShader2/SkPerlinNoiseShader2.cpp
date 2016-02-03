/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDither.h"
#include "SkPerlinNoiseShader2.h"
#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkShader.h"
#include "SkUnPreMultiply.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "SkGr.h"
#include "effects/GrConstColorProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

static const int kBlockSize = 256;
static const int kBlockMask = kBlockSize - 1;
static const int kPerlinNoise = 4096;
static const int kRandMaximum = SK_MaxS32; // 2**31 - 1

static uint8_t improved_noise_permutations[] = { 
    151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225, 140,  36, 103, 
     30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148, 247, 120, 234,  75,   0,  26, 
    197,  62,  94, 252, 219, 203, 117,  35,  11,  32,  57, 177,  33,  88, 237, 149,  56,  87, 174, 
     20, 125, 136, 171, 168,  68, 175,  74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231, 
     83, 111, 229, 122,  60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143, 
     54,  65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169, 200, 196, 
    135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,  52, 217, 226, 250, 124, 
    123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212, 207, 206,  59, 227,  47,  16,  58,  17, 
    182, 189,  28,  42, 223, 183, 170, 213, 119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 
    155, 167,  43, 172,   9, 129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 
    112, 104, 218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,  81, 
     51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157, 184,  84, 204, 176, 
    115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93, 222, 114,  67,  29,  24,  72, 243, 
    141, 128, 195,  78,  66, 215,  61, 156, 180,
    151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225, 140,  36, 103, 
     30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148, 247, 120, 234,  75,   0,  26, 
    197,  62,  94, 252, 219, 203, 117,  35,  11,  32,  57, 177,  33,  88, 237, 149,  56,  87, 174, 
     20, 125, 136, 171, 168,  68, 175,  74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231, 
     83, 111, 229, 122,  60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143, 
     54,  65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169, 200, 196, 
    135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,  52, 217, 226, 250, 124, 
    123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212, 207, 206,  59, 227,  47,  16,  58,  17, 
    182, 189,  28,  42, 223, 183, 170, 213, 119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 
    155, 167,  43, 172,   9, 129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 
    112, 104, 218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,  81, 
     51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157, 184,  84, 204, 176, 
    115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93, 222, 114,  67,  29,  24,  72, 243, 
    141, 128, 195,  78,  66, 215,  61, 156, 180
};

namespace {

// noiseValue is the color component's value (or color)
// limitValue is the maximum perlin noise array index value allowed
// newValue is the current noise dimension (either width or height)
inline int checkNoise(int noiseValue, int limitValue, int newValue) {
    // If the noise value would bring us out of bounds of the current noise array while we are
    // stiching noise tiles together, wrap the noise around the current dimension of the noise to
    // stay within the array bounds in a continuous fashion (so that tiling lines are not visible)
    if (noiseValue >= limitValue) {
        noiseValue -= newValue;
    }
    return noiseValue;
}

inline SkScalar smoothCurve(SkScalar t) {
    static const SkScalar SK_Scalar3 = 3.0f;

    // returns t * t * (3 - 2 * t)
    return SkScalarMul(SkScalarSquare(t), SK_Scalar3 - 2 * t);
}

} // end namespace

struct SkPerlinNoiseShader2::StitchData {
    StitchData()
      : fWidth(0)
      , fWrapX(0)
      , fHeight(0)
      , fWrapY(0)
    {}

    bool operator==(const StitchData& other) const {
        return fWidth == other.fWidth &&
               fWrapX == other.fWrapX &&
               fHeight == other.fHeight &&
               fWrapY == other.fWrapY;
    }

    int fWidth; // How much to subtract to wrap for stitching.
    int fWrapX; // Minimum value to wrap.
    int fHeight;
    int fWrapY;
};

struct SkPerlinNoiseShader2::PaintingData {
    PaintingData(const SkISize& tileSize, SkScalar seed,
                 SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                 const SkMatrix& matrix)
    {
        SkVector vec[2] = {
            { SkScalarInvert(baseFrequencyX),   SkScalarInvert(baseFrequencyY)  },
            { SkIntToScalar(tileSize.fWidth),   SkIntToScalar(tileSize.fHeight) },
        };
        matrix.mapVectors(vec, 2);

        fBaseFrequency.set(SkScalarInvert(vec[0].fX), SkScalarInvert(vec[0].fY));
        fTileSize.set(SkScalarRoundToInt(vec[1].fX), SkScalarRoundToInt(vec[1].fY));
        this->init(seed);
        if (!fTileSize.isEmpty()) {
            this->stitch();
        }

#if SK_SUPPORT_GPU
        fPermutationsBitmap.setInfo(SkImageInfo::MakeA8(kBlockSize, 1));
        fPermutationsBitmap.setPixels(fLatticeSelector);

        fNoiseBitmap.setInfo(SkImageInfo::MakeN32Premul(kBlockSize, 4));
        fNoiseBitmap.setPixels(fNoise[0][0]);

        fImprovedPermutationsBitmap.setInfo(SkImageInfo::MakeA8(256, 1));
        fImprovedPermutationsBitmap.setPixels(improved_noise_permutations);

        fGradientBitmap.setInfo(SkImageInfo::MakeN32Premul(16, 1));
        static uint8_t gradients[] = { 2, 2, 1, 0,
                                       0, 2, 1, 0, 
                                       2, 0, 1, 0, 
                                       0, 0, 1, 0, 
                                       2, 1, 2, 0, 
                                       0, 1, 2, 0, 
                                       2, 1, 0, 0, 
                                       0, 1, 0, 0, 
                                       1, 2, 2, 0, 
                                       1, 0, 2, 0, 
                                       1, 2, 0, 0, 
                                       1, 0, 0, 0, 
                                       2, 2, 1, 0, 
                                       1, 0, 2, 0, 
                                       0, 2, 1, 0, 
                                       1, 0, 0, 0 };
        fGradientBitmap.setPixels(gradients);
#endif
    }

    int         fSeed;
    uint8_t     fLatticeSelector[kBlockSize];
    uint16_t    fNoise[4][kBlockSize][2];
    SkPoint     fGradient[4][kBlockSize];
    SkISize     fTileSize;
    SkVector    fBaseFrequency;
    StitchData  fStitchDataInit;

private:

#if SK_SUPPORT_GPU
    SkBitmap   fPermutationsBitmap;
    SkBitmap   fNoiseBitmap;
    SkBitmap   fImprovedPermutationsBitmap;
    SkBitmap   fGradientBitmap;
#endif

    inline int random()  {
        static const int gRandAmplitude = 16807; // 7**5; primitive root of m
        static const int gRandQ = 127773; // m / a
        static const int gRandR = 2836; // m % a

        int result = gRandAmplitude * (fSeed % gRandQ) - gRandR * (fSeed / gRandQ);
        if (result <= 0)
            result += kRandMaximum;
        fSeed = result;
        return result;
    }

    // Only called once. Could be part of the constructor.
    void init(SkScalar seed)
    {
        static const SkScalar gInvBlockSizef = SkScalarInvert(SkIntToScalar(kBlockSize));

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
        static const SkScalar gHalfMax16bits = 32767.5f;

        // Compute gradients from permutated noise data
        for (int channel = 0; channel < 4; ++channel) {
            for (int i = 0; i < kBlockSize; ++i) {
                fGradient[channel][i] = SkPoint::Make(
                    SkScalarMul(SkIntToScalar(fNoise[channel][i][0] - kBlockSize),
                                gInvBlockSizef),
                    SkScalarMul(SkIntToScalar(fNoise[channel][i][1] - kBlockSize),
                                gInvBlockSizef));
                fGradient[channel][i].normalize();
                // Put the normalized gradient back into the noise data
                fNoise[channel][i][0] = SkScalarRoundToInt(SkScalarMul(
                    fGradient[channel][i].fX + SK_Scalar1, gHalfMax16bits));
                fNoise[channel][i][1] = SkScalarRoundToInt(SkScalarMul(
                    fGradient[channel][i].fY + SK_Scalar1, gHalfMax16bits));
            }
        }
    }

    // Only called once. Could be part of the constructor.
    void stitch() {
        SkScalar tileWidth  = SkIntToScalar(fTileSize.width());
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
            if (fBaseFrequency.fX / lowFrequencx < highFrequencx / fBaseFrequency.fX) {
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
            if (fBaseFrequency.fY / lowFrequency < highFrequency / fBaseFrequency.fY) {
                fBaseFrequency.fY = lowFrequency;
            } else {
                fBaseFrequency.fY = highFrequency;
            }
        }
        // Set up TurbulenceInitial stitch values.
        fStitchDataInit.fWidth  =
            SkScalarRoundToInt(tileWidth * fBaseFrequency.fX);
        fStitchDataInit.fWrapX  = kPerlinNoise + fStitchDataInit.fWidth;
        fStitchDataInit.fHeight =
            SkScalarRoundToInt(tileHeight * fBaseFrequency.fY);
        fStitchDataInit.fWrapY  = kPerlinNoise + fStitchDataInit.fHeight;
    }

public:

#if SK_SUPPORT_GPU
    const SkBitmap& getPermutationsBitmap() const { return fPermutationsBitmap; }

    const SkBitmap& getNoiseBitmap() const { return fNoiseBitmap; }

    const SkBitmap& getImprovedPermutationsBitmap() const { return fImprovedPermutationsBitmap; }

    const SkBitmap& getGradientBitmap() const { return fGradientBitmap; }
#endif
};

SkShader* SkPerlinNoiseShader2::CreateFractalNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                                  int numOctaves, SkScalar seed,
                                                  const SkISize* tileSize) {
    return new SkPerlinNoiseShader2(kFractalNoise_Type, baseFrequencyX, baseFrequencyY, numOctaves,
                                   seed, tileSize);
}

SkShader* SkPerlinNoiseShader2::CreateTurbulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                              int numOctaves, SkScalar seed,
                                              const SkISize* tileSize) {
    return new SkPerlinNoiseShader2(kTurbulence_Type, baseFrequencyX, baseFrequencyY, numOctaves,
                                   seed, tileSize);
}

SkShader* SkPerlinNoiseShader2::CreateImprovedNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                                    int numOctaves, SkScalar z) {
    return new SkPerlinNoiseShader2(kImprovedNoise_Type, baseFrequencyX, baseFrequencyY, numOctaves, 
                                    z, NULL);
}

SkPerlinNoiseShader2::SkPerlinNoiseShader2(SkPerlinNoiseShader2::Type type,
                                         SkScalar baseFrequencyX,
                                         SkScalar baseFrequencyY,
                                         int numOctaves,
                                         SkScalar seed,
                                         const SkISize* tileSize)
  : fType(type)
  , fBaseFrequencyX(baseFrequencyX)
  , fBaseFrequencyY(baseFrequencyY)
  , fNumOctaves(numOctaves > 255 ? 255 : numOctaves/*[0,255] octaves allowed*/)
  , fSeed(seed)
  , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
  , fStitchTiles(!fTileSize.isEmpty())
{
    SkASSERT(numOctaves >= 0 && numOctaves < 256);
}

SkPerlinNoiseShader2::~SkPerlinNoiseShader2() {
}

SkFlattenable* SkPerlinNoiseShader2::CreateProc(SkReadBuffer& buffer) {
    Type type = (Type)buffer.readInt();
    SkScalar freqX = buffer.readScalar();
    SkScalar freqY = buffer.readScalar();
    int octaves = buffer.readInt();
    SkScalar seed = buffer.readScalar();
    SkISize tileSize;
    tileSize.fWidth = buffer.readInt();
    tileSize.fHeight = buffer.readInt();

    switch (type) {
        case kFractalNoise_Type:
            return SkPerlinNoiseShader2::CreateFractalNoise(freqX, freqY, octaves, seed, &tileSize);
        case kTurbulence_Type:
            return SkPerlinNoiseShader2::CreateTubulence(freqX, freqY, octaves, seed, &tileSize);
        case kImprovedNoise_Type:
            return SkPerlinNoiseShader2::CreateImprovedNoise(freqX, freqY, octaves, seed);
        default:
            return nullptr;
    }
}

void SkPerlinNoiseShader2::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int) fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

SkScalar SkPerlinNoiseShader2::PerlinNoiseShaderContext::noise2D(
        int channel, const StitchData& stitchData, const SkPoint& noiseVector) const {
    struct Noise {
        int noisePositionIntegerValue;
        int nextNoisePositionIntegerValue;
        SkScalar noisePositionFractionValue;
        Noise(SkScalar component)
        {
            SkScalar position = component + kPerlinNoise;
            noisePositionIntegerValue = SkScalarFloorToInt(position);
            noisePositionFractionValue = position - SkIntToScalar(noisePositionIntegerValue);
            nextNoisePositionIntegerValue = noisePositionIntegerValue + 1;
        }
    };
    Noise noiseX(noiseVector.x());
    Noise noiseY(noiseVector.y());
    SkScalar u, v;
    const SkPerlinNoiseShader2& perlinNoiseShader = static_cast<const SkPerlinNoiseShader2&>(fShader);
    // If stitching, adjust lattice points accordingly.
    if (perlinNoiseShader.fStitchTiles) {
        noiseX.noisePositionIntegerValue =
            checkNoise(noiseX.noisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.noisePositionIntegerValue =
            checkNoise(noiseY.noisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
        noiseX.nextNoisePositionIntegerValue =
            checkNoise(noiseX.nextNoisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.nextNoisePositionIntegerValue =
            checkNoise(noiseY.nextNoisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
    }
    noiseX.noisePositionIntegerValue &= kBlockMask;
    noiseY.noisePositionIntegerValue &= kBlockMask;
    noiseX.nextNoisePositionIntegerValue &= kBlockMask;
    noiseY.nextNoisePositionIntegerValue &= kBlockMask;
    int i =
        fPaintingData->fLatticeSelector[noiseX.noisePositionIntegerValue];
    int j =
        fPaintingData->fLatticeSelector[noiseX.nextNoisePositionIntegerValue];
    int b00 = (i + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b10 = (j + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b01 = (i + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    int b11 = (j + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    SkScalar sx = smoothCurve(noiseX.noisePositionFractionValue);
    SkScalar sy = smoothCurve(noiseY.noisePositionFractionValue);
    // This is taken 1:1 from SVG spec: http://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
    SkPoint fractionValue = SkPoint::Make(noiseX.noisePositionFractionValue,
                                          noiseY.noisePositionFractionValue); // Offset (0,0)
    u = fPaintingData->fGradient[channel][b00].dot(fractionValue);
    fractionValue.fX -= SK_Scalar1; // Offset (-1,0)
    v = fPaintingData->fGradient[channel][b10].dot(fractionValue);
    SkScalar a = SkScalarInterp(u, v, sx);
    fractionValue.fY -= SK_Scalar1; // Offset (-1,-1)
    v = fPaintingData->fGradient[channel][b11].dot(fractionValue);
    fractionValue.fX = noiseX.noisePositionFractionValue; // Offset (0,-1)
    u = fPaintingData->fGradient[channel][b01].dot(fractionValue);
    SkScalar b = SkScalarInterp(u, v, sx);
    return SkScalarInterp(a, b, sy);
}

SkScalar SkPerlinNoiseShader2::PerlinNoiseShaderContext::calculateTurbulenceValueForPoint(
        int channel, StitchData& stitchData, const SkPoint& point) const {
    const SkPerlinNoiseShader2& perlinNoiseShader = static_cast<const SkPerlinNoiseShader2&>(fShader);
    if (perlinNoiseShader.fStitchTiles) {
        // Set up TurbulenceInitial stitch values.
        stitchData = fPaintingData->fStitchDataInit;
    }
    SkScalar turbulenceFunctionResult = 0;
    SkPoint noiseVector(SkPoint::Make(SkScalarMul(point.x(), fPaintingData->fBaseFrequency.fX),
                                      SkScalarMul(point.y(), fPaintingData->fBaseFrequency.fY)));
    SkScalar ratio = SK_Scalar1;
    for (int octave = 0; octave < perlinNoiseShader.fNumOctaves; ++octave) {
        SkScalar noise = noise2D(channel, stitchData, noiseVector);
        SkScalar numer = (perlinNoiseShader.fType == kFractalNoise_Type) ?
                            noise : SkScalarAbs(noise);
        turbulenceFunctionResult += numer / ratio;
        noiseVector.fX *= 2;
        noiseVector.fY *= 2;
        ratio *= 2;
        if (perlinNoiseShader.fStitchTiles) {
            // Update stitch values
            stitchData.fWidth  *= 2;
            stitchData.fWrapX   = stitchData.fWidth + kPerlinNoise;
            stitchData.fHeight *= 2;
            stitchData.fWrapY   = stitchData.fHeight + kPerlinNoise;
        }
    }

    // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
    // by fractalNoise and (turbulenceFunctionResult) by turbulence.
    if (perlinNoiseShader.fType == kFractalNoise_Type) {
        turbulenceFunctionResult =
            SkScalarMul(turbulenceFunctionResult, SK_ScalarHalf) + SK_ScalarHalf;
    }

    if (channel == 3) { // Scale alpha by paint value
        turbulenceFunctionResult *= SkIntToScalar(getPaintAlpha()) / 255;
    }

    // Clamp result
    return SkScalarPin(turbulenceFunctionResult, 0, SK_Scalar1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Improved Perlin Noise based on Java implementation found at http://mrl.nyu.edu/~perlin/noise/
static SkScalar fade(SkScalar t) { 
    return t * t * t * (t * (t * 6 - 15) + 10); 
}

static SkScalar lerp(SkScalar t, SkScalar a, SkScalar b) { 
    return a + t * (b - a); 
}

static SkScalar grad(int hash, SkScalar x, SkScalar y, SkScalar z) {
    int h = hash & 15;
    SkScalar u = h < 8 ? x : y;
    SkScalar v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

SkScalar SkPerlinNoiseShader2::PerlinNoiseShaderContext::calculateImprovedNoiseValueForPoint(
        int channel, const SkPoint& point) const {
    const SkPerlinNoiseShader2& perlinNoiseShader = static_cast<const SkPerlinNoiseShader2&>(fShader);
    SkScalar x = point.fX * perlinNoiseShader.fBaseFrequencyX;
    SkScalar y = point.fY * perlinNoiseShader.fBaseFrequencyY;
    // z offset between different channels, chosen arbitrarily
    static const SkScalar CHANNEL_DELTA = 1000.0f;
    SkScalar z = channel * CHANNEL_DELTA + perlinNoiseShader.fSeed;
    SkScalar result = 0;
    SkScalar ratio = SK_Scalar1;
    for (int i = 0; i < perlinNoiseShader.fNumOctaves; i++) {
        int X = SkScalarFloorToInt(x) & 255;
        int Y = SkScalarFloorToInt(y) & 255;
        int Z = SkScalarFloorToInt(z) & 255;
        SkScalar px = x - SkScalarFloorToScalar(x);
        SkScalar py = y - SkScalarFloorToScalar(y);
        SkScalar pz = z - SkScalarFloorToScalar(z);
        SkScalar u = fade(px);
        SkScalar v = fade(py);
        SkScalar w = fade(pz);
        uint8_t* permutations = improved_noise_permutations;
        int A  = permutations[X] + Y;
        int AA = permutations[A] + Z;
        int AB = permutations[A + 1] + Z;
        int B  = permutations[X + 1] + Y;
        int BA = permutations[B] + Z;
        int BB = permutations[B + 1] + Z;
        result += lerp(w, lerp(v, lerp(u, grad(permutations[AA    ], px    , py    , pz    ),
                                          grad(permutations[BA    ], px - 1, py    , pz    )),
                                  lerp(u, grad(permutations[AB    ], px    , py - 1, pz    ),
                                          grad(permutations[BB    ], px - 1, py - 1, pz    ))),
                          lerp(v, lerp(u, grad(permutations[AA + 1], px    , py    , pz - 1),
                                          grad(permutations[BA + 1], px - 1, py    , pz - 1)),
                                  lerp(u, grad(permutations[AB + 1], px    , py - 1, pz - 1),
                                          grad(permutations[BB + 1], px - 1, py - 1, pz - 1)))) /
                   ratio;
        x *= 2;
        y *= 2;
        ratio *= 2;
    }
    result = SkScalarClampMax((result + 1.0f) / 2.0f, 1.0f);
    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

SkPMColor SkPerlinNoiseShader2::PerlinNoiseShaderContext::shade(
        const SkPoint& point, StitchData& stitchData) const {
    const SkPerlinNoiseShader2& perlinNoiseShader = static_cast<const SkPerlinNoiseShader2&>(fShader);
    SkPoint newPoint;
    fMatrix.mapPoints(&newPoint, &point, 1);
    newPoint.fX = SkScalarRoundToScalar(newPoint.fX);
    newPoint.fY = SkScalarRoundToScalar(newPoint.fY);

    U8CPU rgba[4];
    for (int channel = 3; channel >= 0; --channel) {
        SkScalar value;
        if (perlinNoiseShader.fType == kImprovedNoise_Type) {
            value = calculateImprovedNoiseValueForPoint(channel, newPoint);
        }
        else {
            value = calculateTurbulenceValueForPoint(channel, stitchData, newPoint);
        }
        rgba[channel] = SkScalarFloorToInt(255 * value);
    }
    return SkPreMultiplyARGB(rgba[3], rgba[0], rgba[1], rgba[2]);
}

SkShader::Context* SkPerlinNoiseShader2::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    return new (storage) PerlinNoiseShaderContext(*this, rec);
}

size_t SkPerlinNoiseShader2::contextSize() const {
    return sizeof(PerlinNoiseShaderContext);
}

SkPerlinNoiseShader2::PerlinNoiseShaderContext::PerlinNoiseShaderContext(
        const SkPerlinNoiseShader2& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
{
    SkMatrix newMatrix = *rec.fMatrix;
    newMatrix.preConcat(shader.getLocalMatrix());
    if (rec.fLocalMatrix) {
        newMatrix.preConcat(*rec.fLocalMatrix);
    }
    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the setData() function.
    fMatrix.setTranslate(-newMatrix.getTranslateX() + SK_Scalar1, -newMatrix.getTranslateY() + SK_Scalar1);
    fPaintingData = new PaintingData(shader.fTileSize, shader.fSeed, shader.fBaseFrequencyX,
                                     shader.fBaseFrequencyY, newMatrix);
}

SkPerlinNoiseShader2::PerlinNoiseShaderContext::~PerlinNoiseShaderContext() { delete fPaintingData; }

void SkPerlinNoiseShader2::PerlinNoiseShaderContext::shadeSpan(
        int x, int y, SkPMColor result[], int count) {
    SkPoint point = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
    StitchData stitchData;
    for (int i = 0; i < count; ++i) {
        result[i] = shade(point, stitchData);
        point.fX += SK_Scalar1;
    }
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

class GrGLPerlinNoise2 : public GrGLSLFragmentProcessor {
public:
    GrGLPerlinNoise2(const GrProcessor&);
    virtual ~GrGLPerlinNoise2() {}

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:

    GrGLSLProgramDataManager::UniformHandle fStitchDataUni;
    SkPerlinNoiseShader2::Type              fType;
    bool                                    fStitchTiles;
    int                                     fNumOctaves;
    GrGLSLProgramDataManager::UniformHandle fBaseFrequencyUni;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrPerlinNoise2Effect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(SkPerlinNoiseShader2::Type type,
                                       int numOctaves, bool stitchTiles,
                                       SkPerlinNoiseShader2::PaintingData* paintingData,
                                       GrTexture* permutationsTexture, GrTexture* noiseTexture,
                                       const SkMatrix& matrix) {
        return new GrPerlinNoise2Effect(type, numOctaves, stitchTiles, paintingData,
                                       permutationsTexture, noiseTexture, matrix);
    }

    virtual ~GrPerlinNoise2Effect() { delete fPaintingData; }

    const char* name() const override { return "PerlinNoise"; }

    const SkPerlinNoiseShader2::StitchData& stitchData() const { return fPaintingData->fStitchDataInit; }

    SkPerlinNoiseShader2::Type type() const { return fType; }
    bool stitchTiles() const { return fStitchTiles; }
    const SkVector& baseFrequency() const { return fPaintingData->fBaseFrequency; }
    int numOctaves() const { return fNumOctaves; }
    const SkMatrix& matrix() const { return fCoordTransform.getMatrix(); }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GrGLPerlinNoise2(*this);
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GrGLPerlinNoise2::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const GrPerlinNoise2Effect& s = sBase.cast<GrPerlinNoise2Effect>();
        return fType == s.fType &&
               fPaintingData->fBaseFrequency == s.fPaintingData->fBaseFrequency &&
               fNumOctaves == s.fNumOctaves &&
               fStitchTiles == s.fStitchTiles &&
               fPaintingData->fStitchDataInit == s.fPaintingData->fStitchDataInit;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
    }

    GrPerlinNoise2Effect(SkPerlinNoiseShader2::Type type,
                        int numOctaves, bool stitchTiles,
                        SkPerlinNoiseShader2::PaintingData* paintingData,
                        GrTexture* permutationsTexture, GrTexture* noiseTexture,
                        const SkMatrix& matrix)
      : fType(type)
      , fNumOctaves(numOctaves)
      , fStitchTiles(stitchTiles)
      , fPermutationsAccess(permutationsTexture)
      , fNoiseAccess(noiseTexture)
      , fPaintingData(paintingData) {
        this->initClassID<GrPerlinNoise2Effect>();
        this->addTextureAccess(&fPermutationsAccess);
        this->addTextureAccess(&fNoiseAccess);
        fCoordTransform.reset(kLocal_GrCoordSet, matrix);
        this->addCoordTransform(&fCoordTransform);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkPerlinNoiseShader2::Type       fType;
    GrCoordTransform                fCoordTransform;
    int                             fNumOctaves;
    bool                            fStitchTiles;
    GrTextureAccess                 fPermutationsAccess;
    GrTextureAccess                 fNoiseAccess;
    SkPerlinNoiseShader2::PaintingData *fPaintingData;

private:
    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrPerlinNoise2Effect);

const GrFragmentProcessor* GrPerlinNoise2Effect::TestCreate(GrProcessorTestData* d) {
    int      numOctaves = d->fRandom->nextRangeU(2, 10);
    bool     stitchTiles = d->fRandom->nextBool();
    SkScalar seed = SkIntToScalar(d->fRandom->nextU());
    SkISize  tileSize = SkISize::Make(d->fRandom->nextRangeU(4, 4096),
                                      d->fRandom->nextRangeU(4, 4096));
    SkScalar baseFrequencyX = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);
    SkScalar baseFrequencyY = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);

    SkAutoTUnref<SkShader> shader(d->fRandom->nextBool() ?
        SkPerlinNoiseShader2::CreateFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                                stitchTiles ? &tileSize : nullptr) :
        SkPerlinNoiseShader2::CreateTurbulence(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                             stitchTiles ? &tileSize : nullptr));

    GrPaint grPaint;
    return shader->asFragmentProcessor(d->fContext,
                                       GrTest::TestMatrix(d->fRandom), nullptr,
                                       kNone_SkFilterQuality);
}

GrGLPerlinNoise2::GrGLPerlinNoise2(const GrProcessor& processor)
  : fType(processor.cast<GrPerlinNoise2Effect>().type())
  , fStitchTiles(processor.cast<GrPerlinNoise2Effect>().stitchTiles())
  , fNumOctaves(processor.cast<GrPerlinNoise2Effect>().numOctaves()) {
}

void GrGLPerlinNoise2::emitCode(EmitArgs& args) {
    GrGLSLFragmentBuilder* fsBuilder = args.fFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    SkString vCoords = fsBuilder->ensureFSCoords2D(args.fCoords, 0);

    fBaseFrequencyUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "baseFrequency");
    const char* baseFrequencyUni = uniformHandler->getUniformCStr(fBaseFrequencyUni);

    const char* stitchDataUni = nullptr;
    if (fStitchTiles) {
        fStitchDataUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "stitchData");
        stitchDataUni = uniformHandler->getUniformCStr(fStitchDataUni);
    }

    // There are 4 lines, so the center of each line is 1/8, 3/8, 5/8 and 7/8
    const char* chanCoordR  = "0.125";
    const char* chanCoordG  = "0.375";
    const char* chanCoordB  = "0.625";
    const char* chanCoordA  = "0.875";
    const char* chanCoord   = "chanCoord";
    const char* stitchData  = "stitchData";
    const char* ratio       = "ratio";
    const char* noiseVec    = "noiseVec";
    const char* noiseSmooth = "noiseSmooth";
    const char* floorVal    = "floorVal";
    const char* fractVal    = "fractVal";
    const char* uv          = "uv";
    const char* ab          = "ab";
    const char* latticeIdx  = "latticeIdx";
    const char* bcoords     = "bcoords";
    const char* lattice     = "lattice";
    const char* inc8bit     = "0.00390625";  // 1.0 / 256.0
    // This is the math to convert the two 16bit integer packed into rgba 8 bit input into a
    // [-1,1] vector and perform a dot product between that vector and the provided vector.
    const char* dotLattice  = "dot(((%s.ga + %s.rb * vec2(%s)) * vec2(2.0) - vec2(1.0)), %s);";

    // Add noise function
    static const GrGLSLShaderVar gPerlinNoiseArgs[] =  {
        GrGLSLShaderVar(chanCoord, kFloat_GrSLType),
        GrGLSLShaderVar(noiseVec, kVec2f_GrSLType)
    };

    static const GrGLSLShaderVar gPerlinNoiseStitchArgs[] =  {
        GrGLSLShaderVar(chanCoord, kFloat_GrSLType),
        GrGLSLShaderVar(noiseVec, kVec2f_GrSLType),
        GrGLSLShaderVar(stitchData, kVec2f_GrSLType)
    };

    SkString noiseCode;

    noiseCode.appendf("\tvec4 %s;\n", floorVal);
    noiseCode.appendf("\t%s.xy = floor(%s);\n", floorVal, noiseVec);
    noiseCode.appendf("\t%s.zw = %s.xy + vec2(1.0);\n", floorVal, floorVal);
    noiseCode.appendf("\tvec2 %s = fract(%s);\n", fractVal, noiseVec);

    // smooth curve : t * t * (3 - 2 * t)
    noiseCode.appendf("\n\tvec2 %s = %s * %s * (vec2(3.0) - vec2(2.0) * %s);",
        noiseSmooth, fractVal, fractVal, fractVal);

    // Adjust frequencies if we're stitching tiles
    if (fStitchTiles) {
        noiseCode.appendf("\n\tif(%s.x >= %s.x) { %s.x -= %s.x; }",
            floorVal, stitchData, floorVal, stitchData);
        noiseCode.appendf("\n\tif(%s.y >= %s.y) { %s.y -= %s.y; }",
            floorVal, stitchData, floorVal, stitchData);
        noiseCode.appendf("\n\tif(%s.z >= %s.x) { %s.z -= %s.x; }",
            floorVal, stitchData, floorVal, stitchData);
        noiseCode.appendf("\n\tif(%s.w >= %s.y) { %s.w -= %s.y; }",
            floorVal, stitchData, floorVal, stitchData);
    }

    // Get texture coordinates and normalize
    noiseCode.appendf("\n\t%s = fract(floor(mod(%s, 256.0)) / vec4(256.0));\n",
        floorVal, floorVal);

    // Get permutation for x
    {
        SkString xCoords("");
        xCoords.appendf("vec2(%s.x, 0.5)", floorVal);

        noiseCode.appendf("\n\tvec2 %s;\n\t%s.x = ", latticeIdx, latticeIdx);
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[0], xCoords.c_str(),
                                       kVec2f_GrSLType);
        noiseCode.append(".r;");
    }

    // Get permutation for x + 1
    {
        SkString xCoords("");
        xCoords.appendf("vec2(%s.z, 0.5)", floorVal);

        noiseCode.appendf("\n\t%s.y = ", latticeIdx);
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[0], xCoords.c_str(),
                                       kVec2f_GrSLType);
        noiseCode.append(".r;");
    }

#if defined(SK_BUILD_FOR_ANDROID)
    // Android rounding for Tegra devices, like, for example: Xoom (Tegra 2), Nexus 7 (Tegra 3).
    // The issue is that colors aren't accurate enough on Tegra devices. For example, if an 8 bit
    // value of 124 (or 0.486275 here) is entered, we can get a texture value of 123.513725
    // (or 0.484368 here). The following rounding operation prevents these precision issues from
    // affecting the result of the noise by making sure that we only have multiples of 1/255.
    // (Note that 1/255 is about 0.003921569, which is the value used here).
    noiseCode.appendf("\n\t%s = floor(%s * vec2(255.0) + vec2(0.5)) * vec2(0.003921569);",
                      latticeIdx, latticeIdx);
#endif

    // Get (x,y) coordinates with the permutated x
    noiseCode.appendf("\n\tvec4 %s = fract(%s.xyxy + %s.yyww);", bcoords, latticeIdx, floorVal);

    noiseCode.appendf("\n\n\tvec2 %s;", uv);
    // Compute u, at offset (0,0)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.x, %s)", bcoords, chanCoord);
        noiseCode.appendf("\n\tvec4 %s = ", lattice);
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[1], latticeCoords.c_str(),
            kVec2f_GrSLType);
        noiseCode.appendf(".bgra;\n\t%s.x = ", uv);
        noiseCode.appendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    noiseCode.appendf("\n\t%s.x -= 1.0;", fractVal);
    // Compute v, at offset (-1,0)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.y, %s)", bcoords, chanCoord);
        noiseCode.append("\n\tlattice = ");
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[1], latticeCoords.c_str(),
            kVec2f_GrSLType);
        noiseCode.appendf(".bgra;\n\t%s.y = ", uv);
        noiseCode.appendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    // Compute 'a' as a linear interpolation of 'u' and 'v'
    noiseCode.appendf("\n\tvec2 %s;", ab);
    noiseCode.appendf("\n\t%s.x = mix(%s.x, %s.y, %s.x);", ab, uv, uv, noiseSmooth);

    noiseCode.appendf("\n\t%s.y -= 1.0;", fractVal);
    // Compute v, at offset (-1,-1)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.w, %s)", bcoords, chanCoord);
        noiseCode.append("\n\tlattice = ");
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[1], latticeCoords.c_str(),
            kVec2f_GrSLType);
        noiseCode.appendf(".bgra;\n\t%s.y = ", uv);
        noiseCode.appendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    noiseCode.appendf("\n\t%s.x += 1.0;", fractVal);
    // Compute u, at offset (0,-1)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.z, %s)", bcoords, chanCoord);
        noiseCode.append("\n\tlattice = ");
        fsBuilder->appendTextureLookup(&noiseCode, args.fSamplers[1], latticeCoords.c_str(),
            kVec2f_GrSLType);
        noiseCode.appendf(".bgra;\n\t%s.x = ", uv);
        noiseCode.appendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    // Compute 'b' as a linear interpolation of 'u' and 'v'
    noiseCode.appendf("\n\t%s.y = mix(%s.x, %s.y, %s.x);", ab, uv, uv, noiseSmooth);
    // Compute the noise as a linear interpolation of 'a' and 'b'
    noiseCode.appendf("\n\treturn mix(%s.x, %s.y, %s.y);\n", ab, ab, noiseSmooth);

    SkString noiseFuncName;
    if (fStitchTiles) {
        fsBuilder->emitFunction(kFloat_GrSLType,
                                "perlinnoise", SK_ARRAY_COUNT(gPerlinNoiseStitchArgs),
                                gPerlinNoiseStitchArgs, noiseCode.c_str(), &noiseFuncName);
    } else {
        fsBuilder->emitFunction(kFloat_GrSLType,
                                "perlinnoise", SK_ARRAY_COUNT(gPerlinNoiseArgs),
                                gPerlinNoiseArgs, noiseCode.c_str(), &noiseFuncName);
    }

    // There are rounding errors if the floor operation is not performed here
    fsBuilder->codeAppendf("\n\t\tvec2 %s = floor(%s.xy) * %s;",
                           noiseVec, vCoords.c_str(), baseFrequencyUni);

    // Clear the color accumulator
    fsBuilder->codeAppendf("\n\t\t%s = vec4(0.0);", args.fOutputColor);

    if (fStitchTiles) {
        // Set up TurbulenceInitial stitch values.
        fsBuilder->codeAppendf("\n\t\tvec2 %s = %s;", stitchData, stitchDataUni);
    }

    fsBuilder->codeAppendf("\n\t\tfloat %s = 1.0;", ratio);

    // Loop over all octaves
    fsBuilder->codeAppendf("\n\t\tfor (int octave = 0; octave < %d; ++octave) {", fNumOctaves);

    fsBuilder->codeAppendf("\n\t\t\t%s += ", args.fOutputColor);
    if (fType != SkPerlinNoiseShader2::kFractalNoise_Type) {
        fsBuilder->codeAppend("abs(");
    }
    if (fStitchTiles) {
        fsBuilder->codeAppendf(
            "vec4(\n\t\t\t\t%s(%s, %s, %s),\n\t\t\t\t%s(%s, %s, %s),"
                 "\n\t\t\t\t%s(%s, %s, %s),\n\t\t\t\t%s(%s, %s, %s))",
            noiseFuncName.c_str(), chanCoordR, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordG, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordB, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordA, noiseVec, stitchData);
    } else {
        fsBuilder->codeAppendf(
            "vec4(\n\t\t\t\t%s(%s, %s),\n\t\t\t\t%s(%s, %s),"
                 "\n\t\t\t\t%s(%s, %s),\n\t\t\t\t%s(%s, %s))",
            noiseFuncName.c_str(), chanCoordR, noiseVec,
            noiseFuncName.c_str(), chanCoordG, noiseVec,
            noiseFuncName.c_str(), chanCoordB, noiseVec,
            noiseFuncName.c_str(), chanCoordA, noiseVec);
    }
    if (fType != SkPerlinNoiseShader2::kFractalNoise_Type) {
        fsBuilder->codeAppendf(")"); // end of "abs("
    }
    fsBuilder->codeAppendf(" * %s;", ratio);

    fsBuilder->codeAppendf("\n\t\t\t%s *= vec2(2.0);", noiseVec);
    fsBuilder->codeAppendf("\n\t\t\t%s *= 0.5;", ratio);

    if (fStitchTiles) {
        fsBuilder->codeAppendf("\n\t\t\t%s *= vec2(2.0);", stitchData);
    }
    fsBuilder->codeAppend("\n\t\t}"); // end of the for loop on octaves

    if (fType == SkPerlinNoiseShader2::kFractalNoise_Type) {
        // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
        // by fractalNoise and (turbulenceFunctionResult) by turbulence.
        fsBuilder->codeAppendf("\n\t\t%s = %s * vec4(0.5) + vec4(0.5);",
                               args.fOutputColor,args.fOutputColor);
    }

    // Clamp values
    fsBuilder->codeAppendf("\n\t\t%s = clamp(%s, 0.0, 1.0);", args.fOutputColor, args.fOutputColor);

    // Pre-multiply the result
    fsBuilder->codeAppendf("\n\t\t%s = vec4(%s.rgb * %s.aaa, %s.a);\n",
                           args.fOutputColor, args.fOutputColor,
                           args.fOutputColor, args.fOutputColor);
}

void GrGLPerlinNoise2::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
    const GrPerlinNoise2Effect& turbulence = processor.cast<GrPerlinNoise2Effect>();

    uint32_t key = turbulence.numOctaves();

    key = key << 3; // Make room for next 3 bits

    switch (turbulence.type()) {
        case SkPerlinNoiseShader2::kFractalNoise_Type:
            key |= 0x1;
            break;
        case SkPerlinNoiseShader2::kTurbulence_Type:
            key |= 0x2;
            break;
        default:
            // leave key at 0
            break;
    }

    if (turbulence.stitchTiles()) {
        key |= 0x4; // Flip the 3rd bit if tile stitching is on
    }

    b->add32(key);
}

void GrGLPerlinNoise2::onSetData(const GrGLSLProgramDataManager& pdman,
                                const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);

    const GrPerlinNoise2Effect& turbulence = processor.cast<GrPerlinNoise2Effect>();

    const SkVector& baseFrequency = turbulence.baseFrequency();
    pdman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);

    if (turbulence.stitchTiles()) {
        const SkPerlinNoiseShader2::StitchData& stitchData = turbulence.stitchData();
        pdman.set2f(fStitchDataUni, SkIntToScalar(stitchData.fWidth),
                                   SkIntToScalar(stitchData.fHeight));
    }
}

/////////////////////////////////////////////////////////////////////

class GrGLImprovedPerlinNoise : public GrGLSLFragmentProcessor {
public:
    GrGLImprovedPerlinNoise(const GrProcessor&);
    virtual ~GrGLImprovedPerlinNoise() {}

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:

    SkScalar fZ;
    GrGLSLProgramDataManager::UniformHandle fZUni;
    GrGLSLProgramDataManager::UniformHandle fOctavesUni;
    GrGLSLProgramDataManager::UniformHandle fBaseFrequencyUni;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrImprovedPerlinNoiseEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(int octaves, SkScalar z, 
                                       SkPerlinNoiseShader2::PaintingData* paintingData,
                                       GrTexture* permutationsTexture, GrTexture* gradientTexture,
                                       const SkMatrix& matrix) {
        return new GrImprovedPerlinNoiseEffect(octaves, z, paintingData, permutationsTexture,
                                               gradientTexture, matrix);
    }

    virtual ~GrImprovedPerlinNoiseEffect() { delete fPaintingData; }

    const char* name() const override { return "ImprovedPerlinNoise"; }

    const SkVector& baseFrequency() const { return fPaintingData->fBaseFrequency; }
    SkScalar z() const { return fZ; }
    int octaves() const { return fOctaves; }
    const SkMatrix& matrix() const { return fCoordTransform.getMatrix(); }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GrGLImprovedPerlinNoise(*this);
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GrGLImprovedPerlinNoise::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const GrImprovedPerlinNoiseEffect& s = sBase.cast<GrImprovedPerlinNoiseEffect>();
        return fZ == fZ &&
               fPaintingData->fBaseFrequency == s.fPaintingData->fBaseFrequency;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
    }

    GrImprovedPerlinNoiseEffect(int octaves, SkScalar z, 
                                SkPerlinNoiseShader2::PaintingData* paintingData,
                                GrTexture* permutationsTexture, GrTexture* gradientTexture,
                                const SkMatrix& matrix)
      : fOctaves(octaves)
      , fZ(z)
      , fPermutationsAccess(permutationsTexture)
      , fGradientAccess(gradientTexture)
      , fPaintingData(paintingData) {
        this->initClassID<GrImprovedPerlinNoiseEffect>();
        this->addTextureAccess(&fPermutationsAccess);
        this->addTextureAccess(&fGradientAccess);
        fCoordTransform.reset(kLocal_GrCoordSet, matrix);
        this->addCoordTransform(&fCoordTransform);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    GrCoordTransform                  fCoordTransform;
    int                               fOctaves;
    SkScalar                          fZ;
    GrTextureAccess                   fPermutationsAccess;
    GrTextureAccess                   fGradientAccess;
    SkPerlinNoiseShader2::PaintingData *fPaintingData;

private:
    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrImprovedPerlinNoiseEffect);

const GrFragmentProcessor* GrImprovedPerlinNoiseEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar baseFrequencyX = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);
    SkScalar baseFrequencyY = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);
    int numOctaves = d->fRandom->nextRangeU(2, 10);
    SkScalar z = SkIntToScalar(d->fRandom->nextU());

    SkAutoTUnref<SkShader> shader(SkPerlinNoiseShader2::CreateImprovedNoise(baseFrequencyX, 
                                                                           baseFrequencyY, 
                                                                           numOctaves,
                                                                           z));

    GrPaint grPaint;
    return shader->asFragmentProcessor(d->fContext,
                                       GrTest::TestMatrix(d->fRandom), nullptr,
                                       kNone_SkFilterQuality);
}

GrGLImprovedPerlinNoise::GrGLImprovedPerlinNoise(const GrProcessor& processor)
  : fZ(processor.cast<GrImprovedPerlinNoiseEffect>().z()) {
}

void GrGLImprovedPerlinNoise::emitCode(EmitArgs& args) {
    GrGLSLFragmentBuilder* fsBuilder = args.fFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    SkString vCoords = fsBuilder->ensureFSCoords2D(args.fCoords, 0);

    fBaseFrequencyUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "baseFrequency");
    const char* baseFrequencyUni = uniformHandler->getUniformCStr(fBaseFrequencyUni);

    fOctavesUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                             kFloat_GrSLType, kDefault_GrSLPrecision,
                                             "octaves");
    const char* octavesUni = uniformHandler->getUniformCStr(fOctavesUni);

    fZUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                       kFloat_GrSLType, kDefault_GrSLPrecision,
                                       "z");
    const char* zUni = uniformHandler->getUniformCStr(fZUni);

    // fade function
    static const GrGLSLShaderVar fadeArgs[] =  {
        GrGLSLShaderVar("t", kVec3f_GrSLType)
    };
    SkString fadeFuncName;
    fsBuilder->emitFunction(kVec3f_GrSLType, "fade", SK_ARRAY_COUNT(fadeArgs),
                            fadeArgs, 
                            "return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);", 
                            &fadeFuncName);

    // perm function
    static const GrGLSLShaderVar permArgs[] =  {
        GrGLSLShaderVar("x", kFloat_GrSLType)
    };
    SkString permFuncName;
    SkString permCode("return ");
    // FIXME even though I'm creating these textures with kRepeat_TileMode, they're clamped. Not 
    // sure why. Using fract() (here and the next texture lookup) as a workaround.
    fsBuilder->appendTextureLookup(&permCode, args.fSamplers[0], "vec2(fract(x / 256.0), 0.0)", 
                                   kVec2f_GrSLType);
    permCode.append(".r * 255.0;");
    fsBuilder->emitFunction(kFloat_GrSLType, "perm", SK_ARRAY_COUNT(permArgs), permArgs, 
                            permCode.c_str(), &permFuncName);

    // grad function
    static const GrGLSLShaderVar gradArgs[] =  {
        GrGLSLShaderVar("x", kFloat_GrSLType),
        GrGLSLShaderVar("p", kVec3f_GrSLType)
    };
    SkString gradFuncName;
    SkString gradCode("return dot(");
    fsBuilder->appendTextureLookup(&gradCode, args.fSamplers[1], "vec2(fract(x / 16.0), 0.0)", 
                                   kVec2f_GrSLType);
    gradCode.append(".rgb * 255.0 - vec3(1.0), p);");
    fsBuilder->emitFunction(kFloat_GrSLType, "grad", SK_ARRAY_COUNT(gradArgs), gradArgs, 
                            gradCode.c_str(), &gradFuncName);

    // lerp function
    static const GrGLSLShaderVar lerpArgs[] =  {
        GrGLSLShaderVar("a", kFloat_GrSLType),
        GrGLSLShaderVar("b", kFloat_GrSLType),
        GrGLSLShaderVar("w", kFloat_GrSLType)
    };
    SkString lerpFuncName;
    fsBuilder->emitFunction(kFloat_GrSLType, "lerp", SK_ARRAY_COUNT(lerpArgs), lerpArgs, 
                            "return a + w * (b - a);", &lerpFuncName);

    // noise function
    static const GrGLSLShaderVar noiseArgs[] =  {
        GrGLSLShaderVar("p", kVec3f_GrSLType),
    };
    SkString noiseFuncName;
    SkString noiseCode;
    noiseCode.append("vec3 P = mod(floor(p), 256.0);");
    noiseCode.append("p -= floor(p);");
    noiseCode.appendf("vec3 f = %s(p);", fadeFuncName.c_str());
    noiseCode.appendf("float A = %s(P.x) + P.y;", permFuncName.c_str());
    noiseCode.appendf("float AA = %s(A) + P.z;", permFuncName.c_str());
    noiseCode.appendf("float AB = %s(A + 1.0) + P.z;", permFuncName.c_str());
    noiseCode.appendf("float B =  %s(P.x + 1.0) + P.y;", permFuncName.c_str());
    noiseCode.appendf("float BA = %s(B) + P.z;", permFuncName.c_str());
    noiseCode.appendf("float BB = %s(B + 1.0) + P.z;", permFuncName.c_str());
    noiseCode.appendf("float result = %s(", lerpFuncName.c_str());
    noiseCode.appendf("%s(%s(%s(%s(AA), p),", lerpFuncName.c_str(), lerpFuncName.c_str(), 
                      gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.appendf("%s(%s(BA), p + vec3(-1.0, 0.0, 0.0)), f.x),", gradFuncName.c_str(), 
                      permFuncName.c_str());
    noiseCode.appendf("%s(%s(%s(AB), p + vec3(0.0, -1.0, 0.0)),", lerpFuncName.c_str(), 
                      gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.appendf("%s(%s(BB), p + vec3(-1.0, -1.0, 0.0)), f.x), f.y),", 
                      gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.appendf("%s(%s(%s(%s(AA + 1.0), p + vec3(0.0, 0.0, -1.0)),", 
                      lerpFuncName.c_str(), lerpFuncName.c_str(), gradFuncName.c_str(), 
                      permFuncName.c_str());
    noiseCode.appendf("%s(%s(BA + 1.0), p + vec3(-1.0, 0.0, -1.0)), f.x),", 
                      gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.appendf("%s(%s(%s(AB + 1.0), p + vec3(0.0, -1.0, -1.0)),",
                      lerpFuncName.c_str(), gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.appendf("%s(%s(BB + 1.0), p + vec3(-1.0, -1.0, -1.0)), f.x), f.y), f.z);", 
                      gradFuncName.c_str(), permFuncName.c_str());
    noiseCode.append("return result;");
    fsBuilder->emitFunction(kFloat_GrSLType, "noise", SK_ARRAY_COUNT(noiseArgs), noiseArgs, 
                            noiseCode.c_str(), &noiseFuncName);
    
    // noiseOctaves function
    static const GrGLSLShaderVar noiseOctavesArgs[] =  {
        GrGLSLShaderVar("p", kVec3f_GrSLType),
        GrGLSLShaderVar("octaves", kFloat_GrSLType),
    };
    SkString noiseOctavesFuncName;
    SkString noiseOctavesCode;
    noiseOctavesCode.append("float result = 0.0;");
    noiseOctavesCode.append("float ratio = 1.0;");
    noiseOctavesCode.append("for (float i = 0.0; i < octaves; i++) {");
    noiseOctavesCode.appendf("result += %s(p) / ratio;", noiseFuncName.c_str());
    noiseOctavesCode.append("p *= 2.0;");
    noiseOctavesCode.append("ratio *= 2.0;");
    noiseOctavesCode.append("}");
    noiseOctavesCode.append("return (result + 1.0) / 2.0;");
    fsBuilder->emitFunction(kFloat_GrSLType, "noiseOctaves", SK_ARRAY_COUNT(noiseOctavesArgs), 
                            noiseOctavesArgs, noiseOctavesCode.c_str(), &noiseOctavesFuncName);

    fsBuilder->codeAppendf("vec2 coords = %s * %s;", vCoords.c_str(), baseFrequencyUni);
    fsBuilder->codeAppendf("float r = %s(vec3(coords, %s), %s);", noiseOctavesFuncName.c_str(), 
                           zUni, octavesUni);
    fsBuilder->codeAppendf("float g = %s(vec3(coords, %s + 0000.0), %s);", 
                           noiseOctavesFuncName.c_str(), zUni, octavesUni);
    fsBuilder->codeAppendf("float b = %s(vec3(coords, %s + 0000.0), %s);", 
                           noiseOctavesFuncName.c_str(), zUni, octavesUni);
    fsBuilder->codeAppendf("float a = %s(vec3(coords, %s + 0000.0), %s);", 
                           noiseOctavesFuncName.c_str(), zUni, octavesUni);
    fsBuilder->codeAppendf("%s = vec4(r, g, b, a);", args.fOutputColor);

    // Clamp values
    fsBuilder->codeAppendf("%s = clamp(%s, 0.0, 1.0);", args.fOutputColor, args.fOutputColor);

    // Pre-multiply the result
    fsBuilder->codeAppendf("\n\t\t%s = vec4(%s.rgb * %s.aaa, %s.a);\n",
                           args.fOutputColor, args.fOutputColor,
                           args.fOutputColor, args.fOutputColor);
}

void GrGLImprovedPerlinNoise::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                                     GrProcessorKeyBuilder* b) {
}

void GrGLImprovedPerlinNoise::onSetData(const GrGLSLProgramDataManager& pdman,
                                const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);

    const GrImprovedPerlinNoiseEffect& noise = processor.cast<GrImprovedPerlinNoiseEffect>();

    const SkVector& baseFrequency = noise.baseFrequency();
    pdman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);

    pdman.set1f(fOctavesUni, SkIntToScalar(noise.octaves()));

    pdman.set1f(fZUni, noise.z());
}

/////////////////////////////////////////////////////////////////////
const GrFragmentProcessor* SkPerlinNoiseShader2::asFragmentProcessor(
                                                    GrContext* context,
                                                    const SkMatrix& viewM,
                                                    const SkMatrix* externalLocalMatrix,
                                                    SkFilterQuality) const {
    SkASSERT(context);

    SkMatrix localMatrix = this->getLocalMatrix();
    if (externalLocalMatrix) {
        localMatrix.preConcat(*externalLocalMatrix);
    }

    SkMatrix matrix = viewM;
    matrix.preConcat(localMatrix);

    // Either we don't stitch tiles, either we have a valid tile size
    SkASSERT(!fStitchTiles || !fTileSize.isEmpty());

    SkPerlinNoiseShader2::PaintingData* paintingData =
            new PaintingData(fTileSize, fSeed, fBaseFrequencyX, fBaseFrequencyY, matrix);

    SkMatrix m = viewM;
    m.setTranslateX(-localMatrix.getTranslateX() + SK_Scalar1);
    m.setTranslateY(-localMatrix.getTranslateY() + SK_Scalar1);

    if (fType == kImprovedNoise_Type) {
        GrTextureParams textureParams(SkShader::TileMode::kRepeat_TileMode, 
                                      GrTextureParams::FilterMode::kNone_FilterMode);
        SkAutoTUnref<GrTexture> permutationsTexture(
            GrRefCachedBitmapTexture(context, paintingData->getImprovedPermutationsBitmap(),
                                     textureParams));
        SkAutoTUnref<GrTexture> gradientTexture(
            GrRefCachedBitmapTexture(context, paintingData->getGradientBitmap(),
                                     textureParams));
        return GrImprovedPerlinNoiseEffect::Create(fNumOctaves, fSeed, paintingData, 
                                                   permutationsTexture, gradientTexture, m);
    }

    if (0 == fNumOctaves) {
        if (kFractalNoise_Type == fType) {
            // Extract the incoming alpha and emit rgba = (a/4, a/4, a/4, a/2)
            SkAutoTUnref<const GrFragmentProcessor> inner(
                GrConstColorProcessor::Create(0x80404040,
                                              GrConstColorProcessor::kModulateRGBA_InputMode));
            return GrFragmentProcessor::MulOutputByInputAlpha(inner);
        }
        // Emit zero.
        return GrConstColorProcessor::Create(0x0, GrConstColorProcessor::kIgnore_InputMode);
    }

    SkAutoTUnref<GrTexture> permutationsTexture(
        GrRefCachedBitmapTexture(context, paintingData->getPermutationsBitmap(),
                                 GrTextureParams::ClampNoFilter()));
    SkAutoTUnref<GrTexture> noiseTexture(
        GrRefCachedBitmapTexture(context, paintingData->getNoiseBitmap(),
                                 GrTextureParams::ClampNoFilter()));

    if ((permutationsTexture) && (noiseTexture)) {
        SkAutoTUnref<GrFragmentProcessor> inner(
            GrPerlinNoise2Effect::Create(fType,
                                        fNumOctaves,
                                        fStitchTiles,
                                        paintingData,
                                        permutationsTexture, noiseTexture,
                                        m));
        return GrFragmentProcessor::MulOutputByInputAlpha(inner);
    }
    delete paintingData;
    return nullptr;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkPerlinNoiseShader2::toString(SkString* str) const {
    str->append("SkPerlinNoiseShader2: (");

    str->append("type: ");
    switch (fType) {
        case kFractalNoise_Type:
            str->append("\"fractal noise\"");
            break;
        case kTurbulence_Type:
            str->append("\"turbulence\"");
            break;
        default:
            str->append("\"unknown\"");
            break;
    }
    str->append(" base frequency: (");
    str->appendScalar(fBaseFrequencyX);
    str->append(", ");
    str->appendScalar(fBaseFrequencyY);
    str->append(") number of octaves: ");
    str->appendS32(fNumOctaves);
    str->append(" seed: ");
    str->appendScalar(fSeed);
    str->append(" stitch tiles: ");
    str->append(fStitchTiles ? "true " : "false ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
