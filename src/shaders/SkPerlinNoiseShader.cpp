/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkPerlinNoiseShader.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkTPin.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#endif

static const int kBlockSize = 256;
static const int kBlockMask = kBlockSize - 1;
static const int kPerlinNoise = 4096;
static const int kRandMaximum = SK_MaxS32; // 2**31 - 1

class SkPerlinNoiseShaderImpl : public SkShaderBase {
public:
    struct StitchData {
        StitchData()
          : fWidth(0)
          , fWrapX(0)
          , fHeight(0)
          , fWrapY(0)
        {}

        StitchData(SkScalar w, SkScalar h)
          : fWidth(std::min(SkScalarRoundToInt(w), SK_MaxS32 - kPerlinNoise))
          , fWrapX(kPerlinNoise + fWidth)
          , fHeight(std::min(SkScalarRoundToInt(h), SK_MaxS32 - kPerlinNoise))
          , fWrapY(kPerlinNoise + fHeight) {}

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

    struct PaintingData {
        PaintingData(const SkISize& tileSize, SkScalar seed,
                     SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                     const SkMatrix& matrix)
        {
            SkVector tileVec;
            matrix.mapVector(SkIntToScalar(tileSize.fWidth), SkIntToScalar(tileSize.fHeight),
                             &tileVec);

            SkSize scale;
            if (!matrix.decomposeScale(&scale, nullptr)) {
                scale.set(SK_ScalarNearlyZero, SK_ScalarNearlyZero);
            }
            fBaseFrequency.set(baseFrequencyX * SkScalarInvert(scale.width()),
                               baseFrequencyY * SkScalarInvert(scale.height()));
            fTileSize.set(SkScalarRoundToInt(tileVec.fX), SkScalarRoundToInt(tileVec.fY));
            this->init(seed);
            if (!fTileSize.isEmpty()) {
                this->stitch();
            }

    #if SK_SUPPORT_GPU
            SkImageInfo info = SkImageInfo::MakeA8(kBlockSize, 1);
            fPermutationsBitmap.installPixels(info, fLatticeSelector, info.minRowBytes());
            fPermutationsBitmap.setImmutable();

            info = SkImageInfo::Make(kBlockSize, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            fNoiseBitmap.installPixels(info, fNoise[0][0], info.minRowBytes());
            fNoiseBitmap.setImmutable();
    #endif
        }

    #if SK_SUPPORT_GPU
        PaintingData(const PaintingData& that)
                : fSeed(that.fSeed)
                , fTileSize(that.fTileSize)
                , fBaseFrequency(that.fBaseFrequency)
                , fStitchDataInit(that.fStitchDataInit)
                , fPermutationsBitmap(that.fPermutationsBitmap)
                , fNoiseBitmap(that.fNoiseBitmap) {
            memcpy(fLatticeSelector, that.fLatticeSelector, sizeof(fLatticeSelector));
            memcpy(fNoise, that.fNoise, sizeof(fNoise));
            memcpy(fGradient, that.fGradient, sizeof(fGradient));
        }
    #endif

        int         fSeed;
        uint8_t     fLatticeSelector[kBlockSize];
        uint16_t    fNoise[4][kBlockSize][2];
        SkPoint     fGradient[4][kBlockSize];
        SkISize     fTileSize;
        SkVector    fBaseFrequency;
        StitchData  fStitchDataInit;

    private:

    #if SK_SUPPORT_GPU
        SkBitmap fPermutationsBitmap;
        SkBitmap fNoiseBitmap;
    #endif

        inline int random()  {
            // See https://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
            // m = kRandMaximum, 2**31 - 1 (2147483647)
            static constexpr int kRandAmplitude = 16807; // 7**5; primitive root of m
            static constexpr int kRandQ = 127773; // m / a
            static constexpr int kRandR = 2836; // m % a

            int result = kRandAmplitude * (fSeed % kRandQ) - kRandR * (fSeed / kRandQ);
            if (result <= 0) {
                result += kRandMaximum;
            }
            fSeed = result;
            return result;
        }

        // Only called once. Could be part of the constructor.
        void init(SkScalar seed)
        {
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

            // Compute gradients from permutated noise data
            static constexpr SkScalar kInvBlockSizef = 1.0 / SkIntToScalar(kBlockSize);
            for (int channel = 0; channel < 4; ++channel) {
                for (int i = 0; i < kBlockSize; ++i) {
                    fGradient[channel][i] = SkPoint::Make(
                        (fNoise[channel][i][0] - kBlockSize) * kInvBlockSizef,
                        (fNoise[channel][i][1] - kBlockSize) * kInvBlockSizef);
                    fGradient[channel][i].normalize();
                    // Put the normalized gradient back into the noise data
                    fNoise[channel][i][0] =
                            SkScalarRoundToInt((fGradient[channel][i].fX + 1) * kHalfMax16bits);
                    fNoise[channel][i][1] =
                            SkScalarRoundToInt((fGradient[channel][i].fY + 1) * kHalfMax16bits);
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
                // lowFrequencx can be 0 if fBaseFrequency.fX is very small.
                if (sk_ieee_float_divide(fBaseFrequency.fX, lowFrequencx) < highFrequencx / fBaseFrequency.fX) {
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
                if (sk_ieee_float_divide(fBaseFrequency.fY, lowFrequency) < highFrequency / fBaseFrequency.fY) {
                    fBaseFrequency.fY = lowFrequency;
                } else {
                    fBaseFrequency.fY = highFrequency;
                }
            }
            // Set up TurbulenceInitial stitch values.
            fStitchDataInit = StitchData(tileWidth * fBaseFrequency.fX,
                                         tileHeight * fBaseFrequency.fY);
        }

    public:

#if SK_SUPPORT_GPU
        const SkBitmap& getPermutationsBitmap() const { return fPermutationsBitmap; }

        const SkBitmap& getNoiseBitmap() const { return fNoiseBitmap; }
#endif
    };

    /**
     *  About the noise types : the difference between the first 2 is just minor tweaks to the
     *  algorithm, they're not 2 entirely different noises. The output looks different, but once the
     *  noise is generated in the [1, -1] range, the output is brought back in the [0, 1] range by
     *  doing :
     *  kFractalNoise_Type : noise * 0.5 + 0.5
     *  kTurbulence_Type   : abs(noise)
     *  Very little differences between the 2 types, although you can tell the difference visually.
     */
    enum Type {
        kFractalNoise_Type,
        kTurbulence_Type,
        kLast_Type = kTurbulence_Type
    };

    static const int kMaxOctaves = 255; // numOctaves must be <= 0 and <= kMaxOctaves

    SkPerlinNoiseShaderImpl(SkPerlinNoiseShaderImpl::Type type, SkScalar baseFrequencyX,
                      SkScalar baseFrequencyY, int numOctaves, SkScalar seed,
                      const SkISize* tileSize);

    class PerlinNoiseShaderContext : public Context {
    public:
        PerlinNoiseShaderContext(const SkPerlinNoiseShaderImpl& shader, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    private:
        SkPMColor shade(const SkPoint& point, StitchData& stitchData) const;
        SkScalar calculateTurbulenceValueForPoint(
                                                  int channel,
                                                  StitchData& stitchData, const SkPoint& point) const;
        SkScalar noise2D(int channel,
                         const StitchData& stitchData, const SkPoint& noiseVector) const;

        SkMatrix     fMatrix;
        PaintingData fPaintingData;

        using INHERITED = Context;
    };

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    skvm::Color onProgram(skvm::Builder*,
                          skvm::Coord, skvm::Coord, skvm::Color,
                          const SkMatrixProvider&, const SkMatrix*,
                          SkFilterQuality, const SkColorInfo&,
                          skvm::Uniforms*, SkArenaAlloc*) const override {
        // TODO?
        return {};
    }

protected:
    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkPerlinNoiseShaderImpl)

    const SkPerlinNoiseShaderImpl::Type fType;
    const SkScalar                  fBaseFrequencyX;
    const SkScalar                  fBaseFrequencyY;
    const int                       fNumOctaves;
    const SkScalar                  fSeed;
    const SkISize                   fTileSize;
    const bool                      fStitchTiles;

    friend class ::SkPerlinNoiseShader;

    using INHERITED = SkShaderBase;
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
    return t * t * (3 - 2 * t);
}

} // end namespace

SkPerlinNoiseShaderImpl::SkPerlinNoiseShaderImpl(SkPerlinNoiseShaderImpl::Type type,
                                                 SkScalar baseFrequencyX,
                                                 SkScalar baseFrequencyY,
                                                 int numOctaves,
                                                 SkScalar seed,
                                                 const SkISize* tileSize)
  : fType(type)
  , fBaseFrequencyX(baseFrequencyX)
  , fBaseFrequencyY(baseFrequencyY)
  , fNumOctaves(numOctaves > kMaxOctaves ? kMaxOctaves : numOctaves/*[0,255] octaves allowed*/)
  , fSeed(seed)
  , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
  , fStitchTiles(!fTileSize.isEmpty())
{
    SkASSERT(numOctaves >= 0 && numOctaves <= kMaxOctaves);
    SkASSERT(fBaseFrequencyX >= 0);
    SkASSERT(fBaseFrequencyY >= 0);
}

sk_sp<SkFlattenable> SkPerlinNoiseShaderImpl::CreateProc(SkReadBuffer& buffer) {
    Type type = buffer.read32LE(kLast_Type);

    SkScalar freqX = buffer.readScalar();
    SkScalar freqY = buffer.readScalar();
    int octaves = buffer.read32LE<int>(kMaxOctaves);

    SkScalar seed = buffer.readScalar();
    SkISize tileSize;
    tileSize.fWidth = buffer.readInt();
    tileSize.fHeight = buffer.readInt();

    switch (type) {
        case kFractalNoise_Type:
            return SkPerlinNoiseShader::MakeFractalNoise(freqX, freqY, octaves, seed, &tileSize);
        case kTurbulence_Type:
            return SkPerlinNoiseShader::MakeTurbulence(freqX, freqY, octaves, seed, &tileSize);
        default:
            // Really shouldn't get here b.c. of earlier check on type
            buffer.validate(false);
            return nullptr;
    }
}

void SkPerlinNoiseShaderImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int) fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

SkScalar SkPerlinNoiseShaderImpl::PerlinNoiseShaderContext::noise2D(
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
    const SkPerlinNoiseShaderImpl& perlinNoiseShader = static_cast<const SkPerlinNoiseShaderImpl&>(fShader);
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
    int i = fPaintingData.fLatticeSelector[noiseX.noisePositionIntegerValue];
    int j = fPaintingData.fLatticeSelector[noiseX.nextNoisePositionIntegerValue];
    int b00 = (i + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b10 = (j + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b01 = (i + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    int b11 = (j + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    SkScalar sx = smoothCurve(noiseX.noisePositionFractionValue);
    SkScalar sy = smoothCurve(noiseY.noisePositionFractionValue);

    if (sx < 0 || sy < 0 || sx > 1 || sy > 1) {
        return 0;  // Check for pathological inputs.
    }

    // This is taken 1:1 from SVG spec: http://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
    SkPoint fractionValue = SkPoint::Make(noiseX.noisePositionFractionValue,
                                          noiseY.noisePositionFractionValue); // Offset (0,0)
    u = fPaintingData.fGradient[channel][b00].dot(fractionValue);
    fractionValue.fX -= SK_Scalar1; // Offset (-1,0)
    v = fPaintingData.fGradient[channel][b10].dot(fractionValue);
    SkScalar a = SkScalarInterp(u, v, sx);
    fractionValue.fY -= SK_Scalar1; // Offset (-1,-1)
    v = fPaintingData.fGradient[channel][b11].dot(fractionValue);
    fractionValue.fX = noiseX.noisePositionFractionValue; // Offset (0,-1)
    u = fPaintingData.fGradient[channel][b01].dot(fractionValue);
    SkScalar b = SkScalarInterp(u, v, sx);
    return SkScalarInterp(a, b, sy);
}

SkScalar SkPerlinNoiseShaderImpl::PerlinNoiseShaderContext::calculateTurbulenceValueForPoint(
        int channel, StitchData& stitchData, const SkPoint& point) const {
    const SkPerlinNoiseShaderImpl& perlinNoiseShader = static_cast<const SkPerlinNoiseShaderImpl&>(fShader);
    if (perlinNoiseShader.fStitchTiles) {
        // Set up TurbulenceInitial stitch values.
        stitchData = fPaintingData.fStitchDataInit;
    }
    SkScalar turbulenceFunctionResult = 0;
    SkPoint noiseVector(SkPoint::Make(point.x() * fPaintingData.fBaseFrequency.fX,
                                      point.y() * fPaintingData.fBaseFrequency.fY));
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
            stitchData = StitchData(SkIntToScalar(stitchData.fWidth) * 2,
                                    SkIntToScalar(stitchData.fHeight) * 2);
        }
    }

    // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
    // by fractalNoise and (turbulenceFunctionResult) by turbulence.
    if (perlinNoiseShader.fType == kFractalNoise_Type) {
        turbulenceFunctionResult = SkScalarHalf(turbulenceFunctionResult + 1);
    }

    if (channel == 3) { // Scale alpha by paint value
        turbulenceFunctionResult *= SkIntToScalar(getPaintAlpha()) / 255;
    }

    // Clamp result
    return SkTPin(turbulenceFunctionResult, 0.0f, SK_Scalar1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SkPMColor SkPerlinNoiseShaderImpl::PerlinNoiseShaderContext::shade(
        const SkPoint& point, StitchData& stitchData) const {
    SkPoint newPoint;
    fMatrix.mapPoints(&newPoint, &point, 1);
    newPoint.fX = SkScalarRoundToScalar(newPoint.fX);
    newPoint.fY = SkScalarRoundToScalar(newPoint.fY);

    U8CPU rgba[4];
    for (int channel = 3; channel >= 0; --channel) {
        SkScalar value;
        value = calculateTurbulenceValueForPoint(channel, stitchData, newPoint);
        rgba[channel] = SkScalarFloorToInt(255 * value);
    }
    return SkPreMultiplyARGB(rgba[3], rgba[0], rgba[1], rgba[2]);
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkPerlinNoiseShaderImpl::onMakeContext(const ContextRec& rec,
                                                              SkArenaAlloc* alloc) const {
    // should we pay attention to rec's device-colorspace?
    return alloc->make<PerlinNoiseShaderContext>(*this, rec);
}
#endif

static inline SkMatrix total_matrix(const SkShaderBase::ContextRec& rec,
                                    const SkShaderBase& shader) {
    SkMatrix matrix = SkMatrix::Concat(*rec.fMatrix, shader.getLocalMatrix());
    if (rec.fLocalMatrix) {
        matrix.preConcat(*rec.fLocalMatrix);
    }

    return matrix;
}

SkPerlinNoiseShaderImpl::PerlinNoiseShaderContext::PerlinNoiseShaderContext(
        const SkPerlinNoiseShaderImpl& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
    , fMatrix(total_matrix(rec, shader)) // used for temp storage, adjusted below
    , fPaintingData(shader.fTileSize, shader.fSeed, shader.fBaseFrequencyX,
                    shader.fBaseFrequencyY, fMatrix)
{
    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the setData() function.
    fMatrix.setTranslate(-fMatrix.getTranslateX() + SK_Scalar1,
                         -fMatrix.getTranslateY() + SK_Scalar1);
}

void SkPerlinNoiseShaderImpl::PerlinNoiseShaderContext::shadeSpan(
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

class GrGLPerlinNoise : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fStitchDataUni;
    GrGLSLProgramDataManager::UniformHandle fBaseFrequencyUni;

    using INHERITED = GrGLSLFragmentProcessor;
};

/////////////////////////////////////////////////////////////////////

class GrPerlinNoise2Effect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            SkPerlinNoiseShaderImpl::Type type,
            int numOctaves,
            bool stitchTiles,
            std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> paintingData,
            GrSurfaceProxyView permutationsView,
            GrSurfaceProxyView noiseView,
            const SkMatrix& matrix,
            const GrCaps& caps) {
        static constexpr GrSamplerState kRepeatXSampler = {GrSamplerState::WrapMode::kRepeat,
                                                           GrSamplerState::WrapMode::kClamp,
                                                           GrSamplerState::Filter::kNearest};
        auto permutationsFP =
                GrTextureEffect::Make(std::move(permutationsView), kPremul_SkAlphaType,
                                      SkMatrix::I(), kRepeatXSampler, caps);
        auto noiseFP = GrTextureEffect::Make(std::move(noiseView), kPremul_SkAlphaType,
                                             SkMatrix::I(), kRepeatXSampler, caps);

        return GrMatrixEffect::Make(matrix, std::unique_ptr<GrFragmentProcessor>(
                new GrPerlinNoise2Effect(type, numOctaves, stitchTiles, std::move(paintingData),
                                         std::move(permutationsFP), std::move(noiseFP))));
    }

    const char* name() const override { return "PerlinNoise"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrPerlinNoise2Effect(*this));
    }

    const SkPerlinNoiseShaderImpl::StitchData& stitchData() const { return fPaintingData->fStitchDataInit; }

    SkPerlinNoiseShaderImpl::Type type() const { return fType; }
    bool stitchTiles() const { return fStitchTiles; }
    const SkVector& baseFrequency() const { return fPaintingData->fBaseFrequency; }
    int numOctaves() const { return fNumOctaves; }

private:
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override {
        return std::make_unique<GrGLPerlinNoise>();
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GrGLPerlinNoise::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const GrPerlinNoise2Effect& s = sBase.cast<GrPerlinNoise2Effect>();
        return fType == s.fType &&
               fPaintingData->fBaseFrequency == s.fPaintingData->fBaseFrequency &&
               fNumOctaves == s.fNumOctaves &&
               fStitchTiles == s.fStitchTiles &&
               fPaintingData->fStitchDataInit == s.fPaintingData->fStitchDataInit;
    }

    GrPerlinNoise2Effect(SkPerlinNoiseShaderImpl::Type type,
                         int numOctaves,
                         bool stitchTiles,
                         std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> paintingData,
                         std::unique_ptr<GrFragmentProcessor> permutationsFP,
                         std::unique_ptr<GrFragmentProcessor> noiseFP)
            : INHERITED(kGrPerlinNoise2Effect_ClassID, kNone_OptimizationFlags)
            , fType(type)
            , fNumOctaves(numOctaves)
            , fStitchTiles(stitchTiles)
            , fPaintingData(std::move(paintingData)) {
        this->registerChild(std::move(permutationsFP), SkSL::SampleUsage::Explicit());
        this->registerChild(std::move(noiseFP), SkSL::SampleUsage::Explicit());
        this->setUsesSampleCoordsDirectly();
    }

    GrPerlinNoise2Effect(const GrPerlinNoise2Effect& that)
            : INHERITED(kGrPerlinNoise2Effect_ClassID, kNone_OptimizationFlags)
            , fType(that.fType)
            , fNumOctaves(that.fNumOctaves)
            , fStitchTiles(that.fStitchTiles)
            , fPaintingData(new SkPerlinNoiseShaderImpl::PaintingData(*that.fPaintingData)) {
        this->cloneAndRegisterAllChildProcessors(that);
        this->setUsesSampleCoordsDirectly();
    }


    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkPerlinNoiseShaderImpl::Type       fType;
    int                                 fNumOctaves;
    bool                                fStitchTiles;

    std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> fPaintingData;

    using INHERITED = GrFragmentProcessor;
};

/////////////////////////////////////////////////////////////////////
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrPerlinNoise2Effect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrPerlinNoise2Effect::TestCreate(GrProcessorTestData* d) {
    int      numOctaves = d->fRandom->nextRangeU(2, 10);
    bool     stitchTiles = d->fRandom->nextBool();
    SkScalar seed = SkIntToScalar(d->fRandom->nextU());
    SkISize  tileSize = SkISize::Make(d->fRandom->nextRangeU(4, 4096),
                                      d->fRandom->nextRangeU(4, 4096));
    SkScalar baseFrequencyX = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);
    SkScalar baseFrequencyY = d->fRandom->nextRangeScalar(0.01f,
                                                          0.99f);

    sk_sp<SkShader> shader(d->fRandom->nextBool() ?
        SkPerlinNoiseShader::MakeFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                               stitchTiles ? &tileSize : nullptr) :
        SkPerlinNoiseShader::MakeTurbulence(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                             stitchTiles ? &tileSize : nullptr));

    GrTest::TestAsFPArgs asFPArgs(d);
    return as_SB(shader)->asFragmentProcessor(asFPArgs.args());
}
#endif

void GrGLPerlinNoise::emitCode(EmitArgs& args) {
    const GrPerlinNoise2Effect& pne = args.fFp.cast<GrPerlinNoise2Effect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    fBaseFrequencyUni = uniformHandler->addUniform(&pne, kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                   "baseFrequency");
    const char* baseFrequencyUni = uniformHandler->getUniformCStr(fBaseFrequencyUni);

    const char* stitchDataUni = nullptr;
    if (pne.stitchTiles()) {
        fStitchDataUni = uniformHandler->addUniform(&pne, kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                    "stitchData");
        stitchDataUni = uniformHandler->getUniformCStr(fStitchDataUni);
    }

    // Add noise function
    const GrShaderVar gPerlinNoiseArgs[] = {{"chanCoord", kHalf_GrSLType },
                                            {"noiseVec ", kHalf2_GrSLType}};

    const GrShaderVar gPerlinNoiseStitchArgs[] = {{"chanCoord" , kHalf_GrSLType },
                                                  {"noiseVec"  , kHalf2_GrSLType},
                                                  {"stitchData", kHalf2_GrSLType}};

    SkString noiseCode;

    noiseCode.append(
            R"(half4 floorVal;
               floorVal.xy = floor(noiseVec);
               floorVal.zw = floorVal.xy + half2(1);
               half2 fractVal = fract(noiseVec);
               // smooth curve : t^2*(3 - 2*t)
               half2 noiseSmooth = fractVal*fractVal*(half2(3) - 2*fractVal);)");

    // Adjust frequencies if we're stitching tiles
    if (pne.stitchTiles()) {
        noiseCode.append(
             R"(if (floorVal.x >= stitchData.x) { floorVal.x -= stitchData.x; };
                if (floorVal.y >= stitchData.y) { floorVal.y -= stitchData.y; };
                if (floorVal.z >= stitchData.x) { floorVal.z -= stitchData.x; };
                if (floorVal.w >= stitchData.y) { floorVal.w -= stitchData.y; };)");
    }

    // NOTE: We need to explicitly pass half4(1) as input color here, because the helper function
    // can't see fInputColor (which is "_input" in the FP's outer function). skbug.com/10506
    SkString sampleX = this->invokeChild(0, "half4(1)", args, "half2(floorVal.x, 0.5)");
    SkString sampleY = this->invokeChild(0, "half4(1)", args, "half2(floorVal.z, 0.5)");
    noiseCode.appendf("half2 latticeIdx = half2(%s.a, %s.a);", sampleX.c_str(), sampleY.c_str());

#if defined(SK_BUILD_FOR_ANDROID)
    // Android rounding for Tegra devices, like, for example: Xoom (Tegra 2), Nexus 7 (Tegra 3).
    // The issue is that colors aren't accurate enough on Tegra devices. For example, if an 8 bit
    // value of 124 (or 0.486275 here) is entered, we can get a texture value of 123.513725
    // (or 0.484368 here). The following rounding operation prevents these precision issues from
    // affecting the result of the noise by making sure that we only have multiples of 1/255.
    // (Note that 1/255 is about 0.003921569, which is the value used here).
    noiseCode.append(
            "latticeIdx = floor(latticeIdx * half2(255.0) + half2(0.5)) * half2(0.003921569);");
#endif

    // Get (x,y) coordinates with the permutated x
    noiseCode.append("half4 bcoords = 256*latticeIdx.xyxy + floorVal.yyww;");

    noiseCode.append("half2 uv;");

    // This is the math to convert the two 16bit integer packed into rgba 8 bit input into a
    // [-1,1] vector and perform a dot product between that vector and the provided vector.
    // Save it as a string because we will repeat it 4x.
    static constexpr const char* inc8bit = "0.00390625";  // 1.0 / 256.0
    SkString dotLattice =
            SkStringPrintf("dot((lattice.ga + lattice.rb*%s)*2 - half2(1), fractVal)", inc8bit);

    SkString sampleA = this->invokeChild(1, "half4(1)", args, "half2(bcoords.x, chanCoord)");
    SkString sampleB = this->invokeChild(1, "half4(1)", args, "half2(bcoords.y, chanCoord)");
    SkString sampleC = this->invokeChild(1, "half4(1)", args, "half2(bcoords.w, chanCoord)");
    SkString sampleD = this->invokeChild(1, "half4(1)", args, "half2(bcoords.z, chanCoord)");

    // Compute u, at offset (0,0)
    noiseCode.appendf("half4 lattice = %s;", sampleA.c_str());
    noiseCode.appendf("uv.x = %s;", dotLattice.c_str());

    // Compute v, at offset (-1,0)
    noiseCode.append("fractVal.x -= 1.0;");
    noiseCode.appendf("lattice = %s;", sampleB.c_str());
    noiseCode.appendf("uv.y = %s;", dotLattice.c_str());

    // Compute 'a' as a linear interpolation of 'u' and 'v'
    noiseCode.append("half2 ab;");
    noiseCode.append("ab.x = mix(uv.x, uv.y, noiseSmooth.x);");

    // Compute v, at offset (-1,-1)
    noiseCode.append("fractVal.y -= 1.0;");
    noiseCode.appendf("lattice = %s;", sampleC.c_str());
    noiseCode.appendf("uv.y = %s;", dotLattice.c_str());

    // Compute u, at offset (0,-1)
    noiseCode.append("fractVal.x += 1.0;");
    noiseCode.appendf("lattice = %s;", sampleD.c_str());
    noiseCode.appendf("uv.x = %s;", dotLattice.c_str());

    // Compute 'b' as a linear interpolation of 'u' and 'v'
    noiseCode.append("ab.y = mix(uv.x, uv.y, noiseSmooth.x);");
    // Compute the noise as a linear interpolation of 'a' and 'b'
    noiseCode.append("return mix(ab.x, ab.y, noiseSmooth.y);");

    SkString noiseFuncName = fragBuilder->getMangledFunctionName("noiseFuncName");
    if (pne.stitchTiles()) {
        fragBuilder->emitFunction(kHalf_GrSLType, noiseFuncName.c_str(),
                                  {gPerlinNoiseStitchArgs, SK_ARRAY_COUNT(gPerlinNoiseStitchArgs)},
                                  noiseCode.c_str());
    } else {
        fragBuilder->emitFunction(kHalf_GrSLType, noiseFuncName.c_str(),
                                  {gPerlinNoiseArgs, SK_ARRAY_COUNT(gPerlinNoiseArgs)},
                                  noiseCode.c_str());
    }

    // There are rounding errors if the floor operation is not performed here
    fragBuilder->codeAppendf("half2 noiseVec = half2(floor(%s.xy) * %s);",
                             args.fSampleCoord, baseFrequencyUni);

    // Clear the color accumulator
    fragBuilder->codeAppendf("half4 color = half4(0);");

    if (pne.stitchTiles()) {
        // Set up TurbulenceInitial stitch values.
        fragBuilder->codeAppendf("half2 stitchData = %s;", stitchDataUni);
    }

    fragBuilder->codeAppendf("half ratio = 1.0;");

    // Loop over all octaves
    fragBuilder->codeAppendf("for (int octave = 0; octave < %d; ++octave) {", pne.numOctaves());
    fragBuilder->codeAppendf("    color += ");
    if (pne.type() != SkPerlinNoiseShaderImpl::kFractalNoise_Type) {
        fragBuilder->codeAppend("abs(");
    }

    // There are 4 lines, put y coords at center of each.
    static constexpr const char* chanCoordR = "0.5";
    static constexpr const char* chanCoordG = "1.5";
    static constexpr const char* chanCoordB = "2.5";
    static constexpr const char* chanCoordA = "3.5";
    if (pne.stitchTiles()) {
        fragBuilder->codeAppendf(R"(
           half4(%s(%s, noiseVec, stitchData), %s(%s, noiseVec, stitchData),
                 %s(%s, noiseVec, stitchData), %s(%s, noiseVec, stitchData)))",
            noiseFuncName.c_str(), chanCoordR,
            noiseFuncName.c_str(), chanCoordG,
            noiseFuncName.c_str(), chanCoordB,
            noiseFuncName.c_str(), chanCoordA);
    } else {
        fragBuilder->codeAppendf(R"(
            half4(%s(%s, noiseVec), %s(%s, noiseVec),
                  %s(%s, noiseVec), %s(%s, noiseVec)))",
            noiseFuncName.c_str(), chanCoordR,
            noiseFuncName.c_str(), chanCoordG,
            noiseFuncName.c_str(), chanCoordB,
            noiseFuncName.c_str(), chanCoordA);
    }
    if (pne.type() != SkPerlinNoiseShaderImpl::kFractalNoise_Type) {
        fragBuilder->codeAppend(")");  // end of "abs("
    }
    fragBuilder->codeAppend(" * ratio;");

