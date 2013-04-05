/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDither.h"
#include "SkPerlinNoiseShader.h"
#include "SkFlattenableBuffers.h"
#include "SkShader.h"
#include "SkUnPreMultiply.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "GrTBackendEffectFactory.h"
#include "SkGr.h"
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
    if (noiseValue >= limitValue - 1) {
        noiseValue -= newValue - 1;
    }
    return noiseValue;
}

inline SkScalar smoothCurve(SkScalar t) {
    static const SkScalar SK_Scalar3 = SkFloatToScalar(3.0f);

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
    PaintingData(const SkISize& tileSize)
      : fSeed(0)
      , fTileSize(tileSize)
      , fPermutationsBitmap(NULL)
      , fNoiseBitmap(NULL)
    {}

    ~PaintingData()
    {
        SkDELETE(fPermutationsBitmap);
        SkDELETE(fNoiseBitmap);
    }

    int         fSeed;
    uint8_t     fLatticeSelector[kBlockSize];
    uint16_t    fNoise[4][kBlockSize][2];
    SkPoint     fGradient[4][kBlockSize];
    SkISize     fTileSize;
    SkVector    fBaseFrequency;
    StitchData  fStitchDataInit;

private:

    SkBitmap*    fPermutationsBitmap;
    SkBitmap*    fNoiseBitmap;

public:

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

    void init(SkScalar seed)
    {
        static const SkScalar gInvBlockSizef = SkScalarInvert(SkIntToScalar(kBlockSize));

        // The seed value clamp to the range [1, kRandMaximum - 1].
        fSeed = SkScalarRoundToInt(seed);
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
        static const SkScalar halfMax16bits = SkFloatToScalar(32767.5f);

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
                    fGradient[channel][i].fX + SK_Scalar1, halfMax16bits));
                fNoise[channel][i][1] = SkScalarRoundToInt(SkScalarMul(
                    fGradient[channel][i].fY + SK_Scalar1, halfMax16bits));
            }
        }

        // Invalidate bitmaps
        SkDELETE(fPermutationsBitmap);
        fPermutationsBitmap = NULL;
        SkDELETE(fNoiseBitmap);
        fNoiseBitmap = NULL;
    }

    void stitch() {
        SkScalar tileWidth  = SkIntToScalar(fTileSize.width());
        SkScalar tileHeight = SkIntToScalar(fTileSize.height());
        SkASSERT(tileWidth > 0 && tileHeight > 0);
        // When stitching tiled turbulence, the frequencies must be adjusted
        // so that the tile borders will be continuous.
        if (fBaseFrequency.fX) {
            SkScalar lowFrequencx = SkScalarDiv(
                SkScalarMulFloor(tileWidth, fBaseFrequency.fX), tileWidth);
            SkScalar highFrequencx = SkScalarDiv(
                SkScalarMulCeil(tileWidth, fBaseFrequency.fX), tileWidth);
            // BaseFrequency should be non-negative according to the standard.
            if (SkScalarDiv(fBaseFrequency.fX, lowFrequencx) < 
                SkScalarDiv(highFrequencx, fBaseFrequency.fX)) {
                fBaseFrequency.fX = lowFrequencx;
            } else {
                fBaseFrequency.fX = highFrequencx;
            }
        }
        if (fBaseFrequency.fY) {
            SkScalar lowFrequency = SkScalarDiv(
                SkScalarMulFloor(tileHeight, fBaseFrequency.fY), tileHeight);
            SkScalar highFrequency = SkScalarDiv(
                SkScalarMulCeil(tileHeight, fBaseFrequency.fY), tileHeight);
            if (SkScalarDiv(fBaseFrequency.fY, lowFrequency) < 
                SkScalarDiv(highFrequency, fBaseFrequency.fY)) {
                fBaseFrequency.fY = lowFrequency;
            } else {
                fBaseFrequency.fY = highFrequency;
            }
        }
        // Set up TurbulenceInitial stitch values.
        fStitchDataInit.fWidth  =
            SkScalarMulRound(tileWidth, fBaseFrequency.fX);
        fStitchDataInit.fWrapX  = kPerlinNoise + fStitchDataInit.fWidth;
        fStitchDataInit.fHeight =
            SkScalarMulRound(tileHeight, fBaseFrequency.fY);
        fStitchDataInit.fWrapY  = kPerlinNoise + fStitchDataInit.fHeight;
    }

    SkBitmap* getPermutationsBitmap()
    {
        if (!fPermutationsBitmap) {
            fPermutationsBitmap = SkNEW(SkBitmap);
            fPermutationsBitmap->setConfig(SkBitmap::kA8_Config, kBlockSize, 1);
            fPermutationsBitmap->allocPixels();
            uint8_t* bitmapPixels = fPermutationsBitmap->getAddr8(0, 0);
            memcpy(bitmapPixels, fLatticeSelector, sizeof(uint8_t) * kBlockSize);
        }
        return fPermutationsBitmap;
    }

    SkBitmap* getNoiseBitmap()
    {
        if (!fNoiseBitmap) {
            fNoiseBitmap = SkNEW(SkBitmap);
            fNoiseBitmap->setConfig(SkBitmap::kARGB_8888_Config, kBlockSize, 4);
            fNoiseBitmap->allocPixels();
            uint32_t* bitmapPixels = fNoiseBitmap->getAddr32(0, 0);
            memcpy(bitmapPixels, fNoise[0][0], sizeof(uint16_t) * kBlockSize * 4 * 2);
        }
        return fNoiseBitmap;
    }
};

