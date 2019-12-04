/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrYUVtoRGBEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::Make(const sk_sp<GrTextureProxy> proxies[],
                                                            const SkYUVAIndex yuvaIndices[4],
                                                            SkYUVColorSpace yuvColorSpace,
                                                            GrSamplerState::Filter filterMode,
                                                            const SkMatrix& localMatrix,
                                                            const SkRect* domain) {
    int numPlanes;
    SkAssertResult(SkYUVAIndex::AreValidIndices(yuvaIndices, &numPlanes));

    const SkISize YDimensions = proxies[yuvaIndices[SkYUVAIndex::kY_Index].fIndex]->dimensions();

    // This promotion of nearest to bilinear for UV planes exists to mimic libjpeg[-turbo]'s
    // do_fancy_upsampling option. However, skbug.com/9693.
    GrSamplerState::Filter subsampledPlaneFilterMode = GrSamplerState::Filter::kMipMap == filterMode
                                                               ? GrSamplerState::Filter::kMipMap
                                                               : GrSamplerState::Filter::kBilerp;
    std::unique_ptr<GrFragmentProcessor> planeFPs[4];
    for (int i = 0; i < numPlanes; ++i) {
        SkISize dimensions = proxies[i]->dimensions();
        SkTCopyOnFirstWrite<SkMatrix> planeMatrix(&localMatrix);
        GrSamplerState::Filter planeFilter = filterMode;
        SkRect planeDomain;
        if (dimensions != YDimensions) {
            // JPEG chroma subsampling of odd dimensions produces U and V planes with the ceiling of
            // the image size divided by the subsampling factor (2). Our API for creating YUVA doesn't
            // capture the intended subsampling (and we should fix that). This fixes up 2x subsampling
            // for images with odd widths/heights (e.g. JPEG 420 or 422).
            float sx = (float)dimensions.width()  / YDimensions.width();
            float sy = (float)dimensions.height() / YDimensions.height();
            if ((YDimensions.width() & 0b1) && dimensions.width() == YDimensions.width() / 2 + 1) {
                sx = 0.5f;
            }
            if ((YDimensions.height() & 0b1) && dimensions.height() == YDimensions.height() / 2 + 1) {
                sy = 0.5f;
            }
            *planeMatrix.writable() = SkMatrix::MakeScale(sx, sy);
            planeMatrix.writable()->preConcat(localMatrix);
            planeFilter = subsampledPlaneFilterMode;
            if (domain) {
                planeDomain = {domain->fLeft   * sx,
                               domain->fTop    * sy,
                               domain->fRight  * sx,
                               domain->fBottom * sy};
            }
        } else if (domain) {
            planeDomain = *domain;
        }
        planeFPs[i] = GrSimpleTextureEffect::Make(proxies[i], kUnknown_SkAlphaType, *planeMatrix, planeFilter);
        if (domain) {
            SkASSERT(planeFilter != GrSamplerState::Filter::kMipMap);
            if (planeFilter != GrSamplerState::Filter::kNearest) {
                // Inset by half a pixel for bilerp, after scaling to the size of the plane
                planeDomain.inset(0.5f, 0.5f);
            }
            planeFPs[i] = GrDomainEffect::Make(std::move(planeFPs[i]), planeDomain, GrTextureDomain::kClamp_Mode, false);
        }
    }

    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(
            planeFPs, numPlanes, yuvaIndices, yuvColorSpace));
}

