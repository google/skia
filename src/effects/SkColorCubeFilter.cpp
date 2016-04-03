/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorCubeFilter.h"
#include "SkColorPriv.h"
#include "SkOnce.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrTexturePriv.h"
#include "SkGr.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

///////////////////////////////////////////////////////////////////////////////
namespace {

int32_t SkNextColorCubeUniqueID() {
    static int32_t gColorCubeUniqueID;
    // do a loop in case our global wraps around, as we never want to return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gColorCubeUniqueID) + 1;
    } while (0 == genID);
    return genID;
}

} // end namespace

static const int MIN_CUBE_SIZE = 4;
static const int MAX_CUBE_SIZE = 64;

static bool is_valid_3D_lut(SkData* cubeData, int cubeDimension) {
    size_t minMemorySize = sizeof(uint8_t) * 4 * cubeDimension * cubeDimension * cubeDimension;
    return (cubeDimension >= MIN_CUBE_SIZE) && (cubeDimension <= MAX_CUBE_SIZE) &&
           (nullptr != cubeData) && (cubeData->size() >= minMemorySize);
}

sk_sp<SkColorFilter> SkColorCubeFilter::Make(sk_sp<SkData> cubeData, int cubeDimension) {
    if (!is_valid_3D_lut(cubeData.get(), cubeDimension)) {
        return nullptr;
    }

    return sk_sp<SkColorFilter>(new SkColorCubeFilter(std::move(cubeData), cubeDimension));
}

SkColorCubeFilter::SkColorCubeFilter(sk_sp<SkData> cubeData, int cubeDimension)
    : fCubeData(std::move(cubeData))
    , fUniqueID(SkNextColorCubeUniqueID())
    , fCache(cubeDimension)
{}

uint32_t SkColorCubeFilter::getFlags() const {
    return this->INHERITED::getFlags() | kAlphaUnchanged_Flag;
}

SkColorCubeFilter::ColorCubeProcesingCache::ColorCubeProcesingCache(int cubeDimension)
  : fCubeDimension(cubeDimension)
  , fLutsInited(false) {
    fColorToIndex[0] = fColorToIndex[1] = nullptr;
    fColorToFactors[0] = fColorToFactors[1] = nullptr;
    fColorToScalar = nullptr;
}

void SkColorCubeFilter::ColorCubeProcesingCache::getProcessingLuts(
    const int* (*colorToIndex)[2], const SkScalar* (*colorToFactors)[2],
    const SkScalar** colorToScalar) {
    SkOnce(&fLutsInited, &fLutsMutex,
           SkColorCubeFilter::ColorCubeProcesingCache::initProcessingLuts, this);
    SkASSERT((fColorToIndex[0] != nullptr) &&
             (fColorToIndex[1] != nullptr) &&
             (fColorToFactors[0] != nullptr) &&
             (fColorToFactors[1] != nullptr) &&
             (fColorToScalar != nullptr));
    (*colorToIndex)[0] = fColorToIndex[0];
    (*colorToIndex)[1] = fColorToIndex[1];
    (*colorToFactors)[0] = fColorToFactors[0];
    (*colorToFactors)[1] = fColorToFactors[1];
    (*colorToScalar) = fColorToScalar;
}

