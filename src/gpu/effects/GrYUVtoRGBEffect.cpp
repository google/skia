/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrYUVtoRGBEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

static const float kJPEGConversionMatrix[16] = {
    1.0f,  0.0f,       1.402f,    -0.703749f,
    1.0f, -0.344136f, -0.714136f,  0.531211f,
    1.0f,  1.772f,     0.0f,      -0.889475f,
    0.0f,  0.0f,       0.0f,       1.0
};

static const float kRec601ConversionMatrix[16] = {
    1.164f,  0.0f,    1.596f, -0.87075f,
    1.164f, -0.391f, -0.813f,  0.52925f,
    1.164f,  2.018f,  0.0f,   -1.08175f,
    0.0f,    0.0f,    0.0f,    1.0
};

static const float kRec709ConversionMatrix[16] = {
    1.164f,  0.0f,    1.793f, -0.96925f,
    1.164f, -0.213f, -0.533f,  0.30025f,
    1.164f,  2.112f,  0.0f,   -1.12875f,
    0.0f,    0.0f,    0.0f,    1.0f
};

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::Make(const sk_sp<GrTextureProxy> proxies[],
                                                            const SkYUVAIndex yuvaIndices[4],
                                                            SkYUVColorSpace yuvColorSpace,
                                                            GrSamplerState::Filter filterMode,
                                                            const SkMatrix& localMatrix,
                                                            const SkRect* domain) {
    int numPlanes;
    SkAssertResult(SkYUVAIndex::AreValidIndices(yuvaIndices, &numPlanes));

    const SkISize YSize = proxies[yuvaIndices[SkYUVAIndex::kY_Index].fIndex]->isize();

    GrSamplerState::Filter minimizeFilterMode = GrSamplerState::Filter::kMipMap == filterMode ?
                                                GrSamplerState::Filter::kMipMap :
                                                GrSamplerState::Filter::kBilerp;

    GrSamplerState::Filter filterModes[4];
    SkSize scales[4];
    for (int i = 0; i < numPlanes; ++i) {
        SkISize size = proxies[i]->isize();
        scales[i] = SkSize::Make(SkIntToScalar(size.width()) / SkIntToScalar(YSize.width()),
                                 SkIntToScalar(size.height()) / SkIntToScalar(YSize.height()));
        filterModes[i] = (size == YSize) ? filterMode : minimizeFilterMode;
    }

    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(
            proxies, scales, filterModes, numPlanes, yuvaIndices, yuvColorSpace, localMatrix,
            domain));
}

#ifdef SK_DEBUG
SkString GrYUVtoRGBEffect::dumpInfo() const {
    SkString str;
    for (int i = 0; i < this->numTextureSamplers(); ++i) {
        str.appendf("%d: %d %d ", i,
                    this->textureSampler(i).proxy()->uniqueID().asUInt(),
                    this->textureSampler(i).proxy()->underlyingUniqueID().asUInt());
    }
    str.appendf("\n");

    return str;
}
#endif

