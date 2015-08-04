/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorCubeFilter.h"
#include "SkColorPriv.h"
#include "SkOnce.h"
#include "SkReadBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrTexturePriv.h"
#include "SkGr.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
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
           (NULL != cubeData) && (cubeData->size() >= minMemorySize);
}

SkColorFilter* SkColorCubeFilter::Create(SkData* cubeData, int cubeDimension) {
    if (!is_valid_3D_lut(cubeData, cubeDimension)) {
        return NULL;
    }

    return SkNEW_ARGS(SkColorCubeFilter, (cubeData, cubeDimension));
}

SkColorCubeFilter::SkColorCubeFilter(SkData* cubeData, int cubeDimension)
  : fCubeData(SkRef(cubeData))
  , fUniqueID(SkNextColorCubeUniqueID())
  , fCache(cubeDimension) {
}

uint32_t SkColorCubeFilter::getFlags() const {
    return this->INHERITED::getFlags() | kAlphaUnchanged_Flag;
}

SkColorCubeFilter::ColorCubeProcesingCache::ColorCubeProcesingCache(int cubeDimension)
  : fCubeDimension(cubeDimension)
  , fLutsInited(false) {
    fColorToIndex[0] = fColorToIndex[1] = NULL;
    fColorToFactors[0] = fColorToFactors[1] = NULL;
    fColorToScalar = NULL;
}

void SkColorCubeFilter::ColorCubeProcesingCache::getProcessingLuts(
    const int* (*colorToIndex)[2], const SkScalar* (*colorToFactors)[2],
    const SkScalar** colorToScalar) {
    SkOnce(&fLutsInited, &fLutsMutex,
           SkColorCubeFilter::ColorCubeProcesingCache::initProcessingLuts, this);
    SkASSERT((fColorToIndex[0] != NULL) &&
             (fColorToIndex[1] != NULL) &&
             (fColorToFactors[0] != NULL) &&
             (fColorToFactors[1] != NULL) &&
             (fColorToScalar != NULL));
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
    uint8_t* storage = (uint8_t*)cache->fLutStorage.get();
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

    const int dim = fCache.cubeDimension();
    SkColor* colorCube = (SkColor*)fCubeData->data();
    for (int i = 0; i < count; ++i) {
        SkColor inputColor = SkUnPreMultiply::PMColorToColor(src[i]);
        uint8_t r = SkColorGetR(inputColor);
        uint8_t g = SkColorGetG(inputColor);
        uint8_t b = SkColorGetB(inputColor);
        uint8_t a = SkColorGetA(inputColor);
        SkScalar rOut(0), gOut(0), bOut(0);
        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 2; ++y) {
                for (int z = 0; z < 2; ++z) {
                    SkColor lutColor = colorCube[colorToIndex[x][r] +
                                                (colorToIndex[y][g] +
                                                 colorToIndex[z][b] * dim) * dim];
                    SkScalar factor = colorToFactors[x][r] *
                                      colorToFactors[y][g] *
                                      colorToFactors[z][b];
                    rOut += colorToScalar[SkColorGetR(lutColor)] * factor;
                    gOut += colorToScalar[SkColorGetG(lutColor)] * factor;
                    bOut += colorToScalar[SkColorGetB(lutColor)] * factor;
                }
            }
        }
        const SkScalar aOut = SkIntToScalar(a);
        dst[i] = SkPackARGB32(a,
            SkScalarRoundToInt(rOut * aOut),
            SkScalarRoundToInt(gOut * aOut),
            SkScalarRoundToInt(bOut * aOut));
    }
}

SkFlattenable* SkColorCubeFilter::CreateProc(SkReadBuffer& buffer) {
    int cubeDimension = buffer.readInt();
    SkAutoDataUnref cubeData(buffer.readByteArrayAsData());
    if (!buffer.validate(is_valid_3D_lut(cubeData, cubeDimension))) {
        return NULL;
    }
    return Create(cubeData, cubeDimension);
}

void SkColorCubeFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fCache.cubeDimension());
    buffer.writeDataAsByteArray(fCubeData);
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
    static GrFragmentProcessor* Create(GrTexture* colorCube) {
        return (NULL != colorCube) ? SkNEW_ARGS(GrColorCubeEffect, (colorCube)) : NULL;
    }

    virtual ~GrColorCubeEffect();

    const char* name() const override { return "ColorCube"; }

    GrGLFragmentProcessor* createGLInstance() const override;
    int colorCubeSize() const { return fColorCubeAccess.getTexture()->width(); }


    void onComputeInvariantOutput(GrInvariantOutput*) const override;

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        GLProcessor(const GrProcessor&);
        virtual ~GLProcessor();

        virtual void emitCode(EmitArgs&) override;

        static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

        void setData(const GrGLProgramDataManager&, const GrProcessor&) override;

    private:
        GrGLProgramDataManager::UniformHandle fColorCubeSizeUni;
        GrGLProgramDataManager::UniformHandle fColorCubeInvSizeUni;

        typedef GrGLFragmentProcessor INHERITED;
    };