SkShader* SkPerlinNoiseShader::CreateFractalNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                                  int numOctaves, SkScalar seed,
                                                  const SkISize* tileSize) {
    return SkNEW_ARGS(SkPerlinNoiseShader, (kFractalNoise_Type, baseFrequencyX, baseFrequencyY,
                                            numOctaves, seed, tileSize));
}

SkShader* SkPerlinNoiseShader::CreateTubulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                              int numOctaves, SkScalar seed,
                                              const SkISize* tileSize) {
    return SkNEW_ARGS(SkPerlinNoiseShader, (kTurbulence_Type, baseFrequencyX, baseFrequencyY,
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
  , fNumOctaves(numOctaves & 0xFF /*[0,255] octaves allowed*/)
  , fSeed(seed)
  , fStitchTiles((tileSize != NULL) && !tileSize->isEmpty())
  , fPaintingData(NULL)
{
    SkASSERT(numOctaves >= 0 && numOctaves < 256);
    setTileSize(fStitchTiles ? *tileSize : SkISize::Make(0,0));
    fMatrix.reset();
}

SkPerlinNoiseShader::SkPerlinNoiseShader(SkFlattenableReadBuffer& buffer) :
        INHERITED(buffer), fPaintingData(NULL) {
    fType           = (SkPerlinNoiseShader::Type) buffer.readInt();
    fBaseFrequencyX = buffer.readScalar();
    fBaseFrequencyY = buffer.readScalar();
    fNumOctaves     = buffer.readInt();
    fSeed           = buffer.readScalar();
    fStitchTiles    = buffer.readBool();
    fTileSize.fWidth  = buffer.readInt();
    fTileSize.fHeight = buffer.readInt();
    setTileSize(fTileSize);
    fMatrix.reset();
}

SkPerlinNoiseShader::~SkPerlinNoiseShader() {
    // Safety, should have been done in endContext()
    SkDELETE(fPaintingData);
}

void SkPerlinNoiseShader::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeBool(fStitchTiles);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

void SkPerlinNoiseShader::initPaint(PaintingData& paintingData)
{
    paintingData.init(fSeed);

    // Set frequencies to original values
    paintingData.fBaseFrequency.set(fBaseFrequencyX, fBaseFrequencyY);
    // Adjust frequecies based on size if stitching is enabled
    if (fStitchTiles) {
        paintingData.stitch();
    }
}

void SkPerlinNoiseShader::setTileSize(const SkISize& tileSize) {
    fTileSize = tileSize;

    if (NULL == fPaintingData) {
        fPaintingData = SkNEW_ARGS(PaintingData, (fTileSize));
        initPaint(*fPaintingData);
    } else {
        // Set Size
        fPaintingData->fTileSize = fTileSize;
        // Set frequencies to original values
        fPaintingData->fBaseFrequency.set(fBaseFrequencyX, fBaseFrequencyY);
        // Adjust frequecies based on size if stitching is enabled
        if (fStitchTiles) {
            fPaintingData->stitch();
        }
    }
}

SkScalar SkPerlinNoiseShader::noise2D(int channel, const PaintingData& paintingData,
                                     const StitchData& stitchData, const SkPoint& noiseVector)
{
    struct Noise {
        int noisePositionIntegerValue;
        SkScalar noisePositionFractionValue;
        Noise(SkScalar component)
        {
            SkScalar position = component + kPerlinNoise;
            noisePositionIntegerValue = SkScalarFloorToInt(position);
            noisePositionFractionValue = position - SkIntToScalar(noisePositionIntegerValue);
        }
    };
    Noise noiseX(noiseVector.x());
    Noise noiseY(noiseVector.y());
    SkScalar u, v;
    // If stitching, adjust lattice points accordingly.
    if (fStitchTiles) {
        noiseX.noisePositionIntegerValue =
            checkNoise(noiseX.noisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.noisePositionIntegerValue =
            checkNoise(noiseY.noisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
    }
    noiseX.noisePositionIntegerValue &= kBlockMask;
    noiseY.noisePositionIntegerValue &= kBlockMask;
    int latticeIndex =
        paintingData.fLatticeSelector[noiseX.noisePositionIntegerValue] + 
        noiseY.noisePositionIntegerValue;
    int nextLatticeIndex =
        paintingData.fLatticeSelector[(noiseX.noisePositionIntegerValue + 1) & kBlockMask] + 
        noiseY.noisePositionIntegerValue;
    SkScalar sx = smoothCurve(noiseX.noisePositionFractionValue);
    SkScalar sy = smoothCurve(noiseY.noisePositionFractionValue);
    // This is taken 1:1 from SVG spec: http://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
    SkPoint fractionValue = SkPoint::Make(noiseX.noisePositionFractionValue,
                                          noiseY.noisePositionFractionValue); // Offset (0,0)
    u = paintingData.fGradient[channel][latticeIndex & kBlockMask].dot(fractionValue);
    fractionValue.fX -= SK_Scalar1; // Offset (-1,0)
    v = paintingData.fGradient[channel][nextLatticeIndex & kBlockMask].dot(fractionValue);
    SkScalar a = SkScalarInterp(u, v, sx);
    fractionValue.fY -= SK_Scalar1; // Offset (-1,-1)
    v = paintingData.fGradient[channel][(nextLatticeIndex + 1) & kBlockMask].dot(fractionValue);
    fractionValue.fX = noiseX.noisePositionFractionValue; // Offset (0,-1)
    u = paintingData.fGradient[channel][(latticeIndex + 1) & kBlockMask].dot(fractionValue);
    SkScalar b = SkScalarInterp(u, v, sx);
    return SkScalarInterp(a, b, sy);
}

SkScalar SkPerlinNoiseShader::calculateTurbulenceValueForPoint(
    int channel, const PaintingData& paintingData, StitchData& stitchData, const SkPoint& point)
{
    if (fStitchTiles) {
        // Set up TurbulenceInitial stitch values.
        stitchData = paintingData.fStitchDataInit;
    }
    SkScalar turbulenceFunctionResult = 0;
    SkPoint noiseVector(SkPoint::Make(SkScalarMul(point.x(), paintingData.fBaseFrequency.fX),
                                      SkScalarMul(point.y(), paintingData.fBaseFrequency.fY)));
    SkScalar ratio = SK_Scalar1;
    for (int octave = 0; octave < fNumOctaves; ++octave) {
        SkScalar noise = noise2D(channel, paintingData, stitchData, noiseVector);
        turbulenceFunctionResult += SkScalarDiv(
            (fType == kFractalNoise_Type) ? noise : SkScalarAbs(noise), ratio);
        noiseVector.fX *= 2;
        noiseVector.fY *= 2;
        ratio *= 2;
        if (fStitchTiles) {
            // Update stitch values
            stitchData.fWidth  *= 2;
            stitchData.fWrapX   = stitchData.fWidth + kPerlinNoise;
            stitchData.fHeight *= 2;
            stitchData.fWrapY   = stitchData.fHeight + kPerlinNoise;
        }
    }

    // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
    // by fractalNoise and (turbulenceFunctionResult) by turbulence.
    if (fType == kFractalNoise_Type) {
        turbulenceFunctionResult =
            SkScalarMul(turbulenceFunctionResult, SK_ScalarHalf) + SK_ScalarHalf;
    }

    if (channel == 3) { // Scale alpha by paint value
        turbulenceFunctionResult = SkScalarMul(turbulenceFunctionResult,
            SkScalarDiv(SkIntToScalar(getPaintAlpha()), SkIntToScalar(255)));
    }

    // Clamp result
    return SkScalarPin(turbulenceFunctionResult, 0, SK_Scalar1);
}

SkPMColor SkPerlinNoiseShader::shade(const SkPoint& point, StitchData& stitchData) {
    SkMatrix matrix = fMatrix;
    SkMatrix invMatrix;
    if (!matrix.invert(&invMatrix)) {
        invMatrix.reset();
    } else {
        invMatrix.postConcat(invMatrix); // Square the matrix
    }
    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the setData() function.
    matrix.postTranslate(SK_Scalar1, SK_Scalar1);
    SkPoint newPoint;
    matrix.mapPoints(&newPoint, &point, 1);
    invMatrix.mapPoints(&newPoint, &newPoint, 1);
    newPoint.fX = SkScalarRoundToScalar(newPoint.fX);
    newPoint.fY = SkScalarRoundToScalar(newPoint.fY);

    U8CPU rgba[4];
    for (int channel = 3; channel >= 0; --channel) {
        rgba[channel] = SkScalarFloorToInt(255 *
            calculateTurbulenceValueForPoint(channel, *fPaintingData, stitchData, newPoint));
    }
    return SkPreMultiplyARGB(rgba[3], rgba[0], rgba[1], rgba[2]);
}

bool SkPerlinNoiseShader::setContext(const SkBitmap& device, const SkPaint& paint,
                                     const SkMatrix& matrix) {
    fMatrix = matrix;
    return INHERITED::setContext(device, paint, matrix);
}

void SkPerlinNoiseShader::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkPoint point = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
    StitchData stitchData;
    for (int i = 0; i < count; ++i) {
        result[i] = shade(point, stitchData);
        point.fX += SK_Scalar1;
    }
}

void SkPerlinNoiseShader::shadeSpan16(int x, int y, uint16_t result[], int count) {
    SkPoint point = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
    StitchData stitchData;
    DITHER_565_SCAN(y);
    for (int i = 0; i < count; ++i) {
        unsigned dither = DITHER_VALUE(x);
        result[i] = SkDitherRGB32To565(shade(point, stitchData), dither);
        DITHER_INC_X(x);
        point.fX += SK_Scalar1;
    }
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTBackendEffectFactory.h"

class GrGLPerlinNoise : public GrGLEffect {
public:

    GrGLPerlinNoise(const GrBackendEffectFactory& factory,
                    const GrDrawEffect& drawEffect);
    virtual ~GrGLPerlinNoise() { }

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&);

private:
    SkPerlinNoiseShader::Type           fType;
    bool                                fStitchTiles;
    int                                 fNumOctaves;
    GrGLUniformManager::UniformHandle   fBaseFrequencyUni;
    GrGLUniformManager::UniformHandle   fStitchDataUni;
    GrGLUniformManager::UniformHandle   fAlphaUni;
    GrGLUniformManager::UniformHandle   fInvMatrixUni;
    GrGLEffectMatrix                    fEffectMatrix;

    typedef GrGLEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrPerlinNoiseEffect : public GrEffect {
public:
    static GrEffectRef* Create(SkPerlinNoiseShader::Type type, const SkVector& baseFrequency,
                               int numOctaves, bool stitchTiles,
                               const SkPerlinNoiseShader::StitchData& stitchData,
                               GrTexture* permutationsTexture, GrTexture* noiseTexture,
                               const SkMatrix& matrix, uint8_t alpha) {
        AutoEffectUnref effect(SkNEW_ARGS(GrPerlinNoiseEffect, (type, baseFrequency, numOctaves,
            stitchTiles, stitchData, permutationsTexture, noiseTexture, matrix, alpha)));
        return CreateEffectRef(effect);
    }

    virtual ~GrPerlinNoiseEffect() { }

    static const char* Name() { return "PerlinNoise"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrPerlinNoiseEffect>::getInstance();
    }
    SkPerlinNoiseShader::Type type() const { return fType; }
    bool stitchTiles() const { return fStitchTiles; }
    const SkVector& baseFrequency() const { return fBaseFrequency; }
    int numOctaves() const { return fNumOctaves & 0xFF; /*[0,255] octaves allowed*/ }
    const SkPerlinNoiseShader::StitchData& stitchData() const { return fStitchData; }
    const SkMatrix& matrix() const { return fMatrix; }
    uint8_t alpha() const { return fAlpha; }
    GrGLEffectMatrix::CoordsType coordsType() const { return GrEffect::kLocal_CoordsType; }

    typedef GrGLPerlinNoise GLEffect;

    void getConstantColorComponents(GrColor*, uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0; // This is noise. Nothing is constant.
    }

private:
    virtual bool onIsEqual(const GrEffect& sBase) const SK_OVERRIDE {
        const GrPerlinNoiseEffect& s = CastEffect<GrPerlinNoiseEffect>(sBase);
        return fPermutationsAccess.getTexture() == s.fPermutationsAccess.getTexture() &&
               fNoiseAccess.getTexture() == s.fNoiseAccess.getTexture() &&
               fType == s.fType &&
               fBaseFrequency == s.fBaseFrequency &&
               fStitchTiles == s.fStitchTiles &&
               fStitchData == s.fStitchData &&
               fMatrix == s.fMatrix &&
               fAlpha == s.fAlpha;
    }

    GrPerlinNoiseEffect(SkPerlinNoiseShader::Type type, const SkVector& baseFrequency,
                        int numOctaves, bool stitchTiles,
                        const SkPerlinNoiseShader::StitchData& stitchData,
                        GrTexture* permutationsTexture, GrTexture* noiseTexture,
                        const SkMatrix& matrix, uint8_t alpha)
      : fPermutationsAccess(permutationsTexture)
      , fNoiseAccess(noiseTexture)
      , fType(type)
      , fBaseFrequency(baseFrequency)
      , fNumOctaves(numOctaves)
      , fStitchTiles(stitchTiles)
      , fStitchData(stitchData)
      , fMatrix(matrix)
      , fAlpha(alpha)
    {
        this->addTextureAccess(&fPermutationsAccess);
        this->addTextureAccess(&fNoiseAccess);
    }

//    GR_DECLARE_EFFECT_TEST;

    GrTextureAccess                 fPermutationsAccess;
    GrTextureAccess                 fNoiseAccess;
    SkPerlinNoiseShader::Type       fType;
    SkVector                        fBaseFrequency;
    int                             fNumOctaves;
    bool                            fStitchTiles;
    SkPerlinNoiseShader::StitchData fStitchData;
    SkMatrix                        fMatrix;
    uint8_t                         fAlpha;

    typedef GrEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////
#if 0
GR_DEFINE_EFFECT_TEST(GrPerlinNoiseEffect);

GrEffectRef* GrPerlinNoiseEffect::TestCreate(SkMWCRandom* random,
                                             GrContext* context,
                                             const GrDrawTargetCaps&,
                                             GrTexture**) {
    int      numOctaves = random->nextRangeU(2, 10);
    bool     stitchTiles = random->nextBool();
    SkScalar seed = SkIntToScalar(random->nextU());
    SkISize  tileSize = SkISize::Make(random->nextRangeU(4, 4096), random->nextRangeU(4, 4096));
    SkScalar baseFrequencyX = random->nextRangeScalar(SkFloatToScalar(0.01f),
                                                      SkFloatToScalar(0.99f));
    SkScalar baseFrequencyY = random->nextRangeScalar(SkFloatToScalar(0.01f),
                                                      SkFloatToScalar(0.99f));

    SkShader* shader = random->nextBool() ?
        SkPerlinNoiseShader::CreateFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                                stitchTiles ? &tileSize : NULL) :
        SkPerlinNoiseShader::CreateTubulence(baseFrequencyX, baseFrequencyY, numOctaves, seed,
                                             stitchTiles ? &tileSize : NULL);

    SkPaint paint;
    GrEffectRef* effect = shader->asNewEffect(context, paint);

    SkDELETE(shader);

    return effect;
}
#endif
/////////////////////////////////////////////////////////////////////

void GrGLPerlinNoise::emitCode(GrGLShaderBuilder* builder,
                               const GrDrawEffect&,
                               EffectKey key,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);

    const char* vCoords;
    fEffectMatrix.emitCodeMakeFSCoords2D(builder, key, &vCoords);

    fInvMatrixUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                        kMat33f_GrSLType, "invMatrix");
    const char* invMatrixUni = builder->getUniformCStr(fInvMatrixUni);
    fBaseFrequencyUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                            kVec2f_GrSLType, "baseFrequency");
    const char* baseFrequencyUni = builder->getUniformCStr(fBaseFrequencyUni);
    fAlphaUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                    kFloat_GrSLType, "alpha");
    const char* alphaUni = builder->getUniformCStr(fAlphaUni);

    const char* stitchDataUni = NULL;
    if (fStitchTiles) {
        fStitchDataUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec4f_GrSLType, "stitchData");
        stitchDataUni = builder->getUniformCStr(fStitchDataUni);
    }

    const char* chanCoords  = "chanCoords";
    const char* stitchData  = "stitchData";
    const char* ratio       = "ratio";
    const char* noise       = "noise";
    const char* noiseXY     = "noiseXY";
    const char* noiseVec    = "noiseVec";
    const char* noiseVecIni = "noiseVecIni";
    const char* noiseSmooth = "noiseSmooth";
    const char* fractVal    = "fractVal";
    const char* uv          = "uv";
    const char* ab          = "ab";
    const char* latticeIdx  = "latticeIdx";
    const char* lattice     = "lattice";
    const char* perlinNoise = "4096.0";
    const char* inc8bit     = "0.00390625";  // 1.0 / 256.0
    // This is the math to convert the two 16bit integer packed into rgba 8 bit input into a
    // [-1,1] vector and perform a dot product between that vector and the provided vector.
    const char* dotLattice  = "dot(((%s.ga + %s.rb * vec2(%s)) * vec2(2.0) - vec2(1.0)), %s);";

    // There are 4 lines, so the center of each line is 1/8, 3/8, 5/8 and 7/8
    builder->fsCodeAppendf("\t\tconst vec4 %s = vec4(0.125, 0.375, 0.625, 0.875);", chanCoords);

    // There are rounding errors if the floor operation is not performed here
    builder->fsCodeAppendf("\t\tvec2 %s = floor((%s*vec3(%s, 1.0)).xy) * %s;",
                           noiseVecIni, invMatrixUni, vCoords, baseFrequencyUni);

    // Loop over the 4 channels
    builder->fsCodeAppend("\t\tfor (int channel = 3; channel >= 0; --channel) {");

    if (fStitchTiles) {
        // Set up TurbulenceInitial stitch values.
        builder->fsCodeAppendf("\t\tvec4 %s = %s;", stitchData, stitchDataUni);
    }
    
    builder->fsCodeAppendf("\t\t%s[channel] = 0.0;", outputColor);

    builder->fsCodeAppendf("\t\tfloat %s = 1.0;", ratio);
    builder->fsCodeAppendf("\t\tvec2 %s = %s;", noiseVec, noiseVecIni);

    // Loop over all octaves
    builder->fsCodeAppendf("\t\tfor (int octave = 0; octave < %d; ++octave) {", fNumOctaves);

    builder->fsCodeAppendf("\t\tvec4 %s = vec4(floor(%s) + vec2(%s), fract(%s));",
                  noiseXY, noiseVec, perlinNoise, noiseVec);

    // smooth curve : t * t * (3 - 2 * t)
    builder->fsCodeAppendf("\t\tvec2 %s = %s.zw * %s.zw * (vec2(3.0) - vec2(2.0) * %s.zw);",
                  noiseSmooth, noiseXY, noiseXY, noiseXY);

    // Adjust frequencies if we're stitching tiles
    if (fStitchTiles) {
        builder->fsCodeAppendf("\t\tif(%s.x >= %s.y) { %s.x -= %s.x; }",
                      noiseXY, stitchData, noiseXY, stitchData);
        builder->fsCodeAppendf("\t\tif(%s.x >= (%s.y - 1.0)) { %s.x -= (%s.x - 1.0); }",
                      noiseXY, stitchData, noiseXY, stitchData);
        builder->fsCodeAppendf("\t\tif(%s.y >= %s.w) { %s.y -= %s.z; }",
                      noiseXY, stitchData, noiseXY, stitchData);
        builder->fsCodeAppendf("\t\tif(%s.y >= (%s.w - 1.0)) { %s.y -= (%s.z - 1.0); }",
                      noiseXY, stitchData, noiseXY, stitchData);
    }

    // Get texture coordinates and normalize
    builder->fsCodeAppendf("\t\t%s.xy = fract(floor(mod(%s.xy, 256.0)) / vec2(256.0));",
                  noiseXY, noiseXY);

    // Get permutation for x
    {
        SkString xCoords("");
        xCoords.appendf("vec2(%s.x, 0.5)", noiseXY);

        builder->fsCodeAppendf("\t\tvec2 %s;\t\t%s.x = ", latticeIdx, latticeIdx);
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[0], xCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppend(".r;\n");
    }

    // Get permutation for x + 1
    {
        SkString xCoords("");
        xCoords.appendf("vec2(fract(%s.x + %s), 0.5)", noiseXY, inc8bit);

        builder->fsCodeAppendf("\t\t%s.y = ", latticeIdx);
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[0], xCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppend(".r;\n");
    }

    // Get (x,y) coordinates with the permutated x
    builder->fsCodeAppendf("\t\t%s = fract(%s + %s.yy);", latticeIdx, latticeIdx, noiseXY);

    builder->fsCodeAppendf("\t\tvec2 %s = %s.zw;", fractVal, noiseXY);

    builder->fsCodeAppendf("\t\tvec2 %s;", uv);
    // Compute u, at offset (0,0)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.x, %s[channel])", latticeIdx, chanCoords);
        builder->fsCodeAppendf("vec4 %s = ", lattice);
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[1], latticeCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppendf(".bgra;\n\t\t%s.x = ", uv);
        builder->fsCodeAppendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    builder->fsCodeAppendf("\t\t%s.x -= 1.0;", fractVal);
    // Compute v, at offset (-1,0)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(%s.y, %s[channel])", latticeIdx, chanCoords);
        builder->fsCodeAppend("lattice = ");
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[1], latticeCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppendf(".bgra;\n\t\t%s.y = ", uv);
        builder->fsCodeAppendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    // Compute 'a' as a linear interpolation of 'u' and 'v'
    builder->fsCodeAppendf("\t\tvec2 %s;", ab);
    builder->fsCodeAppendf("\t\t%s.x = mix(%s.x, %s.y, %s.x);", ab, uv, uv, noiseSmooth);

    builder->fsCodeAppendf("\t\t%s.y -= 1.0;", fractVal);
    // Compute v, at offset (-1,-1)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(fract(%s.y + %s), %s[channel])",
            latticeIdx, inc8bit, chanCoords);
        builder->fsCodeAppend("lattice = ");
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[1], latticeCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppendf(".bgra;\n\t\t%s.y = ", uv);
        builder->fsCodeAppendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    builder->fsCodeAppendf("\t\t%s.x += 1.0;", fractVal);
    // Compute u, at offset (0,-1)
    {
        SkString latticeCoords("");
        latticeCoords.appendf("vec2(fract(%s.x + %s), %s[channel])",
            latticeIdx, inc8bit, chanCoords);
        builder->fsCodeAppend("lattice = ");
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                     samplers[1], latticeCoords.c_str(), kVec2f_GrSLType);
        builder->fsCodeAppendf(".bgra;\n\t\t%s.x = ", uv);
        builder->fsCodeAppendf(dotLattice, lattice, lattice, inc8bit, fractVal);
    }

    // Compute 'b' as a linear interpolation of 'u' and 'v'
    builder->fsCodeAppendf("\t\t%s.y = mix(%s.x, %s.y, %s.x);", ab, uv, uv, noiseSmooth);
    // Compute the noise as a linear interpolation of 'a' and 'b'
    builder->fsCodeAppendf("\t\tfloat %s = mix(%s.x, %s.y, %s.y);", noise, ab, ab, noiseSmooth);

    builder->fsCodeAppendf("\t\t%s[channel] += ", outputColor);
    builder->fsCodeAppendf((fType == SkPerlinNoiseShader::kFractalNoise_Type) ?
                  "%s / %s;" : "abs(%s) / %s;", noise, ratio);

    builder->fsCodeAppendf("\t\t%s *= vec2(2.0);", noiseVec);
    builder->fsCodeAppendf("\t\t%s *= 2.0;", ratio);

    if (fStitchTiles) {
        builder->fsCodeAppendf("\t\t%s.xz *= vec2(2.0);", stitchData);
        builder->fsCodeAppendf("\t\t%s.yw = %s.xz + vec2(%s);", stitchData, stitchData, perlinNoise);
    }
    builder->fsCodeAppend("\t\t}"); // end of the for loop on octaves

    builder->fsCodeAppend("\t\t}"); // end of the for loop on channels

    if (fType == SkPerlinNoiseShader::kFractalNoise_Type) {
        // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
        // by fractalNoise and (turbulenceFunctionResult) by turbulence.
        builder->fsCodeAppendf("\t\t%s = %s * vec4(0.5) + vec4(0.5);", outputColor, outputColor);
    }

    builder->fsCodeAppendf("\t\t%s.a *= %s;", outputColor, alphaUni);

    // Clamp values
    builder->fsCodeAppendf("\t\t%s = clamp(%s, 0.0, 1.0);", outputColor, outputColor);

    // Pre-multiply the result
    builder->fsCodeAppendf("\t\t%s = vec4(%s.rgb * %s.aaa, %s.a);\n",
                  outputColor, outputColor, outputColor, outputColor);
}