void SkColorCubeFilter::ColorCubeProcesingCache::initProcessingLuts(
    SkColorCubeFilter::ColorCubeProcesingCache* cache) {
    static const SkScalar inv8bit = SkScalarInvert(SkIntToScalar(255));

    // We need 256 int * 2 for fColorToIndex, so a total of 512 int.
    // We need 256 SkScalar * 2 for fColorToFactors and 256 SkScalar
    // for fColorToScalar, so a total of 768 SkScalar.
    cache->fLutStorage.reset(512 * sizeof(int) + 768 * sizeof(SkScalar));
    uint8_t* storage = cache->fLutStorage.get();
    cache->fColorToIndex[0] = (int*)storage;
    cache->fColorToIndex[1] = cache->fColorToIndex[0] + 256;
    cache->fColorToFactors[0] = (SkScalar*)(storage + (512 * sizeof(int)));
    cache->fColorToFactors[1] = cache->fColorToFactors[0] + 256;
    cache->fColorToScalar = cache->fColorToFactors[1] + 256;

    SkScalar size = SkIntToScalar(cache->fCubeDimension);
    SkScalar scale = (size - SK_Scalar1) * inv8bit;

    for (int i = 0; i < 256; ++i) {
        SkScalar index = scale * i;
        cache->fColorToIndex[0][i] = SkScalarFloorToInt(index);
        cache->fColorToIndex[1][i] = cache->fColorToIndex[0][i] + 1;
        cache->fColorToScalar[i] = inv8bit * i;
        if (cache->fColorToIndex[1][i] < cache->fCubeDimension) {
            cache->fColorToFactors[1][i] = index - SkIntToScalar(cache->fColorToIndex[0][i]);
            cache->fColorToFactors[0][i] = SK_Scalar1 - cache->fColorToFactors[1][i];
        } else {
            cache->fColorToIndex[1][i] = cache->fColorToIndex[0][i];
            cache->fColorToFactors[0][i] = SK_Scalar1;
            cache->fColorToFactors[1][i] = 0;
        }
    }
}

void SkColorCubeFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    const int* colorToIndex[2];
    const SkScalar* colorToFactors[2];
    const SkScalar* colorToScalar;
    fCache.getProcessingLuts(&colorToIndex, &colorToFactors, &colorToScalar);

    SkOpts::color_cube_filter_span(src, count, dst, colorToIndex,
                                   colorToFactors, fCache.cubeDimension(),
                                   (const SkColor*)fCubeData->data());
}

sk_sp<SkFlattenable> SkColorCubeFilter::CreateProc(SkReadBuffer& buffer) {
    int cubeDimension = buffer.readInt();
    auto cubeData(buffer.readByteArrayAsData());
    if (!buffer.validate(is_valid_3D_lut(cubeData.get(), cubeDimension))) {
        return nullptr;
    }
    return Make(std::move(cubeData), cubeDimension);
}

void SkColorCubeFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fCache.cubeDimension());
    buffer.writeDataAsByteArray(fCubeData.get());
}

#ifndef SK_IGNORE_TO_STRING
void SkColorCubeFilter::toString(SkString* str) const {
    str->append("SkColorCubeFilter ");
}
#endif

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU

class GrColorCubeEffect : public GrFragmentProcessor {
public:
    static const GrFragmentProcessor* Create(GrTexture* colorCube) {
        return (nullptr != colorCube) ? new GrColorCubeEffect(colorCube) : nullptr;
    }

    virtual ~GrColorCubeEffect();

    const char* name() const override { return "ColorCube"; }

    int colorCubeSize() const { return fColorCubeAccess.getTexture()->width(); }


    void onComputeInvariantOutput(GrInvariantOutput*) const override;

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs&) override;

        static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

    protected:
        void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    private:
        GrGLSLProgramDataManager::UniformHandle fColorCubeSizeUni;
        GrGLSLProgramDataManager::UniformHandle fColorCubeInvSizeUni;

        typedef GrGLSLFragmentProcessor INHERITED;
    };

private:
    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override;

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GrColorCubeEffect(GrTexture* colorCube);

    GrTextureAccess     fColorCubeAccess;

    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrColorCubeEffect::GrColorCubeEffect(GrTexture* colorCube)
    : fColorCubeAccess(colorCube, GrTextureParams::kBilerp_FilterMode) {
    this->initClassID<GrColorCubeEffect>();
    this->addTextureAccess(&fColorCubeAccess);
}

GrColorCubeEffect::~GrColorCubeEffect() {
}

void GrColorCubeEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GLSLProcessor::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrColorCubeEffect::onCreateGLSLInstance() const {
    return new GLSLProcessor;
}

void GrColorCubeEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

