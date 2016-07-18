/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPerlinNoiseShader.h"
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

struct SkPerlinNoiseShader::StitchData {
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

struct SkPerlinNoiseShader::PaintingData {
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
#endif
};

sk_sp<SkShader> SkPerlinNoiseShader::MakeFractalNoise(SkScalar baseFrequencyX,
                                                      SkScalar baseFrequencyY,
                                                      int numOctaves, SkScalar seed,
                                                      const SkISize* tileSize) {
    return sk_sp<SkShader>(new SkPerlinNoiseShader(kFractalNoise_Type, baseFrequencyX,
                                                   baseFrequencyY, numOctaves,
                                                   seed, tileSize));
}

sk_sp<SkShader> SkPerlinNoiseShader::MakeTurbulence(SkScalar baseFrequencyX,
                                                    SkScalar baseFrequencyY,
                                                    int numOctaves, SkScalar seed,
                                                    const SkISize* tileSize) {
    return sk_sp<SkShader>(new SkPerlinNoiseShader(kTurbulence_Type, baseFrequencyX, baseFrequencyY,
                                                   numOctaves, seed, tileSize));
}

SkPerlinNoiseShader::SkPerlinNoiseShader(SkPerlinNoiseShader::Type type,
                                         SkScalar baseFrequencyX,
                                         SkScalar baseFrequencyY,
                                         int numOctaves,
                                         SkScalar seed,
                                         const SkISize* tileSize)
  : fType(type)
  , fBaseFrequencyX(baseFrequencyX)
  , fBaseFrequencyY(baseFrequencyY)
  , fNumOctaves(SkTPin<int>(numOctaves, 0, 255)) // [0,255] octaves allowed
  , fSeed(seed)
  , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
  , fStitchTiles(!fTileSize.isEmpty())
{
    SkASSERT(fNumOctaves >= 0 && fNumOctaves < 256);
}

SkPerlinNoiseShader::~SkPerlinNoiseShader() {
}

sk_sp<SkFlattenable> SkPerlinNoiseShader::CreateProc(SkReadBuffer& buffer) {
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
            return SkPerlinNoiseShader::MakeFractalNoise(freqX, freqY, octaves, seed,
                                                         &tileSize);
        case kTurbulence_Type:
            return SkPerlinNoiseShader::MakeTurbulence(freqX, freqY, octaves, seed,
                                                       &tileSize);
        default:
            return nullptr;
    }
}

void SkPerlinNoiseShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int) fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::noise2D(
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
    const SkPerlinNoiseShader& perlinNoiseShader = static_cast<const SkPerlinNoiseShader&>(fShader);
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

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::calculateTurbulenceValueForPoint(
        int channel, StitchData& stitchData, const SkPoint& point) const {
    const SkPerlinNoiseShader& perlinNoiseShader = static_cast<const SkPerlinNoiseShader&>(fShader);
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

SkPMColor SkPerlinNoiseShader::PerlinNoiseShaderContext::shade(
        const SkPoint& point, StitchData& stitchData) const {
    SkPoint newPoint;
    fMatrix.mapPoints(&newPoint, &point, 1);
    newPoint.fX = SkScalarRoundToScalar(newPoint.fX);
    newPoint.fY = SkScalarRoundToScalar(newPoint.fY);

    U8CPU rgba[4];
    for (int channel = 3; channel >= 0; --channel) {
        rgba[channel] = SkScalarFloorToInt(255 *
            calculateTurbulenceValueForPoint(channel, stitchData, newPoint));
    }
    return SkPreMultiplyARGB(rgba[3], rgba[0], rgba[1], rgba[2]);
}

SkShader::Context* SkPerlinNoiseShader::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    return new (storage) PerlinNoiseShaderContext(*this, rec);
}

size_t SkPerlinNoiseShader::onContextSize(const ContextRec&) const {
    return sizeof(PerlinNoiseShaderContext);
}

SkPerlinNoiseShader::PerlinNoiseShaderContext::PerlinNoiseShaderContext(
        const SkPerlinNoiseShader& shader, const ContextRec& rec)
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

SkPerlinNoiseShader::PerlinNoiseShaderContext::~PerlinNoiseShaderContext() { delete fPaintingData; }

void SkPerlinNoiseShader::PerlinNoiseShaderContext::shadeSpan(
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

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fStitchDataUni;
    GrGLSLProgramDataManager::UniformHandle fBaseFrequencyUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrPerlinNoiseEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(SkPerlinNoiseShader::Type type,
                                           int numOctaves, bool stitchTiles,
                                           SkPerlinNoiseShader::PaintingData* paintingData,
                                           GrTexture* permutationsTexture, GrTexture* noiseTexture,
                                           const SkMatrix& matrix) {
        return sk_sp<GrFragmentProcessor>(
            new GrPerlinNoiseEffect(type, numOctaves, stitchTiles, paintingData,
                                    permutationsTexture, noiseTexture, matrix));
    }

    virtual ~GrPerlinNoiseEffect() { delete fPaintingData; }

    const char* name() const override { return "PerlinNoise"; }

    const SkPerlinNoiseShader::StitchData& stitchData() const { return fPaintingData->fStitchDataInit; }

    SkPerlinNoiseShader::Type type() const { return fType; }
    bool stitchTiles() const { return fStitchTiles; }
    const SkVector& baseFrequency() const { return fPaintingData->fBaseFrequency; }
    int numOctaves() const { return fNumOctaves; }
    const SkMatrix& matrix() const { return fCoordTransform.getMatrix(); }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GrGLPerlinNoise;
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GrGLPerlinNoise::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const GrPerlinNoiseEffect& s = sBase.cast<GrPerlinNoiseEffect>();
        return fType == s.fType &&
               fPaintingData->fBaseFrequency == s.fPaintingData->fBaseFrequency &&
               fNumOctaves == s.fNumOctaves &&
               fStitchTiles == s.fStitchTiles &&
               fPaintingData->fStitchDataInit == s.fPaintingData->fStitchDataInit;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
    }

    GrPerlinNoiseEffect(SkPerlinNoiseShader::Type type,
                        int numOctaves, bool stitchTiles,
                        SkPerlinNoiseShader::PaintingData* paintingData,
                        GrTexture* permutationsTexture, GrTexture* noiseTexture,
                        const SkMatrix& matrix)
      : fType(type)
      , fNumOctaves(numOctaves)
      , fStitchTiles(stitchTiles)
      , fPermutationsAccess(permutationsTexture)
      , fNoiseAccess(noiseTexture)
      , fPaintingData(paintingData) {
        this->initClassID<GrPerlinNoiseEffect>();
        this->addTextureAccess(&fPermutationsAccess);
        this->addTextureAccess(&fNoiseAccess);
        fCoordTransform.reset(kLocal_GrCoordSet, matrix);
        this->addCoordTransform(&fCoordTransform);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkPerlinNoiseShader::Type       fType;
    GrCoordTransform                fCoordTransform;
    int                             fNumOctaves;
    bool                            fStitchTiles;
    GrTextureAccess                 fPermutationsAccess;
    GrTextureAccess                 fNoiseAccess;
    SkPerlinNoiseShader::PaintingData *fPaintingData;

private:
    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrPerlinNoiseEffect);

sk_sp<GrFragmentProcessor> GrPerlinNoiseEffect::TestCreate(GrProcessorTestData* d) {
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

    return shader->asFragmentProcessor(d->fContext,
                                       GrTest::TestMatrix(d->fRandom), nullptr,
                                       kNone_SkFilterQuality, SkSourceGammaTreatment::kRespect);
}

void GrGLPerlinNoise::emitCode(EmitArgs& args) {
    const GrPerlinNoiseEffect& pne = args.fFp.cast<GrPerlinNoiseEffect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    SkString vCoords = fragBuilder->ensureFSCoords2D(args.fCoords, 0);

    fBaseFrequencyUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "baseFrequency");
    const char* baseFrequencyUni = uniformHandler->getUniformCStr(fBaseFrequencyUni);

    const char* stitchDataUni = nullptr;
    if (pne.stitchTiles()) {
        fStitchDataUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
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
    if (pne.stitchTiles()) {
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
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[0], xCoords.c_str(),
                                         kVec2f_GrSLType);
        noiseCode.append(".r;");
    }

    // Get permutation for x + 1
    {
        SkString xCoords("");
        xCoords.appendf("vec2(%s.z, 0.5)", floorVal);

        noiseCode.appendf("\n\t%s.y = ", latticeIdx);
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[0], xCoords.c_str(),
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
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[1], latticeCoords.c_str(),
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
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[1], latticeCoords.c_str(),
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
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[1], latticeCoords.c_str(),
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
        fragBuilder->appendTextureLookup(&noiseCode, args.fTexSamplers[1], latticeCoords.c_str(),
                                         kVec2f_GrSLType);
        noiseCode.appendf(".bgra;\n\t%s.x = ", uv);
        noiseCode.appendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    // Compute 'b' as a linear interpolation of 'u' and 'v'
    noiseCode.appendf("\n\t%s.y = mix(%s.x, %s.y, %s.x);", ab, uv, uv, noiseSmooth);
    // Compute the noise as a linear interpolation of 'a' and 'b'
    noiseCode.appendf("\n\treturn mix(%s.x, %s.y, %s.y);\n", ab, ab, noiseSmooth);

    SkString noiseFuncName;
    if (pne.stitchTiles()) {
        fragBuilder->emitFunction(kFloat_GrSLType,
                                  "perlinnoise", SK_ARRAY_COUNT(gPerlinNoiseStitchArgs),
                                  gPerlinNoiseStitchArgs, noiseCode.c_str(), &noiseFuncName);
    } else {
        fragBuilder->emitFunction(kFloat_GrSLType,
                                  "perlinnoise", SK_ARRAY_COUNT(gPerlinNoiseArgs),
                                  gPerlinNoiseArgs, noiseCode.c_str(), &noiseFuncName);
    }

    // There are rounding errors if the floor operation is not performed here
    fragBuilder->codeAppendf("\n\t\tvec2 %s = floor(%s.xy) * %s;",
                             noiseVec, vCoords.c_str(), baseFrequencyUni);

    // Clear the color accumulator
    fragBuilder->codeAppendf("\n\t\t%s = vec4(0.0);", args.fOutputColor);

    if (pne.stitchTiles()) {
        // Set up TurbulenceInitial stitch values.
        fragBuilder->codeAppendf("vec2 %s = %s;", stitchData, stitchDataUni);
    }

    fragBuilder->codeAppendf("float %s = 1.0;", ratio);

    // Loop over all octaves
    fragBuilder->codeAppendf("for (int octave = 0; octave < %d; ++octave) {", pne.numOctaves());

    fragBuilder->codeAppendf("%s += ", args.fOutputColor);
    if (pne.type() != SkPerlinNoiseShader::kFractalNoise_Type) {
        fragBuilder->codeAppend("abs(");
    }
    if (pne.stitchTiles()) {
        fragBuilder->codeAppendf(
            "vec4(\n\t\t\t\t%s(%s, %s, %s),\n\t\t\t\t%s(%s, %s, %s),"
                 "\n\t\t\t\t%s(%s, %s, %s),\n\t\t\t\t%s(%s, %s, %s))",
            noiseFuncName.c_str(), chanCoordR, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordG, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordB, noiseVec, stitchData,
            noiseFuncName.c_str(), chanCoordA, noiseVec, stitchData);
    } else {
        fragBuilder->codeAppendf(
            "vec4(\n\t\t\t\t%s(%s, %s),\n\t\t\t\t%s(%s, %s),"
                 "\n\t\t\t\t%s(%s, %s),\n\t\t\t\t%s(%s, %s))",
            noiseFuncName.c_str(), chanCoordR, noiseVec,
            noiseFuncName.c_str(), chanCoordG, noiseVec,
            noiseFuncName.c_str(), chanCoordB, noiseVec,
            noiseFuncName.c_str(), chanCoordA, noiseVec);
    }
    if (pne.type() != SkPerlinNoiseShader::kFractalNoise_Type) {
        fragBuilder->codeAppendf(")"); // end of "abs("
    }
    fragBuilder->codeAppendf(" * %s;", ratio);

    fragBuilder->codeAppendf("\n\t\t\t%s *= vec2(2.0);", noiseVec);
    fragBuilder->codeAppendf("\n\t\t\t%s *= 0.5;", ratio);

    if (pne.stitchTiles()) {
        fragBuilder->codeAppendf("\n\t\t\t%s *= vec2(2.0);", stitchData);
    }
    fragBuilder->codeAppend("\n\t\t}"); // end of the for loop on octaves

    if (pne.type() == SkPerlinNoiseShader::kFractalNoise_Type) {
        // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
        // by fractalNoise and (turbulenceFunctionResult) by turbulence.
        fragBuilder->codeAppendf("\n\t\t%s = %s * vec4(0.5) + vec4(0.5);",
                                 args.fOutputColor,args.fOutputColor);
    }

    // Clamp values
    fragBuilder->codeAppendf("\n\t\t%s = clamp(%s, 0.0, 1.0);", args.fOutputColor, args.fOutputColor);

    // Pre-multiply the result
    fragBuilder->codeAppendf("\n\t\t%s = vec4(%s.rgb * %s.aaa, %s.a);\n",
                             args.fOutputColor, args.fOutputColor,
                             args.fOutputColor, args.fOutputColor);
}

void GrGLPerlinNoise::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrPerlinNoiseEffect& turbulence = processor.cast<GrPerlinNoiseEffect>();

    uint32_t key = turbulence.numOctaves();

    key = key << 3; // Make room for next 3 bits

    switch (turbulence.type()) {
        case SkPerlinNoiseShader::kFractalNoise_Type:
            key |= 0x1;
            break;
        case SkPerlinNoiseShader::kTurbulence_Type:
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
                                const GrProcessor& processor) {
    INHERITED::onSetData(pdman, processor);

    const GrPerlinNoiseEffect& turbulence = processor.cast<GrPerlinNoiseEffect>();

    const SkVector& baseFrequency = turbulence.baseFrequency();
    pdman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);

    if (turbulence.stitchTiles()) {
        const SkPerlinNoiseShader::StitchData& stitchData = turbulence.stitchData();
        pdman.set2f(fStitchDataUni, SkIntToScalar(stitchData.fWidth),
                                   SkIntToScalar(stitchData.fHeight));
    }
}