GrGLPerlinNoise::GrGLPerlinNoise(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
  : INHERITED (factory)
  , fType(drawEffect.castEffect<GrPerlinNoiseEffect>().type())
  , fStitchTiles(drawEffect.castEffect<GrPerlinNoiseEffect>().stitchTiles())
  , fNumOctaves(drawEffect.castEffect<GrPerlinNoiseEffect>().numOctaves())
  , fEffectMatrix(drawEffect.castEffect<GrPerlinNoiseEffect>().coordsType()) {
}

GrGLEffect::EffectKey GrGLPerlinNoise::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const GrPerlinNoiseEffect& turbulence = drawEffect.castEffect<GrPerlinNoiseEffect>();

    EffectKey key = turbulence.numOctaves();

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

    key = key << GrGLEffectMatrix::kKeyBits;

    SkMatrix m = turbulence.matrix();
    m.postTranslate(SK_Scalar1, SK_Scalar1);
    return key | GrGLEffectMatrix::GenKey(m, drawEffect,
                 drawEffect.castEffect<GrPerlinNoiseEffect>().coordsType(), NULL);
}

void GrGLPerlinNoise::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const GrPerlinNoiseEffect& turbulence = drawEffect.castEffect<GrPerlinNoiseEffect>();

    const SkVector& baseFrequency = turbulence.baseFrequency();
    uman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);
    if (turbulence.stitchTiles()) {
        const SkPerlinNoiseShader::StitchData& stitchData = turbulence.stitchData();
        uman.set4f(fStitchDataUni, SkIntToScalar(stitchData.fWidth),
                                   SkIntToScalar(stitchData.fWrapX),
                                   SkIntToScalar(stitchData.fHeight),
                                   SkIntToScalar(stitchData.fWrapY));
    }

    uman.set1f(fAlphaUni, SkScalarDiv(SkIntToScalar(turbulence.alpha()), SkIntToScalar(255)));

    SkMatrix m = turbulence.matrix();
    SkMatrix invM;
    if (!m.invert(&invM)) {
        invM.reset();
    } else {
        invM.postConcat(invM); // Square the matrix
    }
    uman.setSkMatrix(fInvMatrixUni, invM);

    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the shadeSpan() functions.
    m.postTranslate(SK_Scalar1, SK_Scalar1);
    fEffectMatrix.setData(uman, m, drawEffect, NULL);
}

