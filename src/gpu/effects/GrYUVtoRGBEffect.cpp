/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrYUVtoRGBEffect.h"

#include "src/core/SkYUVMath.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

static void border_colors(SkYUVColorSpace cs,
                          const SkYUVAIndex yuvaIndices[4],
                          float planeBorders[4][4]) {
    float m[20];
    SkColorMatrix_RGB2YUV(cs, m);
    for (int i = 0; i < 4; ++i) {
        if (yuvaIndices[i].fIndex == -1) {
            return;
        }
        auto c = static_cast<int>(yuvaIndices[i].fChannel);
        planeBorders[yuvaIndices[i].fIndex][c] = m[i*5 + 4];
    }
}

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::Make(GrSurfaceProxyView views[],
                                                            const SkYUVAIndex yuvaIndices[4],
                                                            SkYUVColorSpace yuvColorSpace,
                                                            GrSamplerState samplerState,
                                                            const GrCaps& caps,
                                                            const SkMatrix& localMatrix,
                                                            const SkRect* subset,
                                                            const SkRect* domain) {
    int numPlanes;
    SkAssertResult(SkYUVAIndex::AreValidIndices(yuvaIndices, &numPlanes));

    const SkISize yDimensions =
            views[yuvaIndices[SkYUVAIndex::kY_Index].fIndex].proxy()->dimensions();

    bool usesBorder = samplerState.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
                      samplerState.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder;
    float planeBorders[4][4] = {};
    if (usesBorder) {
        border_colors(yuvColorSpace, yuvaIndices, planeBorders);
    }

    bool snap[2] = {false, false};
    std::unique_ptr<GrFragmentProcessor> planeFPs[4];
    for (int i = 0; i < numPlanes; ++i) {
        SkISize dimensions = views[i].proxy()->dimensions();
        SkTCopyOnFirstWrite<SkMatrix> planeMatrix(&SkMatrix::I());
        SkRect planeSubset;
        SkRect planeDomain;
        bool makeLinearWithSnap = false;
        float sx = 1.f,
              sy = 1.f;
        if (dimensions != yDimensions) {
            // JPEG chroma subsampling of odd dimensions produces U and V planes with the ceiling of
            // the image size divided by the subsampling factor (2). Our API for creating YUVA
            // doesn't capture the intended subsampling (and we should fix that). This fixes up 2x
            // subsampling for images with odd widths/heights (e.g. JPEG 420 or 422).
            sx = (float)dimensions.width()  / yDimensions.width();
            sy = (float)dimensions.height() / yDimensions.height();
            if ((yDimensions.width() & 0b1) && dimensions.width() == yDimensions.width() / 2 + 1) {
                sx = 0.5f;
            }
            if ((yDimensions.height() & 0b1) &&
                dimensions.height() == yDimensions.height() / 2 + 1) {
                sy = 0.5f;
            }
            *planeMatrix.writable() = SkMatrix::Scale(sx, sy);
            if (subset) {
                planeSubset = {subset->fLeft   * sx,
                               subset->fTop    * sy,
                               subset->fRight  * sx,
                               subset->fBottom * sy};
            }
            if (domain) {
                planeDomain = {domain->fLeft   * sx,
                               domain->fTop    * sy,
                               domain->fRight  * sx,
                               domain->fBottom * sy};
            }
            // This promotion of nearest to linear filtering for UV planes exists to mimic
            // libjpeg[-turbo]'s do_fancy_upsampling option. We will filter the subsampled plane,
            // however we want to filter at a fixed point for each logical image pixel to simulate
            // nearest neighbor.
            if (samplerState.filter() == GrSamplerState::Filter::kNearest) {
                bool snapX = (sx != 1.f),
                     snapY = (sy != 1.f);
                makeLinearWithSnap = snapX || snapY;
                snap[0] |= snapX;
                snap[1] |= snapY;
                if (domain) {
                    // The outer YUVToRGB effect will ensure sampling happens at pixel centers
                    // within this plane.
                    planeDomain = {std::floor(planeDomain.fLeft)   + 0.5f,
                                   std::floor(planeDomain.fTop)    + 0.5f,
                                   std::floor(planeDomain.fRight)  + 0.5f,
                                   std::floor(planeDomain.fBottom) + 0.5f};
                }
            }
        } else {
            if (subset) {
                planeSubset = *subset;
            }
            if (domain) {
                planeDomain = *domain;
            }
        }
        if (subset) {
            SkASSERT(samplerState.mipmapped() == GrMipmapped::kNo);
            if (makeLinearWithSnap) {
                // The plane is subsampled and we have an overall subset on the image. We're
                // emulating do_fancy_upsampling using linear filtering but snapping look ups to the
                // y-plane pixel centers. Consider a logical image pixel at the edge of the subset.
                // When computing the logical pixel color value we should use a 50/50 blend of two
                // values from the subsampled plane. Depending on where the subset edge falls in
                // actual subsampled plane, one of those values may come from outside the subset.
                // Hence, we use this custom inset factory which applies the wrap mode to
                // planeSubset but allows linear filtering to read pixels from the plane that are
                // just outside planeSubset.
                SkRect* domainRect = domain ? &planeDomain : nullptr;
                planeFPs[i] = GrTextureEffect::MakeCustomLinearFilterInset(
                        views[i], kUnknown_SkAlphaType, *planeMatrix, samplerState.wrapModeX(),
                        samplerState.wrapModeY(), planeSubset, domainRect, {sx / 2.f, sy / 2.f},
                        caps, planeBorders[i]);
            } else if (domain) {
                planeFPs[i] = GrTextureEffect::MakeSubset(views[i], kUnknown_SkAlphaType,
                                                          *planeMatrix, samplerState, planeSubset,
                                                          planeDomain, caps, planeBorders[i]);
            } else {
                planeFPs[i] = GrTextureEffect::MakeSubset(views[i], kUnknown_SkAlphaType,
                                                          *planeMatrix, samplerState, planeSubset,
                                                          caps, planeBorders[i]);
            }
        } else {
            GrSamplerState planeSampler = samplerState;
            if (makeLinearWithSnap) {
                planeSampler.setFilterMode(GrSamplerState::Filter::kLinear);
            }
            planeFPs[i] = GrTextureEffect::Make(views[i], kUnknown_SkAlphaType, *planeMatrix,
                                                planeSampler, caps, planeBorders[i]);
        }
    }
    auto fp = std::unique_ptr<GrFragmentProcessor>(
            new GrYUVtoRGBEffect(planeFPs, numPlanes, yuvaIndices, snap, yuvColorSpace));
    return GrMatrixEffect::Make(localMatrix, std::move(fp));
}