/////////////////////////////////////////////////////////////////////
sk_sp<GrFragmentProcessor> SkPerlinNoiseShader::asFragmentProcessor(
                                                     GrContext* context,
                                                     const SkMatrix& viewM,
                                                     const SkMatrix* externalLocalMatrix,
                                                     SkFilterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {
    SkASSERT(context);

    SkMatrix localMatrix = this->getLocalMatrix();
    if (externalLocalMatrix) {
        localMatrix.preConcat(*externalLocalMatrix);
    }

    SkMatrix matrix = viewM;
    matrix.preConcat(localMatrix);

    if (0 == fNumOctaves) {
        if (kFractalNoise_Type == fType) {
            // Extract the incoming alpha and emit rgba = (a/4, a/4, a/4, a/2)
            sk_sp<GrFragmentProcessor> inner(
                GrConstColorProcessor::Make(0x80404040,
                                            GrConstColorProcessor::kModulateRGBA_InputMode));
            return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
        }
        // Emit zero.
        return GrConstColorProcessor::Make(0x0, GrConstColorProcessor::kIgnore_InputMode);
    }

    // Either we don't stitch tiles, either we have a valid tile size
    SkASSERT(!fStitchTiles || !fTileSize.isEmpty());

    SkPerlinNoiseShader::PaintingData* paintingData =
            new PaintingData(fTileSize, fSeed, fBaseFrequencyX, fBaseFrequencyY, matrix);
    SkAutoTUnref<GrTexture> permutationsTexture(
        GrRefCachedBitmapTexture(context, paintingData->getPermutationsBitmap(),
                                 GrTextureParams::ClampNoFilter(), gammaTreatment));
    SkAutoTUnref<GrTexture> noiseTexture(
        GrRefCachedBitmapTexture(context, paintingData->getNoiseBitmap(),
                                 GrTextureParams::ClampNoFilter(), gammaTreatment));

    SkMatrix m = viewM;
    m.setTranslateX(-localMatrix.getTranslateX() + SK_Scalar1);
    m.setTranslateY(-localMatrix.getTranslateY() + SK_Scalar1);
    if ((permutationsTexture) && (noiseTexture)) {
        sk_sp<GrFragmentProcessor> inner(
            GrPerlinNoiseEffect::Make(fType,
                                      fNumOctaves,
                                      fStitchTiles,
                                      paintingData,
                                      permutationsTexture, noiseTexture,
                                      m));
        return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
    }
    delete paintingData;
    return nullptr;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkPerlinNoiseShader::toString(SkString* str) const {
    str->append("SkPerlinNoiseShader: (");

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
