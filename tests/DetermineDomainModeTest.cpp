/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if 1 //SK_SUPPORT_GPU

#include "GrSurfaceProxy.h"
#include "GrTextureProducer.h"
#include "GrTextureProxy.h"

// For DetermineDomainMode in the MDB world we have 4 rects:
//      1) the final instantiated backing store size (i.e., the actual GrTexture size)
//      2) the proxy's extent
//      3) the content rect
//      4) the constraint rect

class GrTextureProducer_TestAccess {
public:
    typedef GrTextureProducer::DomainMode DomainMode;

    static DomainMode DetermineDomainMode(
                                const SkRect& constraintRect,
                                GrTextureProducer::FilterConstraint filterConstraint,
                                bool coordsLimitedToConstraintRect,
                                int texW, int texH,
                                const SkIRect* textureContentArea,
                                const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
                                SkRect* domainRect) {
        return GrTextureProducer::DetermineDomainMode(constraintRect,
                                                      filterConstraint,
                                                      coordsLimitedToConstraintRect,
                                                      texW, texH,
                                                      textureContentArea,
                                                      filterModeOrNullForBicubic,
                                                      domainRect);
    }

    static DomainMode DetermineDomainMode(
                                const SkRect& constraintRect,
                                GrTextureProducer::FilterConstraint filterConstraint,
                                bool coordsLimitedToConstraintRect,
                                GrTextureProxy* proxy,
                                const SkIRect* textureContentArea,
                                const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
                                SkRect* domainRect) {
        return GrTextureProducer::DetermineDomainMode(constraintRect,
                                                      filterConstraint,
                                                      coordsLimitedToConstraintRect,
                                                      proxy,
                                                      textureContentArea,
                                                      filterModeOrNullForBicubic,
                                                      domainRect);    
    }
};


class RectInfo {
public:
    enum Side { kLeft = 0, kTop = 1, kRight = 2, kBot = 3 };

    enum Bar {
        kSoft,              // there is data on the other side of this edge that we can sample
        kHard,              // the backing resouce ends at this edge
        kBadOnOtherSide     // we can't sample across this edge
    };

    void set(const SkIRect& rect, Bar leftType, Bar topType, Bar rightType, Bar botType) {
        fRect = rect;
        fTypes[kLeft]  = leftType;
        fTypes[kTop]   = topType;
        fTypes[kRight] = rightType;
        fTypes[kBot]   = botType;
    }

    const SkIRect& rect() const { return fRect; }

    Bar get(Side side) const { return fTypes[side]; }

private:
    SkIRect fRect;
    Bar     fTypes[4];
};

// Make a rect that only touches the enclosing rect on the left.
// If 'isInsetHard' is true we can't sample across the inset boundary
static const SkIRect* left_only(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard) {
    SkIRect newR = enclosing.rect();
    newR.fTop    += 5.0f;
    newR.fRight  -= 5.0f;
    newR.fBottom -= 5.0f;

    result->set(newR,
                enclosing.get(RectInfo::kLeft),
                isInsetHard ? RectInfo::kBadOnOtherSide : RectInfo::kSoft,
                isInsetHard ? RectInfo::kBadOnOtherSide : RectInfo::kSoft,
                isInsetHard ? RectInfo::kBadOnOtherSide : RectInfo::kSoft);
    return &result->rect();
}

static const SkIRect* null_rect(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard) {
    return nullptr;
}

static sk_sp<GrTextureProxy> create_proxy(GrResourceProvider* resourceProvider,
                                          bool isPowerOfTwo,
                                          bool isExact,
                                          RectInfo* rect) {
    int size = isPowerOfTwo ? 128 : 100;
    SkBackingFit fit = isExact ? SkBackingFit::kExact : SkBackingFit::kApprox;

    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth = size;
    desc.fHeight = size;

    rect->set(SkIRect::MakeWH(size, size),
              RectInfo::kHard, RectInfo::kHard,
              isPowerOfTwo || isExact ? RectInfo::kHard : RectInfo::kBadOnOtherSide,
              isPowerOfTwo || isExact ? RectInfo::kHard : RectInfo::kBadOnOtherSide);

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(resourceProvider,
                                                               desc, fit,
                                                               SkBudgeted::kYes);
    return proxy;
}


static void proxy_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {

    GrTextureProducer_TestAccess::DomainMode actualMode;
    SkRect actualDomainRect;

#if 0
    static const struct {
        GrTextureProducer_TestAccess::DomainMode fMode;
        SkRect                                   fDomainRect;
    } gExpected [] = {
        { GrTextureProducer_TestAccess::DomainMode::kNoDomain_DomainMode, SkRect::MakeEmpty() },
    };
#endif

    static const GrSamplerParams::FilterMode gModes[] = {
        GrSamplerParams::kNone_FilterMode,
        GrSamplerParams::kBilerp_FilterMode,
        GrSamplerParams::kMipMap_FilterMode,
    };

    static const GrSamplerParams::FilterMode* gModePtrs[] = {
        &gModes[0],
        &gModes[1],
        nullptr,
        &gModes[2]
    };

    int testCase = 0;
    for (auto isPowerOfTwoSized : { true, false }) {
        for (auto isExact : { true, false }) {
            RectInfo outermost;

            sk_sp<GrTextureProxy> proxy = create_proxy(resourceProvider, isPowerOfTwoSized,
                                                       isExact, &outermost);

            for (auto contentRectMaker : { left_only, null_rect }) {
                RectInfo contentRectStorage;
                const SkIRect* contentRect = (*contentRectMaker)(outermost,
                                                                 &contentRectStorage,
                                                                 true);

                for (auto isConstraintRectHard : { true, false }) {
                    for (auto areCoordsLimitedToConstraintRect : { true, false }) {
                        for (auto constraintRectMaker : { left_only }) {
                            RectInfo constraintRectStorage;
                            const SkIRect* constraintRect = (*constraintRectMaker)(
                                                contentRect ? contentRectStorage : outermost,
                                                &constraintRectStorage,
                                                isConstraintRectHard);
                            SkASSERT(constraintRect); // always need one of these

                            for (int filterMode = 0; filterMode < 4; ++filterMode) {
                                actualMode = GrTextureProducer_TestAccess::DetermineDomainMode(
                                                SkRect::Make(*constraintRect),
                                                isConstraintRectHard
                                                    ? GrTextureProducer::kYes_FilterConstraint
                                                    : GrTextureProducer::kYes_FilterConstraint,
                                                areCoordsLimitedToConstraintRect,
                                                proxy.get(),
                                                contentRect,
                                                gModePtrs[filterMode],
                                                &actualDomainRect);

                                ++testCase;
                            }
                        }
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DetermineDomainModeTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    proxy_test(reporter, context->resourceProvider());
}

#endif
