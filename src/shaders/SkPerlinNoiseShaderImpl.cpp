/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkPerlinNoiseShaderImpl.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

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

inline SkScalar smoothCurve(SkScalar t) { return t * t * (3 - 2 * t); }

}  // end namespace

SkPerlinNoiseShader::SkPerlinNoiseShader(SkPerlinNoiseShader::Type type,
                                         SkScalar baseFrequencyX,
                                         SkScalar baseFrequencyY,
                                         int numOctaves,
                                         SkScalar seed,
                                         const SkISize* tileSize)
        : fType(type)
        , fBaseFrequencyX(baseFrequencyX)
        , fBaseFrequencyY(baseFrequencyY)
        , fNumOctaves(numOctaves > kMaxOctaves ? kMaxOctaves
                                               : numOctaves)  //[0,255] octaves allowed
        , fSeed(seed)
        , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
        , fStitchTiles(!fTileSize.isEmpty()) {
    SkASSERT(numOctaves >= 0 && numOctaves <= kMaxOctaves);
    SkASSERT(fBaseFrequencyX >= 0);
    SkASSERT(fBaseFrequencyY >= 0);

    // If kBlockSize changes then it must be changed in the SkSL noise_function
    // implementation and the graphite backend
    static_assert(SkPerlinNoiseShader::kBlockSize == 256);
}

sk_sp<SkFlattenable> SkPerlinNoiseShader::CreateProc(SkReadBuffer& buffer) {
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
            return SkShaders::MakeFractalNoise(freqX, freqY, octaves, seed, &tileSize);
        case kTurbulence_Type:
            return SkShaders::MakeTurbulence(freqX, freqY, octaves, seed, &tileSize);
        default:
            // Really shouldn't get here b.c. of earlier check on type
            buffer.validate(false);
            return nullptr;
    }
}

void SkPerlinNoiseShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int)fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::noise2D(int channel,
                                                                const StitchData& stitchData,
                                                                const SkPoint& noiseVector) const {
    struct Noise {
        int noisePositionIntegerValue;
        int nextNoisePositionIntegerValue;
        SkScalar noisePositionFractionValue;
        Noise(SkScalar component) {
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
        noiseX.nextNoisePositionIntegerValue = checkNoise(
                noiseX.nextNoisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.nextNoisePositionIntegerValue = checkNoise(
                noiseY.nextNoisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
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
                                          noiseY.noisePositionFractionValue);  // Offset (0,0)
    u = fPaintingData.fGradient[channel][b00].dot(fractionValue);
    fractionValue.fX -= SK_Scalar1;  // Offset (-1,0)
    v = fPaintingData.fGradient[channel][b10].dot(fractionValue);
    SkScalar a = SkScalarInterp(u, v, sx);
    fractionValue.fY -= SK_Scalar1;  // Offset (-1,-1)
    v = fPaintingData.fGradient[channel][b11].dot(fractionValue);
    fractionValue.fX = noiseX.noisePositionFractionValue;  // Offset (0,-1)
    u = fPaintingData.fGradient[channel][b01].dot(fractionValue);
    SkScalar b = SkScalarInterp(u, v, sx);
    return SkScalarInterp(a, b, sy);
}

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::calculateTurbulenceValueForPoint(
        int channel, StitchData& stitchData, const SkPoint& point) const {
    const SkPerlinNoiseShader& perlinNoiseShader = static_cast<const SkPerlinNoiseShader&>(fShader);
    if (perlinNoiseShader.fStitchTiles) {
        stitchData = fPaintingData.fStitchDataInit;
    }
    SkScalar turbulenceFunctionResult = 0;
    SkPoint noiseVector(SkPoint::Make(point.x() * fPaintingData.fBaseFrequency.fX,
                                      point.y() * fPaintingData.fBaseFrequency.fY));
    SkScalar ratio = SK_Scalar1;
    for (int octave = 0; octave < perlinNoiseShader.fNumOctaves; ++octave) {
        SkScalar noise = noise2D(channel, stitchData, noiseVector);
        SkScalar numer =
                (perlinNoiseShader.fType == kFractalNoise_Type) ? noise : SkScalarAbs(noise);
        turbulenceFunctionResult += numer / ratio;
        noiseVector.fX *= 2;
        noiseVector.fY *= 2;
        ratio *= 2;
        if (perlinNoiseShader.fStitchTiles) {
            stitchData = StitchData(SkIntToScalar(stitchData.fWidth) * 2,
                                    SkIntToScalar(stitchData.fHeight) * 2);
        }
    }

    if (perlinNoiseShader.fType == kFractalNoise_Type) {
        // For kFractalNoise the result is: noise[-1,1] * 0.5 + 0.5
        turbulenceFunctionResult = SkScalarHalf(turbulenceFunctionResult + 1);
    }

    if (channel == 3) {  // Scale alpha by paint value
        turbulenceFunctionResult *= SkIntToScalar(getPaintAlpha()) / 255;
    }

    // Clamp result
    return SkTPin(turbulenceFunctionResult, 0.0f, SK_Scalar1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SkPMColor SkPerlinNoiseShader::PerlinNoiseShaderContext::shade(const SkPoint& point,
                                                               StitchData& stitchData) const {
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
SkShaderBase::Context* SkPerlinNoiseShader::onMakeContext(const ContextRec& rec,
                                                          SkArenaAlloc* alloc) const {
    // should we pay attention to rec's device-colorspace?
    return alloc->make<PerlinNoiseShaderContext>(*this, rec);
}
#endif

SkPerlinNoiseShader::PerlinNoiseShaderContext::PerlinNoiseShaderContext(
        const SkPerlinNoiseShader& shader, const ContextRec& rec)
        : Context(shader, rec)
        , fMatrix(rec.fMatrixRec.totalMatrix())  // used for temp storage, adjusted below
        , fPaintingData(shader.fTileSize,
                        shader.fSeed,
                        shader.fBaseFrequencyX,
                        shader.fBaseFrequencyY,
                        fMatrix) {
    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the setData() function.
    fMatrix.setTranslate(-fMatrix.getTranslateX() + SK_Scalar1,
                         -fMatrix.getTranslateY() + SK_Scalar1);
}

void SkPerlinNoiseShader::PerlinNoiseShaderContext::shadeSpan(int x,
                                                              int y,
                                                              SkPMColor result[],
                                                              int count) {
    SkPoint point = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
    StitchData stitchData;
    for (int i = 0; i < count; ++i) {
        result[i] = shade(point, stitchData);
        point.fX += SK_Scalar1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_input(
        SkScalar baseX, SkScalar baseY, int numOctaves, const SkISize* tileSize, SkScalar seed) {
    if (!(baseX >= 0 && baseY >= 0)) {
        return false;
    }
    if (!(numOctaves >= 0 && numOctaves <= SkPerlinNoiseShader::kMaxOctaves)) {
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

void SkRegisterPerlinNoiseShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPerlinNoiseShader);
    // Previous name
    SkFlattenable::Register("SkPerlinNoiseShaderImpl", SkPerlinNoiseShader::CreateProc);
}

namespace SkShaders {
sk_sp<SkShader> MakeFractalNoise(SkScalar baseFrequencyX,
                                 SkScalar baseFrequencyY,
                                 int numOctaves,
                                 SkScalar seed,
                                 const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kFractalNoise, w/o any octaves, the entire shader collapses to:
        //    [0,0,0,0] * 0.5 + 0.5
        constexpr SkColor4f kTransparentGray = {0.5f, 0.5f, 0.5f, 0.5f};

        return SkShaders::Color(kTransparentGray, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShader::kFractalNoise_Type,
                                                   baseFrequencyX,
                                                   baseFrequencyY,
                                                   numOctaves,
                                                   seed,
                                                   tileSize));
}

sk_sp<SkShader> MakeTurbulence(SkScalar baseFrequencyX,
                               SkScalar baseFrequencyY,
                               int numOctaves,
                               SkScalar seed,
                               const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kTurbulence, w/o any octaves, the entire shader collapses to: [0,0,0,0]
        return SkShaders::Color(SkColors::kTransparent, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShader::kTurbulence_Type,
                                                   baseFrequencyX,
                                                   baseFrequencyY,
                                                   numOctaves,
                                                   seed,
                                                   tileSize));
}

}  // namespace SkShaders