void GrColorCubeEffect::GLSLProcessor::emitCode(EmitArgs& args) {
    if (nullptr == args.fInputColor) {
        args.fInputColor = "vec4(1)";
    }

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fColorCubeSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kFloat_GrSLType, kDefault_GrSLPrecision,
                                                   "Size");
    const char* colorCubeSizeUni = uniformHandler->getUniformCStr(fColorCubeSizeUni);
    fColorCubeInvSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                      kFloat_GrSLType, kDefault_GrSLPrecision,
                                                      "InvSize");
    const char* colorCubeInvSizeUni = uniformHandler->getUniformCStr(fColorCubeInvSizeUni);

    const char* nonZeroAlpha = "nonZeroAlpha";
    const char* unPMColor = "unPMColor";
    const char* cubeIdx = "cubeIdx";
    const char* cCoords1 = "cCoords1";
    const char* cCoords2 = "cCoords2";

    // Note: if implemented using texture3D in OpenGL ES older than OpenGL ES 3.0,
    //       the shader might need "#extension GL_OES_texture_3D : enable".

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    // Unpremultiply color
    fragBuilder->codeAppendf("\tfloat %s = max(%s.a, 0.00001);\n", nonZeroAlpha, args.fInputColor);
    fragBuilder->codeAppendf("\tvec4 %s = vec4(%s.rgb / %s, %s);\n",
                             unPMColor, args.fInputColor, nonZeroAlpha, nonZeroAlpha);

    // Fit input color into the cube.
    fragBuilder->codeAppendf(
        "vec3 %s = vec3(%s.rg * vec2((%s - 1.0) * %s) + vec2(0.5 * %s), %s.b * (%s - 1.0));\n",
        cubeIdx, unPMColor, colorCubeSizeUni, colorCubeInvSizeUni, colorCubeInvSizeUni,
        unPMColor, colorCubeSizeUni);

    // Compute y coord for for texture fetches.
    fragBuilder->codeAppendf("vec2 %s = vec2(%s.r, (floor(%s.b) + %s.g) * %s);\n",
                             cCoords1, cubeIdx, cubeIdx, cubeIdx, colorCubeInvSizeUni);
    fragBuilder->codeAppendf("vec2 %s = vec2(%s.r, (ceil(%s.b) + %s.g) * %s);\n",
                             cCoords2, cubeIdx, cubeIdx, cubeIdx, colorCubeInvSizeUni);

    // Apply the cube.
    fragBuilder->codeAppendf("%s = vec4(mix(", args.fOutputColor);
    fragBuilder->appendTextureLookup(args.fSamplers[0], cCoords1);
    fragBuilder->codeAppend(".bgr, ");
    fragBuilder->appendTextureLookup(args.fSamplers[0], cCoords2);

    // Premultiply color by alpha. Note that the input alpha is not modified by this shader.
    fragBuilder->codeAppendf(".bgr, fract(%s.b)) * vec3(%s), %s.a);\n",
                             cubeIdx, nonZeroAlpha, args.fInputColor);
}

void GrColorCubeEffect::GLSLProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                 const GrProcessor& proc) {
    const GrColorCubeEffect& colorCube = proc.cast<GrColorCubeEffect>();
    SkScalar size = SkIntToScalar(colorCube.colorCubeSize());
    pdman.set1f(fColorCubeSizeUni, SkScalarToFloat(size));
    pdman.set1f(fColorCubeInvSizeUni, SkScalarToFloat(SkScalarInvert(size)));
}

void GrColorCubeEffect::GLSLProcessor::GenKey(const GrProcessor& proc,
                                              const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
}

const GrFragmentProcessor* SkColorCubeFilter::asFragmentProcessor(GrContext* context) const {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 2);
    builder[0] = fUniqueID;
    builder[1] = fCache.cubeDimension();
    builder.finish();

    GrSurfaceDesc desc;
    desc.fWidth = fCache.cubeDimension();
    desc.fHeight = fCache.cubeDimension() * fCache.cubeDimension();
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fIsMipMapped = false;

    SkAutoTUnref<GrTexture> textureCube(
        context->textureProvider()->findAndRefTextureByUniqueKey(key));
    if (!textureCube) {
        textureCube.reset(context->textureProvider()->createTexture(
            desc, SkBudgeted::kYes, fCubeData->data(), 0));
        if (textureCube) {
            context->textureProvider()->assignUniqueKeyToTexture(key, textureCube);
        } else {
            return nullptr;
        }
    }

    return GrColorCubeEffect::Create(textureCube);
}
#endif
