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

// For DetermineDomainMode (in the MDB world) we have 4 rects:
//      1) the final instantiated backing storage (i.e., the actual GrTexture's extent)
//      2) the proxy's extent, which may or may not match the GrTexture's extent
//      3) the content rect, which can be a subset of the proxy's extent or null
//      4) the constraint rect, which can optionally be hard or soft

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

static bool is_irect(const SkRect& r) {
    return SkScalarIsInt(r.fLeft)  && SkScalarIsInt(r.fTop) &&
           SkScalarIsInt(r.fRight) && SkScalarIsInt(r.fBottom);
}

static SkIRect to_irect(const SkRect& r) {
    SkASSERT(is_irect(r));
    return SkIRect::MakeLTRB(SkScalarRoundToInt(r.fLeft),
                             SkScalarRoundToInt(r.fTop),
                             SkScalarRoundToInt(r.fRight),
                             SkScalarRoundToInt(r.fBottom));
}

class RectInfo {
public:
    enum Side { kLeft = 0, kTop = 1, kRight = 2, kBot = 3 };

    enum Bar {
        kSoft,              // there is data on the other side of this edge that we can sample
        kHard,              // the backing resouce ends at this edge
        kBad                // we can't sample across this edge
    };

    void set(const SkRect& rect, Bar leftType, Bar topType, Bar rightType, Bar botType) {
        fRect = rect;
        fTypes[kLeft]  = leftType;
        fTypes[kTop]   = topType;
        fTypes[kRight] = rightType;
        fTypes[kBot]   = botType;
    }

    const SkRect& asRect() const {
        return fRect;
    }

    Bar get(Side side) const { return fTypes[side]; }

    bool isHardAllAround() const {
        return kHard == fTypes[0] && kHard == fTypes[1] &&
               kHard == fTypes[2] && kHard == fTypes[3];
    }

    bool isNotBadAllAround() const {
        return kBad != fTypes[0] && kBad != fTypes[1] &&
               kBad != fTypes[2] && kBad != fTypes[3];
    }

    void operator=(const RectInfo& other) {
        fRect = other.fRect;
        fTypes[0] = other.fTypes[0];
        fTypes[1] = other.fTypes[1];
        fTypes[2] = other.fTypes[2];
        fTypes[3] = other.fTypes[3];
    }

private:
    SkRect  fRect;
    Bar     fTypes[4];
};

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

    // Proxies are always hard on the left and top but can be bad on the right and bottom
    // TODO: call GrResourceProvider::IsFunctionallyExact somehow?
    rect->set(SkRect::MakeWH(size, size),
              RectInfo::kHard,
              RectInfo::kHard,
              (isPowerOfTwo || isExact) ? RectInfo::kHard : RectInfo::kBad,
              (isPowerOfTwo || isExact) ? RectInfo::kHard : RectInfo::kBad);

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(resourceProvider,
                                                               desc, fit,
                                                               SkBudgeted::kYes);
    return proxy;
}

static const int kInsetLeft_Flag  = 0x1;
static const int kInsetTop_Flag   = 0x2;
static const int kInsetRight_Flag = 0x4;
static const int kInsetBot_Flag   = 0x8;

static RectInfo::Bar foo(RectInfo::Bar previous, bool isInsetHard) {
    if (RectInfo::kHard == previous) {
        return RectInfo::kHard;
    }
    if (isInsetHard) {
        return RectInfo::kBad;
    }
    return previous;
}

static RectInfo::Bar baz(RectInfo::Bar previous,
                         bool isInsetHard, bool areCoordsLimitedToRect,
                         float insetAmount, float halfFilterWidth) {
    if (isInsetHard) {
        return RectInfo::kBad;
    } else {
        return RectInfo::kSoft;
    }
}

// If 'isInsetHard' is true we can't sample across the inset boundary.
// If 'areCoordsLimitedToRect' is true the client promises to never sample outside the inset.
static const SkRect* generic_inset(const RectInfo& enclosing,
                                   RectInfo* result,
                                   bool isInsetHard,
                                   bool areCoordsLimitedToRect,
                                   float insetAmount,
                                   float halfFilterWidth,
                                   uint32_t flags) {
    SkRect newR = enclosing.asRect();

    RectInfo::Bar left = foo(enclosing.get(RectInfo::kLeft), isInsetHard);
    if (flags & kInsetLeft_Flag) {
        newR.fLeft += insetAmount;
        left = baz(left, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth);
    }

    RectInfo::Bar top = foo(enclosing.get(RectInfo::kTop), isInsetHard);
    if (flags & kInsetTop_Flag) {
        newR.fTop += insetAmount;
        top = baz(top, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth);
    }

    RectInfo::Bar right = foo(enclosing.get(RectInfo::kRight), isInsetHard);
    if (flags & kInsetRight_Flag) {
        newR.fRight -= insetAmount;
        right = baz(right, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth);
    }

    RectInfo::Bar bot = foo(enclosing.get(RectInfo::kBot), isInsetHard);
    if (flags & kInsetBot_Flag) {
        newR.fBottom -= insetAmount;
        bot = baz(bot, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth);
    }

    result->set(newR, left, top, right, bot);
    return &result->asRect();
}