static SkAlphaType alpha_type(const SkYUVAIndex yuvaIndices[4]) {
    return yuvaIndices[3].fIndex >= 0 ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4], int numPlanes,
                                   const SkYUVAIndex yuvaIndices[4], SkYUVColorSpace yuvColorSpace)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID, ModulateForClampedSamplerOptFlags(alpha_type(yuvaIndices)))
        , fYUVColorSpace(yuvColorSpace) {
    for (int i = 0; i < numPlanes; ++i) {
        this->registerChildProcessor(std::move(planeFPs[i]));
    }
    std::copy_n(yuvaIndices, 4, fYUVAIndices);
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
            const GrYUVtoRGBEffect& yuvEffect = args.fFp.cast<GrYUVtoRGBEffect>();

            if (kIdentity_SkYUVColorSpace != yuvEffect.fYUVColorSpace) {
                fColorSpaceMatrixVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                        kHalf4x4_GrSLType,
                                                                        "colorSpaceMatrix");
            }

            int numPlanes = yuvEffect.numChildProcessors();

            SkString coords[4];
            fragBuilder->codeAppendf("half4 samples[%d];", numPlanes);
            for (int i = 0; i < numPlanes; ++i) {
                SkString tempVar;
                tempVar.printf("tmp%d", i);
                this->invokeChild(i, &tempVar, args);
                fragBuilder->codeAppendf("samples[%d] = %s;", i, tempVar.c_str());
            }

            static const char kChannelToChar[4] = { 'x', 'y', 'z', 'w' };

            fragBuilder->codeAppendf(
                "half4 yuvOne = half4(samples[%d].%c, samples[%d].%c, samples[%d].%c, 1.0);",
                yuvEffect.yuvaIndex(0).fIndex, kChannelToChar[(int)yuvEffect.yuvaIndex(0).fChannel],
                yuvEffect.yuvaIndex(1).fIndex, kChannelToChar[(int)yuvEffect.yuvaIndex(1).fChannel],
                yuvEffect.yuvaIndex(2).fIndex, kChannelToChar[(int)yuvEffect.yuvaIndex(2).fChannel]);

            if (kIdentity_SkYUVColorSpace != yuvEffect.fYUVColorSpace) {
                SkASSERT(fColorSpaceMatrixVar.isValid());
                fragBuilder->codeAppendf(
                    "yuvOne *= %s;", args.fUniformHandler->getUniformCStr(fColorSpaceMatrixVar));
                fragBuilder->codeAppend("yuvOne.xyz = clamp(yuvOne.xyz, 0, 1);");
            }

            if (yuvEffect.yuvaIndex(3).fIndex >= 0) {
                fragBuilder->codeAppendf(
                    "half a = samples[%d].%c;", yuvEffect.yuvaIndex(3).fIndex,
                                           kChannelToChar[(int)yuvEffect.yuvaIndex(3).fChannel]);
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
            const GrYUVtoRGBEffect& yuvEffect = _proc.cast<GrYUVtoRGBEffect>();

            if (yuvEffect.fYUVColorSpace != kIdentity_SkYUVColorSpace) {
                SkASSERT(fColorSpaceMatrixVar.isValid());
                float yuvM[20];
                SkColorMatrix_YUV2RGB(yuvEffect.fYUVColorSpace, yuvM);
                // Need to drop the fourth column to go to 4x4
                float mtx[16] = {
                    yuvM[ 0], yuvM[ 1], yuvM[ 2], yuvM[ 4],
                    yuvM[ 5], yuvM[ 6], yuvM[ 7], yuvM[ 9],
                    yuvM[10], yuvM[11], yuvM[12], yuvM[14],
                    yuvM[15], yuvM[16], yuvM[17], yuvM[19],
                };
                pdman.setMatrix4f(fColorSpaceMatrixVar, mtx);
            }
        }

        UniformHandle fColorSpaceMatrixVar;
        GrTextureDomain::GLDomain fGLDomains[4];
    };

    return new GrGLSLYUVtoRGBEffect;
}
void GrYUVtoRGBEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    uint32_t packed = 0;
    for (int i = 0; i < 4; ++i) {
        if (this->yuvaIndex(i).fIndex < 0) {
            continue;
        }

        uint8_t index = this->yuvaIndex(i).fIndex;
        uint8_t chann = (uint8_t) this->yuvaIndex(i).fChannel;

        SkASSERT(index < 4 && chann < 4);

        packed |= (index | (chann << 2)) << (i * 4);
    }
    if (fYUVColorSpace == kIdentity_SkYUVColorSpace) {
        packed |= 0x1 << 16;
    }
    b->add32(packed);
}

bool GrYUVtoRGBEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrYUVtoRGBEffect& that = other.cast<GrYUVtoRGBEffect>();

    for (int i = 0; i < 4; ++i) {
        if (fYUVAIndices[i] != that.fYUVAIndices[i]) {
            return false;
        }
    }

    if (fYUVColorSpace != that.fYUVColorSpace) {
        return false;
    }

    return true;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID, src.optimizationFlags())
        , fYUVColorSpace(src.fYUVColorSpace) {
    int numPlanes = src.numChildProcessors();
    for (int i = 0; i < numPlanes; ++i) {
        this->registerChildProcessor(this->childProcessor(i).clone());
    }
    std::copy_n(src.fYUVAIndices, this->numChildProcessors(), fYUVAIndices);
}

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(*this));
}