static SkAlphaType alpha_type(const SkYUVAIndex yuvaIndices[4]) {
    return yuvaIndices[3].fIndex >= 0 ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4],
                                   int numPlanes,
                                   const SkYUVAIndex yuvaIndices[4],
                                   const bool snap[2],
                                   SkYUVColorSpace yuvColorSpace)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID,
                              ModulateForClampedSamplerOptFlags(alpha_type(yuvaIndices)))
        , fYUVColorSpace(yuvColorSpace) {
    std::copy_n(yuvaIndices, 4, fYUVAIndices);
    std::copy_n(snap, 2, fSnap);

    if (fSnap[0] || fSnap[1]) {
        // Need this so that we can access coords in SKSL to perform snapping.
        this->setUsesSampleCoordsDirectly();
        for (int i = 0; i < numPlanes; ++i) {
            this->registerChild(std::move(planeFPs[i]), SkSL::SampleUsage::Explicit());
        }
    } else {
        for (int i = 0; i < numPlanes; ++i) {
            this->registerChild(std::move(planeFPs[i]));
        }
    }
}

#if GR_TEST_UTILS
SkString GrYUVtoRGBEffect::onDumpInfo() const {
    SkString str("(");
    for (int i = 0; i < 4; ++i) {
        str.appendf("YUVAIndices[%d]=%d %d, ",
                    i, fYUVAIndices[i].fIndex, static_cast<int>(fYUVAIndices[i].fChannel));
    }
    str.appendf("YUVColorSpace=%d, snap=(%d, %d))",
                static_cast<int>(fYUVColorSpace), fSnap[0], fSnap[1]);
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

            int numPlanes = yuvEffect.numChildProcessors();

            const char* sampleCoords = "";
            if (yuvEffect.fSnap[0] || yuvEffect.fSnap[1]) {
                fragBuilder->codeAppendf("float2 snappedCoords = %s;", args.fSampleCoord);
                if (yuvEffect.fSnap[0]) {
                    fragBuilder->codeAppend("snappedCoords.x = floor(snappedCoords.x) + 0.5;");
                }
                if (yuvEffect.fSnap[1]) {
                    fragBuilder->codeAppend("snappedCoords.y = floor(snappedCoords.y) + 0.5;");
                }
                sampleCoords = "snappedCoords";
            }

            fragBuilder->codeAppendf("half4 planes[%d];", numPlanes);
            for (int i = 0; i < numPlanes; ++i) {
                SkString tempVar = this->invokeChild(i, args, sampleCoords);
                fragBuilder->codeAppendf("planes[%d] = %s;", i, tempVar.c_str());
            }

            bool hasAlpha = yuvEffect.fYUVAIndices[3].fIndex >= 0;
            SkString rgba[4];
            rgba[3] = "1";
            for (int i = 0; i < (hasAlpha ? 4 : 3); ++i) {
                auto info = yuvEffect.fYUVAIndices[i];
                auto letter = "rgba"[static_cast<int>(info.fChannel)];
                rgba[i].printf("planes[%d].%c", info.fIndex, letter);
            }

            fragBuilder->codeAppendf("half4 color = half4(%s, %s, %s, %s);",
                    rgba[0].c_str(), rgba[1].c_str(), rgba[2].c_str(), rgba[3].c_str());

            if (kIdentity_SkYUVColorSpace != yuvEffect.fYUVColorSpace) {
                fColorSpaceMatrixVar = args.fUniformHandler->addUniform(&yuvEffect,
                        kFragment_GrShaderFlag, kHalf3x3_GrSLType, "colorSpaceMatrix");
                fColorSpaceTranslateVar = args.fUniformHandler->addUniform(&yuvEffect,
                        kFragment_GrShaderFlag, kHalf3_GrSLType, "colorSpaceTranslate");
                fragBuilder->codeAppendf(
                        "color.rgb = saturate(color.rgb * %s + %s);",
                        args.fUniformHandler->getUniformCStr(fColorSpaceMatrixVar),
                        args.fUniformHandler->getUniformCStr(fColorSpaceTranslateVar));
            }

            if (hasAlpha) {
                // premultiply alpha
                fragBuilder->codeAppendf("color.rgb *= color.a;");
            }
            fragBuilder->codeAppendf("return color;");
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const GrYUVtoRGBEffect& yuvEffect = proc.cast<GrYUVtoRGBEffect>();

            if (yuvEffect.fYUVColorSpace != kIdentity_SkYUVColorSpace) {
                SkASSERT(fColorSpaceMatrixVar.isValid());
                float yuvM[20];
                SkColorMatrix_YUV2RGB(yuvEffect.fYUVColorSpace, yuvM);
                // We drop the fourth column entirely since the transformation
                // should not depend on alpha. The fifth column is sent as a separate
                // vector. The fourth row is also dropped entirely because alpha should
                // never be modified.
                SkASSERT(yuvM[3] == 0 && yuvM[8] == 0 && yuvM[13] == 0 && yuvM[18] == 1);
                SkASSERT(yuvM[15] == 0 && yuvM[16] == 0 && yuvM[17] == 0 && yuvM[19] == 0);
                float mtx[9] = {
                    yuvM[ 0], yuvM[ 1], yuvM[ 2],
                    yuvM[ 5], yuvM[ 6], yuvM[ 7],
                    yuvM[10], yuvM[11], yuvM[12],
                };
                float v[3] = {yuvM[4], yuvM[9], yuvM[14]};
                pdman.setMatrix3f(fColorSpaceMatrixVar, mtx);
                pdman.set3fv(fColorSpaceTranslateVar, 1, v);
            }
        }

        UniformHandle fColorSpaceMatrixVar;
        UniformHandle fColorSpaceTranslateVar;
    };

    return new GrGLSLYUVtoRGBEffect;
}
void GrYUVtoRGBEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    uint32_t packed = 0;
    for (int i = 0; i < 4; ++i) {
        if (fYUVAIndices[i].fIndex < 0) {
            continue;
        }

        uint8_t index = fYUVAIndices[i].fIndex;
        uint8_t chann = static_cast<int>(fYUVAIndices[i].fChannel);

        SkASSERT(index < 4 && chann < 4);

        packed |= (index | (chann << 2)) << (i * 4);
    }
    if (fYUVColorSpace == kIdentity_SkYUVColorSpace) {
        packed |= 1 << 16;
    }
    if (fSnap[0]) {
        packed |= 1 << 17;
    }
    if (fSnap[1]) {
        packed |= 1 << 18;
    }
    b->add32(packed);
}

bool GrYUVtoRGBEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrYUVtoRGBEffect& that = other.cast<GrYUVtoRGBEffect>();

    return std::equal(fYUVAIndices, fYUVAIndices + 4, that.fYUVAIndices) &&
           std::equal(fSnap, fSnap + 2, that.fSnap) &&
           fYUVColorSpace == that.fYUVColorSpace;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID, src.optimizationFlags())
        , fYUVColorSpace(src.fYUVColorSpace) {
    this->cloneAndRegisterAllChildProcessors(src);
    if (src.fSnap[0] || src.fSnap[1]) {
        this->setUsesSampleCoordsDirectly();
    }
    std::copy_n(src.fYUVAIndices, this->numChildProcessors(), fYUVAIndices);
    std::copy_n(src.fSnap, 2, fSnap);
}

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(*this));
}