// Make a rect that only touches the enclosing rect on the left.
static const SkRect* left_only(const RectInfo& enclosing,
                               RectInfo* result,
                               bool isInsetHard,
                               bool areCoordsLimitedToRect,
                               float insetAmount,
                               float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth,
                         kInsetTop_Flag|kInsetRight_Flag|kInsetBot_Flag);
}

// Make a rect that only touches the enclosing rect on the top.
static const SkRect* top_only(const RectInfo& enclosing,
                               RectInfo* result,
                               bool isInsetHard,
                               bool areCoordsLimitedToRect,
                               float insetAmount,
                               float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetRight_Flag|kInsetBot_Flag);
}

// Make a rect that only touches the enclosing rect on the right.
static const SkRect* right_only(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard,
                                bool areCoordsLimitedToRect,
                                float insetAmount,
                                float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetBot_Flag);
}

// Make a rect that only touches the enclosing rect on the bottom.
static const SkRect* bot_only(const RectInfo& enclosing,
                              RectInfo* result,
                              bool isInsetHard,
                              bool areCoordsLimitedToRect,
                              float insetAmount,
                              float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetRight_Flag);
}

// Make a rect that is inset all around.
static const SkRect* full_inset(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard,
                                bool areCoordsLimitedToRect,
                                float insetAmount,
                                float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetRight_Flag|kInsetBot_Flag);
}

// This is only used for content rect creation. We ensure 'result' is correct but
// return null to indicate no content area (other than what the proxy specifies).
static const SkRect* null_rect(const RectInfo& enclosing,
                               RectInfo* result,
                               bool isInsetHard,
                               bool areCoordsLimitedToRect,
                               float insetAmount,
                               float halfFilterWidth) {
    generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth, 0);
    return nullptr;
}

// Make a rect with no inset. This is only used for constraint rect creation.
static const SkRect* no_inset(const RectInfo& enclosing,
                              RectInfo* result,
                              bool isInsetHard,
                              bool areCoordsLimitedToRect,
                              float insetAmount,
                              float halfFilterWidth) {
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect, insetAmount, halfFilterWidth, 0);
}

static void proxy_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {

    GrTextureProducer_TestAccess::DomainMode actualMode, expectedMode;
    SkRect actualDomainRect; //, expectedDomainRect;

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

    static const float gHalfFilterWidth[] = { 0.0f, 0.5f, 1.5f, 10000.0f };

    for (auto isPowerOfTwoSized : { true, false }) {
        for (auto isExact : { true, false }) {
            RectInfo outermost;

            sk_sp<GrTextureProxy> proxy = create_proxy(resourceProvider, isPowerOfTwoSized,
                                                       isExact, &outermost);

            for (auto contentRectMaker : { left_only, top_only, right_only, bot_only, full_inset, null_rect}) {
                RectInfo contentRectStorage;
                const SkRect* contentRect = (*contentRectMaker)(outermost,
                                                                &contentRectStorage,
                                                                true, false, 5.0f, -1.0f);
                if (contentRect) {
                    // We only have content rects if they actually reduce the extent of the content
                    SkASSERT(!contentRect->contains(outermost.asRect()));
                    SkASSERT(outermost.asRect().contains(*contentRect));
                    SkASSERT(is_irect(*contentRect));
                }

                for (auto isConstraintRectHard : { true, false }) {
                    for (auto areCoordsLimitedToConstraintRect : { true, false }) {
                        for (int filterMode = 0; filterMode < 4; ++filterMode) {
                            for (auto constraintRectMaker : { left_only, top_only, right_only, bot_only, full_inset, no_inset }) {
                                RectInfo constraintRectStorage;
                                const SkRect* constraintRect = (*constraintRectMaker)(
                                                    contentRect ? contentRectStorage : outermost,
                                                    &constraintRectStorage,
                                                    isConstraintRectHard,
                                                    areCoordsLimitedToConstraintRect,
                                                    5.0f,
                                                    gHalfFilterWidth[filterMode]);
                                SkASSERT(constraintRect); // always need one of these
                                if (contentRect) {
                                    contentRect->contains(*constraintRect);
                                } else {
                                    outermost.asRect().contains(*constraintRect);
                                }

                                SkIRect contentIRect;
                                if (contentRect) {
                                    contentIRect = to_irect(*contentRect);
                                }

                                actualMode = GrTextureProducer_TestAccess::DetermineDomainMode(
                                                *constraintRect,
                                                isConstraintRectHard
                                                    ? GrTextureProducer::kYes_FilterConstraint
                                                    : GrTextureProducer::kNo_FilterConstraint,
                                                areCoordsLimitedToConstraintRect,
                                                proxy.get(),
                                                contentRect ? &contentIRect : nullptr,
                                                gModePtrs[filterMode],
                                                &actualDomainRect);

                                
                                expectedMode = GrTextureProducer_TestAccess::DomainMode::kNoDomain_DomainMode;
                                if (constraintRectStorage.isHardAllAround()) {
                                    expectedMode = GrTextureProducer_TestAccess::DomainMode::kNoDomain_DomainMode;
                                } else if (!isConstraintRectHard && constraintRectStorage.isNotBadAllAround()) {
                                    expectedMode = GrTextureProducer_TestAccess::DomainMode::kNoDomain_DomainMode;
                                }

                                SkASSERT(expectedMode == actualMode);
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
