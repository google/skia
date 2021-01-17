/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrYUVtoRGBEffect.h"

#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrYUVATextureProxies.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

static void border_colors(const GrYUVATextureProxies& yuvaProxies, float planeBorders[4][4]) {
    float m[20];
    SkColorMatrix_RGB2YUV(yuvaProxies.yuvaInfo().yuvColorSpace(), m);
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        auto [plane, channel] = yuvaProxies.yuvaLocations()[i];
        if (plane == -1) {
            return;
        }
        auto c = static_cast<int>(channel);
        planeBorders[plane][c] = m[i*5 + 4];
    }
}

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::Make(const GrYUVATextureProxies& yuvaProxies,
                                                            GrSamplerState samplerState,
                                                            const GrCaps& caps,
                                                            const SkMatrix& localMatrix,
                                                            const SkRect* subset,
                                                            const SkRect* domain) {
    int numPlanes = yuvaProxies.yuvaInfo().numPlanes();
    if (!yuvaProxies.isValid()) {
        return nullptr;
    }

    bool usesBorder = samplerState.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
                      samplerState.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder;
    float planeBorders[4][4] = {};
    if (usesBorder) {
        border_colors(yuvaProxies, planeBorders);
    }

    bool snap[2] = {false, false};
    std::unique_ptr<GrFragmentProcessor> planeFPs[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < numPlanes; ++i) {
        GrSurfaceProxyView view = yuvaProxies.makeView(i);
        SkMatrix planeMatrix = yuvaProxies.yuvaInfo().originMatrix();
        // The returned matrix is a view matrix but we need a local matrix.
        SkAssertResult(planeMatrix.invert(&planeMatrix));
        SkRect planeSubset;
        SkRect planeDomain;
        bool makeLinearWithSnap = false;
        auto [ssx, ssy] = yuvaProxies.yuvaInfo().planeSubsamplingFactors(i);
        SkASSERT(ssx > 0 && ssx <= 4);
        SkASSERT(ssy > 0 && ssy <= 2);
        float scaleX = 1.f;
        float scaleY = 1.f;
        if (ssx > 1 || ssy > 1) {
            // JPEG chroma subsampling of odd dimensions produces U and V planes with the ceiling of
            // the image size divided by the subsampling factor (2). Our API for creating YUVA
            // doesn't capture the intended subsampling (and we should fix that). This fixes up 2x
            // subsampling for images with odd widths/heights (e.g. JPEG 420 or 422).
            scaleX = 1.f/ssx;
            scaleY = 1.f/ssy;
            // We would want to add a translation to this matrix to handle other sitings.
            SkASSERT(yuvaProxies.yuvaInfo().sitingX() == SkYUVAInfo::Siting::kCentered);
            SkASSERT(yuvaProxies.yuvaInfo().sitingY() == SkYUVAInfo::Siting::kCentered);
            planeMatrix.postConcat(SkMatrix::Scale(scaleX, scaleY));
            if (subset) {
                planeSubset = {subset->fLeft  *scaleX,
                               subset->fTop   *scaleY,
                               subset->fRight *scaleX,
                               subset->fBottom*scaleY};
            }
            if (domain) {
                planeDomain = {domain->fLeft  *scaleX,
                               domain->fTop   *scaleY,
                               domain->fRight *scaleX,
                               domain->fBottom*scaleY};
            }
            // This promotion of nearest to linear filtering for UV planes exists to mimic
            // libjpeg[-turbo]'s do_fancy_upsampling option. We will filter the subsampled plane,
            // however we want to filter at a fixed point for each logical image pixel to simulate
            // nearest neighbor.
            if (samplerState.filter() == GrSamplerState::Filter::kNearest) {
                bool snapX = (ssx != 1),
                     snapY = (ssy != 1);
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
                planeFPs[i] = GrTextureEffect::MakeCustomLinearFilterInset(std::move(view),
                                                                           kUnknown_SkAlphaType,
                                                                           planeMatrix,
                                                                           samplerState.wrapModeX(),
                                                                           samplerState.wrapModeY(),
                                                                           planeSubset,
                                                                           domainRect,
                                                                           {scaleX/2.f, scaleY/2.f},
                                                                           caps,
                                                                           planeBorders[i]);
            } else if (domain) {
                planeFPs[i] = GrTextureEffect::MakeSubset(std::move(view),
                                                          kUnknown_SkAlphaType,
                                                          planeMatrix,
                                                          samplerState,
                                                          planeSubset,
                                                          planeDomain,
                                                          caps,
                                                          planeBorders[i]);
            } else {
                planeFPs[i] = GrTextureEffect::MakeSubset(std::move(view),
                                                          kUnknown_SkAlphaType,
                                                          planeMatrix,
                                                          samplerState,
                                                          planeSubset,
                                                          caps,
                                                          planeBorders[i]);
            }
        } else {
            GrSamplerState planeSampler = samplerState;
            if (makeLinearWithSnap) {
                planeSampler.setFilterMode(GrSamplerState::Filter::kLinear);
            }
            planeFPs[i] = GrTextureEffect::Make(std::move(view),
                                                kUnknown_SkAlphaType,
                                                planeMatrix,
                                                planeSampler,
                                                caps,
                                                planeBorders[i]);
        }
    }
    std::unique_ptr<GrFragmentProcessor> fp(
            new GrYUVtoRGBEffect(planeFPs,
                                 numPlanes,
                                 yuvaProxies.yuvaLocations(),
                                 snap,
                                 yuvaProxies.yuvaInfo().yuvColorSpace()));
    return GrMatrixEffect::Make(localMatrix, std::move(fp));
}