/////////////////////////////////////////////////////////////////////

GrEffectRef* SkPerlinNoiseShader::asNewEffect(GrContext* context, const SkPaint& paint) const {
#if 0
    SkASSERT(NULL != context);

    // Either we don't stitch tiles, either we have a valid tile size
    SkASSERT(!fStitchTiles || !fTileSize.isEmpty());

    GrTexture* permutationsTexture = GrLockAndRefCachedBitmapTexture(
        context, *fPaintingData->getPermutationsBitmap(), NULL);
    GrTexture* noiseTexture = GrLockAndRefCachedBitmapTexture(
        context, *fPaintingData->getNoiseBitmap(), NULL);

    GrEffectRef* effect = (NULL != permutationsTexture) && (NULL != noiseTexture) ?
        GrPerlinNoiseEffect::Create(fType, fPaintingData->fBaseFrequency, 
                                    fNumOctaves, fStitchTiles,
                                    fPaintingData->fStitchDataInit,
                                    permutationsTexture, noiseTexture, 
                                    this->getLocalMatrix(), paint.getAlpha()) :
        NULL;

    // Unlock immediately, this is not great, but we don't have a way of
    // knowing when else to unlock it currently. TODO: Remove this when
    // unref becomes the unlock replacement for all types of textures.
    if (NULL != permutationsTexture) {
        GrUnlockAndUnrefCachedBitmapTexture(permutationsTexture);
    }
    if (NULL != noiseTexture) {
        GrUnlockAndUnrefCachedBitmapTexture(noiseTexture);
    }

    return effect;
#else
    sk_ignore_unused_variable(context);
    sk_ignore_unused_variable(paint);
    return NULL;
#endif
}

#else

GrEffectRef* SkPerlinNoiseShader::asNewEffect(GrContext*, const SkPaint&) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif

#ifdef SK_DEVELOPER
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