    fragBuilder->codeAppend(R"(noiseVec *= half2(2.0);
                               ratio *= 0.5;)");

    if (pne.stitchTiles()) {
        fragBuilder->codeAppend("stitchData *= half2(2.0);");
    }
    fragBuilder->codeAppend("}");  // end of the for loop on octaves

    if (pne.type() == SkPerlinNoiseShaderImpl::kFractalNoise_Type) {
        // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
        // by fractalNoise and (turbulenceFunctionResult) by turbulence.
        fragBuilder->codeAppendf("color = color * half4(0.5) + half4(0.5);");
    }

    // Clamp values
    fragBuilder->codeAppendf("color = saturate(color);");

    // Pre-multiply the result
    fragBuilder->codeAppendf("return half4(color.rgb * color.aaa, color.a);");
}

void GrGLPerlinNoise::GenKey(const GrProcessor& processor, const GrShaderCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrPerlinNoise2Effect& turbulence = processor.cast<GrPerlinNoise2Effect>();

    uint32_t key = turbulence.numOctaves();

    key = key << 3; // Make room for next 3 bits

    switch (turbulence.type()) {
        case SkPerlinNoiseShaderImpl::kFractalNoise_Type:
            key |= 0x1;
            break;
        case SkPerlinNoiseShaderImpl::kTurbulence_Type:
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

void GrGLPerlinNoise::onSetData(const GrGLSLProgramDataManager& pdman,
                                const GrFragmentProcessor& processor) {
    INHERITED::onSetData(pdman, processor);

    const GrPerlinNoise2Effect& turbulence = processor.cast<GrPerlinNoise2Effect>();

    const SkVector& baseFrequency = turbulence.baseFrequency();
    pdman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);

    if (turbulence.stitchTiles()) {
        const SkPerlinNoiseShaderImpl::StitchData& stitchData = turbulence.stitchData();
        pdman.set2f(fStitchDataUni, SkIntToScalar(stitchData.fWidth),
                                    SkIntToScalar(stitchData.fHeight));
    }
}

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> SkPerlinNoiseShaderImpl::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkASSERT(args.fContext);

    const auto localMatrix = this->totalLocalMatrix(args.fPreLocalMatrix);
    const auto paintMatrix = SkMatrix::Concat(args.fMatrixProvider.localToDevice(), *localMatrix);

    // Either we don't stitch tiles, either we have a valid tile size
    SkASSERT(!fStitchTiles || !fTileSize.isEmpty());

    std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> paintingData =
        std::make_unique<SkPerlinNoiseShaderImpl::PaintingData>(fTileSize,
                                                                  fSeed,
                                                                  fBaseFrequencyX,
                                                                  fBaseFrequencyY,
                                                                  paintMatrix);

    SkMatrix m = args.fMatrixProvider.localToDevice();
    m.setTranslateX(-localMatrix->getTranslateX() + SK_Scalar1);
    m.setTranslateY(-localMatrix->getTranslateY() + SK_Scalar1);

    auto context = args.fContext;

    if (0 == fNumOctaves) {
        if (kFractalNoise_Type == fType) {
            // Extract the incoming alpha and emit rgba = (a/4, a/4, a/4, a/2)
            // TODO: Either treat the output of this shader as sRGB or allow client to specify a
            // color space of the noise. Either way, this case (and the GLSL) need to convert to
            // the destination.
            auto inner = GrFragmentProcessor::ModulateRGBA(
                    /*child=*/nullptr, SkPMColor4f::FromBytes_RGBA(0x80404040));
            return GrFragmentProcessor::MulChildByInputAlpha(std::move(inner));
        }
        // Emit zero.
        return GrConstColorProcessor::Make(SK_PMColor4fTRANSPARENT);
    }

    // Need to assert that the textures we'll create are power of 2 so that now copy is needed. We
    // also know that we will not be using mipmaps. If things things weren't true we should go
    // through GrBitmapTextureMaker to handle needed copies.
    const SkBitmap& permutationsBitmap = paintingData->getPermutationsBitmap();
    SkASSERT(SkIsPow2(permutationsBitmap.width()) && SkIsPow2(permutationsBitmap.height()));
    auto permutationsView = GrMakeCachedBitmapProxyView(context, permutationsBitmap);

    const SkBitmap& noiseBitmap = paintingData->getNoiseBitmap();
    SkASSERT(SkIsPow2(noiseBitmap.width()) && SkIsPow2(noiseBitmap.height()));
    auto noiseView = GrMakeCachedBitmapProxyView(context, noiseBitmap);

    if (permutationsView.proxy() && noiseView.proxy()) {
        auto inner = GrPerlinNoise2Effect::Make(fType,
                                                fNumOctaves,
                                                fStitchTiles,
                                                std::move(paintingData),
                                                std::move(permutationsView),
                                                std::move(noiseView),
                                                m,
                                                *context->priv().caps());
        return GrFragmentProcessor::MulChildByInputAlpha(std::move(inner));
    }
    return nullptr;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_input(SkScalar baseX, SkScalar baseY, int numOctaves, const SkISize* tileSize,
                        SkScalar seed) {
    if (!(baseX >= 0 && baseY >= 0)) {
        return false;
    }
    if (!(numOctaves >= 0 && numOctaves <= SkPerlinNoiseShaderImpl::kMaxOctaves)) {
        return false;
    }
    if (tileSize && !(tileSize->width() >= 0 && tileSize->height() >= 0)) {
        return false;
    }
    if (!SkScalarIsFinite(seed)) {
        return false;
    }
    return true;
}

sk_sp<SkShader> SkPerlinNoiseShader::MakeFractalNoise(SkScalar baseFrequencyX,
                                                      SkScalar baseFrequencyY,
                                                      int numOctaves, SkScalar seed,
                                                      const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }
    return sk_sp<SkShader>(new SkPerlinNoiseShaderImpl(SkPerlinNoiseShaderImpl::kFractalNoise_Type,
                                                 baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                                 tileSize));
}

sk_sp<SkShader> SkPerlinNoiseShader::MakeTurbulence(SkScalar baseFrequencyX,
                                                    SkScalar baseFrequencyY,
                                                    int numOctaves, SkScalar seed,
                                                    const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }
    return sk_sp<SkShader>(new SkPerlinNoiseShaderImpl(SkPerlinNoiseShaderImpl::kTurbulence_Type,
                                                 baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                                 tileSize));
}

void SkPerlinNoiseShader::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkPerlinNoiseShaderImpl);
}