static SkAlphaType alpha_type(const SkYUVAInfo::YUVALocations locations) {
    return locations[SkYUVAInfo::YUVAChannels::kA].fPlane >= 0 ? kPremul_SkAlphaType
                                                               : kOpaque_SkAlphaType;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4],
                                   int numPlanes,
                                   const SkYUVAInfo::YUVALocations& locations,
                                   const bool snap[2],
                                   SkYUVColorSpace yuvColorSpace)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID,
                              ModulateForClampedSamplerOptFlags(alpha_type(locations)))
        , fLocations(locations)
        , fYUVColorSpace(yuvColorSpace) {
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
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        str.appendf("Locations[%d]=%d %d, ",
                    i, fLocations[i].fPlane, static_cast<int>(fLocations[i].fChannel));
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

            bool hasAlpha = yuvEffect.fLocations[SkYUVAInfo::YUVAChannels::kA].fPlane >= 0;
            SkString rgba[4];
            rgba[3] = "1";
            for (int i = 0; i < (hasAlpha ? 4 : 3); ++i) {
                auto [plane, channel] = yuvEffect.fLocations[i];
                auto letter = "rgba"[static_cast<int>(channel)];
                rgba[i].printf("planes[%d].%c", plane, letter);
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
    int i = 0;
    for (auto [plane, channel] : fLocations) {
        if (plane < 0) {
            continue;
        }

        uint8_t chann = static_cast<int>(channel);

        SkASSERT(plane < 4 && chann < 4);

        packed |= (plane | (chann << 2)) << (i++ * 4);
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

    return fLocations == that.fLocations            &&
           std::equal(fSnap, fSnap + 2, that.fSnap) &&
           fYUVColorSpace == that.fYUVColorSpace;
}

GrYUVtoRGBEffect::GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src)
        : GrFragmentProcessor(kGrYUVtoRGBEffect_ClassID, src.optimizationFlags())
        , fLocations((src.fLocations))
        , fYUVColorSpace(src.fYUVColorSpace) {
    this->cloneAndRegisterAllChildProcessors(src);
    if (src.fSnap[0] || src.fSnap[1]) {
        this->setUsesSampleCoordsDirectly();
    }

    std::copy_n(src.fSnap, 2, fSnap);
}

std::unique_ptr<GrFragmentProcessor> GrYUVtoRGBEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(*this));
}