private:
    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GrColorCubeEffect(GrTexture* colorCube);

    GrTextureAccess     fColorCubeAccess;

    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrColorCubeEffect::GrColorCubeEffect(GrTexture* colorCube)
    : fColorCubeAccess(colorCube, "bgra", GrTextureParams::kBilerp_FilterMode) {
    this->initClassID<GrColorCubeEffect>();
    this->addTextureAccess(&fColorCubeAccess);
}

GrColorCubeEffect::~GrColorCubeEffect() {
}

void GrColorCubeEffect::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLProcessor::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* GrColorCubeEffect::createGLInstance() const {
    return SkNEW_ARGS(GLProcessor, (*this));
}

void GrColorCubeEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GrColorCubeEffect::GLProcessor::GLProcessor(const GrProcessor&) {
}

GrColorCubeEffect::GLProcessor::~GLProcessor() {
}

void GrColorCubeEffect::GLProcessor::emitCode(EmitArgs& args) {
    if (NULL == args.fInputColor) {
        args.fInputColor = "vec4(1)";
    }

    fColorCubeSizeUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                            kFloat_GrSLType, kDefault_GrSLPrecision,
                                            "Size");
    const char* colorCubeSizeUni = args.fBuilder->getUniformCStr(fColorCubeSizeUni);
    fColorCubeInvSizeUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                               kFloat_GrSLType, kDefault_GrSLPrecision,
                                               "InvSize");
    const char* colorCubeInvSizeUni = args.fBuilder->getUniformCStr(fColorCubeInvSizeUni);

    const char* nonZeroAlpha = "nonZeroAlpha";
    const char* unPMColor = "unPMColor";
    const char* cubeIdx = "cubeIdx";
    const char* cCoords1 = "cCoords1";
    const char* cCoords2 = "cCoords2";

    // Note: if implemented using texture3D in OpenGL ES older than OpenGL ES 3.0,
    //       the shader might need "#extension GL_OES_texture_3D : enable".

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();

    // Unpremultiply color
    fsBuilder->codeAppendf("\tfloat %s = max(%s.a, 0.00001);\n", nonZeroAlpha, args.fInputColor);
    fsBuilder->codeAppendf("\tvec4 %s = vec4(%s.rgb / %s, %s);\n",
                           unPMColor, args.fInputColor, nonZeroAlpha, nonZeroAlpha);

    // Fit input color into the cube.
    fsBuilder->codeAppendf(
        "vec3 %s = vec3(%s.rg * vec2((%s - 1.0) * %s) + vec2(0.5 * %s), %s.b * (%s - 1.0));\n",
        cubeIdx, unPMColor, colorCubeSizeUni, colorCubeInvSizeUni, colorCubeInvSizeUni,
        unPMColor, colorCubeSizeUni);

    // Compute y coord for for texture fetches.
    fsBuilder->codeAppendf("vec2 %s = vec2(%s.r, (floor(%s.b) + %s.g) * %s);\n",
                           cCoords1, cubeIdx, cubeIdx, cubeIdx, colorCubeInvSizeUni);
    fsBuilder->codeAppendf("vec2 %s = vec2(%s.r, (ceil(%s.b) + %s.g) * %s);\n",
                           cCoords2, cubeIdx, cubeIdx, cubeIdx, colorCubeInvSizeUni);

    // Apply the cube.
    fsBuilder->codeAppendf("%s = vec4(mix(", args.fOutputColor);
    fsBuilder->appendTextureLookup(args.fSamplers[0], cCoords1);
    fsBuilder->codeAppend(".rgb, ");
    fsBuilder->appendTextureLookup(args.fSamplers[0], cCoords2);

    // Premultiply color by alpha. Note that the input alpha is not modified by this shader.
    fsBuilder->codeAppendf(".rgb, fract(%s.b)) * vec3(%s), %s.a);\n",
                           cubeIdx, nonZeroAlpha, args.fInputColor);
}

void GrColorCubeEffect::GLProcessor::setData(const GrGLProgramDataManager& pdman,
                                             const GrProcessor& proc) {
    const GrColorCubeEffect& colorCube = proc.cast<GrColorCubeEffect>();
    SkScalar size = SkIntToScalar(colorCube.colorCubeSize());
    pdman.set1f(fColorCubeSizeUni, SkScalarToFloat(size));
    pdman.set1f(fColorCubeInvSizeUni, SkScalarToFloat(SkScalarInvert(size)));
}

void GrColorCubeEffect::GLProcessor::GenKey(const GrProcessor& proc,
                                            const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
}

bool SkColorCubeFilter::asFragmentProcessors(GrContext* context, GrProcessorDataManager*,
                                             SkTDArray<GrFragmentProcessor*>* array) const {
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

    SkAutoTUnref<GrTexture> textureCube(
        context->textureProvider()->findAndRefTextureByUniqueKey(key));
    if (!textureCube) {
        textureCube.reset(context->textureProvider()->createTexture(
            desc, true, fCubeData->data(), 0));
        if (textureCube) {
            context->textureProvider()->assignUniqueKeyToTexture(key, textureCube);
        }
    }

    GrFragmentProcessor* frag = textureCube ? GrColorCubeEffect::Create(textureCube) : NULL;
    if (frag) {
        if (array) {
            *array->append() = frag;
        } else {
            frag->unref();
            SkDEBUGCODE(frag = NULL;)
        }
        return true;
    }
    return false;
}
#endif