GrGLSLFragmentProcessor* GrYUVtoRGBEffect::onCreateGLSLInstance() const {
    class GrGLSLYUVtoRGBEffect : public GrGLSLFragmentProcessor {
    public:
        GrGLSLYUVtoRGBEffect() {}

        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            const GrYUVtoRGBEffect& _outer = args.fFp.cast<GrYUVtoRGBEffect>();
            (void)_outer;

            if (kIdentity_SkYUVColorSpace != _outer.yuvColorSpace()) {
                fColorSpaceMatrixVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                        kHalf4x4_GrSLType,
                                                                        "colorSpaceMatrix");
            }

            int numSamplers = args.fTexSamplers.count();

            SkString coords[4];
            for (int i = 0; i < numSamplers; ++i) {
                coords[i] = fragBuilder->ensureCoords2D(args.fTransformedCoords[i]);
            }

            for (int i = 0; i < numSamplers; ++i) {
                SkString sampleVar;
                sampleVar.printf("tmp%d", i);
                fragBuilder->codeAppendf("half4 %s;", sampleVar.c_str());
                fGLDomains[i].sampleTexture(fragBuilder, args.fUniformHandler, args.fShaderCaps,
                        _outer.fDomains[i], sampleVar.c_str(), coords[i], args.fTexSamplers[i]);
            }

            static const char kChannelToChar[4] = { 'x', 'y', 'z', 'w' };

            fragBuilder->codeAppendf(
                "half4 yuvOne = half4(tmp%d.%c, tmp%d.%c, tmp%d.%c, 1.0);",
                    _outer.yuvaIndex(0).fIndex, kChannelToChar[(int)_outer.yuvaIndex(0).fChannel],
                    _outer.yuvaIndex(1).fIndex, kChannelToChar[(int)_outer.yuvaIndex(1).fChannel],
                    _outer.yuvaIndex(2).fIndex, kChannelToChar[(int)_outer.yuvaIndex(2).fChannel]);

            if (kIdentity_SkYUVColorSpace != _outer.yuvColorSpace()) {
                SkASSERT(fColorSpaceMatrixVar.isValid());
                fragBuilder->codeAppendf(
                    "yuvOne *= %s;", args.fUniformHandler->getUniformCStr(fColorSpaceMatrixVar));
            }

            if (_outer.yuvaIndex(3).fIndex >= 0) {
                fragBuilder->codeAppendf(
                    "half a = tmp%d.%c;", _outer.yuvaIndex(3).fIndex,
                                           kChannelToChar[(int)_outer.yuvaIndex(3).fChannel]);
                // premultiply alpha
                fragBuilder->codeAppend("yuvOne *= a;");
            } else {
                fragBuilder->codeAppend("half a = 1.0;");
            }

            fragBuilder->codeAppendf("%s = half4(yuvOne.xyz, a);", args.fOutputColor);
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& _proc) override {
            const GrYUVtoRGBEffect& _outer = _proc.cast<GrYUVtoRGBEffect>();

            switch (_outer.yuvColorSpace()) {
                case kJPEG_SkYUVColorSpace:
                    SkASSERT(fColorSpaceMatrixVar.isValid());
                    pdman.setMatrix4f(fColorSpaceMatrixVar, kJPEGConversionMatrix);
                    break;
                case kRec601_SkYUVColorSpace:
                    SkASSERT(fColorSpaceMatrixVar.isValid());
                    pdman.setMatrix4f(fColorSpaceMatrixVar, kRec601ConversionMatrix);
                    break;
                case kRec709_SkYUVColorSpace:
                    SkASSERT(fColorSpaceMatrixVar.isValid());
                    pdman.setMatrix4f(fColorSpaceMatrixVar, kRec709ConversionMatrix);
                    break;
                case kIdentity_SkYUVColorSpace:
                    break;
            }

            int numSamplers = _outer.numTextureSamplers();
            for (int i = 0; i < numSamplers; ++i) {
                fGLDomains[i].setData(pdman, _outer.fDomains[i],
                        _outer.textureSampler(i).proxy(), _outer.textureSampler(i).samplerState());
            }
        }

        UniformHandle fColorSpaceMatrixVar;
        GrTextureDomain::GLDomain fGLDomains[4];
    };

    return new GrGLSLYUVtoRGBEffect;
}
void GrYUVtoRGBEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    using Domain = GrTextureDomain::GLDomain;

    b->add32(this->numTextureSamplers());

    uint32_t packed = 0;
    uint32_t domain = 0;
    for (int i = 0; i < 4; ++i) {
        if (this->yuvaIndex(i).fIndex < 0) {
            continue;
        }

        uint8_t index = this->yuvaIndex(i).fIndex;
        uint8_t chann = (uint8_t) this->yuvaIndex(i).fChannel;

        SkASSERT(index < 4 && chann < 4);

        packed |= (index | (chann << 2)) << (i * 4);

        domain |= Domain::DomainKey(fDomains[i]) << (i * Domain::kDomainKeyBits);
    }
    if (kIdentity_SkYUVColorSpace == this->yuvColorSpace()) {
        packed |= 0x1 << 16;
    }

    b->add32(packed);
    b->add32(domain);
}
bool GrYUVtoRGBEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrYUVtoRGBEffect& that = other.cast<GrYUVtoRGBEffect>();

    for (int i = 0; i < 4; ++i) {
        if (fYUVAIndices[i] != that.fYUVAIndices[i]) {
            return false;
        }
    }

    for (int i = 0; i < this->numTextureSamplers(); ++i) {
        // 'fSamplers' is checked by the base class
        if (fSamplerTransforms[i] != that.fSamplerTransforms[i]) {
            return false;
        }
        if (!(fDomains[i] == that.fDomains[i])) {
            return false;
        }
    }

    if (fYUVColorSpace != that.fYUVColorSpace) {
        return false;
    }

    return true;
}
GrYUVtoRGBEffect::GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src)
        : INHERITED(kGrYUVtoRGBEffect_ClassID, src.optimizationFlags())
        , fDomains{src.fDomains[0], src.fDomains[1], src.fDomains[2], src.fDomains[3]}
        , fYUVColorSpace(src.fYUVColorSpace) {
    int numPlanes = src.numTextureSamplers();
    for (int i = 0; i < numPlanes; ++i) {
        fSamplers[i].reset(sk_ref_sp(src.fSamplers[i].proxy()), src.fSamplers[i].samplerState());
        fSamplerTransforms[i] = src.fSamplerTransforms[i];
        fSamplerCoordTransforms[i] = src.fSamplerCoordTransforms[i];
    }

    this->setTextureSamplerCnt(numPlanes);
    for (int i = 0; i < numPlanes; ++i) {
        this->addCoordTransform(&fSamplerCoordTransforms[i]);
    }

    memcpy(fYUVAIndices, src.fYUVAIndices, sizeof(fYUVAIndices));
}
std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(*this));
}
const GrFragmentProcessor::TextureSampler& GrYUVtoRGBEffect::onTextureSampler(int index) const {
    SkASSERT(index < this->numTextureSamplers());
    return fSamplers[index];
}
