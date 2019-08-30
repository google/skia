/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/GrTextureProxy.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <initializer_list>

// For DetermineDomainMode (in the MDB world) we have 3 rects:
//      1) the final instantiated backing storage (i.e., the actual GrTexture's extent)
//      2) the proxy's extent, which may or may not match the GrTexture's extent
//      3) the constraint rect, which can optionally be hard or soft
// This test "fuzzes" all the combinations of these rects.
class GrTextureProducer_TestAccess {
public:
    using DomainMode = GrTextureProducer::DomainMode;

    static DomainMode DetermineDomainMode(const SkRect& constraintRect,
                                          GrTextureProducer::FilterConstraint filterConstraint,
                                          bool coordsLimitedToConstraintRect,
                                          GrTextureProxy* proxy,
                                          const GrSamplerState::Filter* filterModeOrNullForBicubic,
                                          SkRect* domainRect) {
        return GrTextureProducer::DetermineDomainMode(constraintRect,
                                                      filterConstraint,
                                                      coordsLimitedToConstraintRect,
                                                      proxy,
                                                      filterModeOrNullForBicubic,
                                                      domainRect);
    }
};

using DomainMode = GrTextureProducer_TestAccess::DomainMode;

class RectInfo {
public:
    enum Side { kLeft = 0, kTop = 1, kRight = 2, kBot = 3 };

    enum EdgeType {
        kSoft = 0,   // there is data on the other side of this edge that we are allowed to sample
        kHard = 1,   // the backing resource ends at this edge
        kBad  = 2    // we can't sample across this edge
    };

    void set(const SkRect& rect, EdgeType left, EdgeType top, EdgeType right, EdgeType bot,
             const char* name) {
        fRect = rect;
        fTypes[kLeft]  = left;
        fTypes[kTop]   = top;
        fTypes[kRight] = right;
        fTypes[kBot]   = bot;
        fName = name;
    }

    const SkRect& rect() const { return fRect; }
    EdgeType edgeType(Side side) const { return fTypes[side]; }
    const char* name() const { return fName; }

#ifdef SK_DEBUG
    bool isHardOrBadAllAround() const {
        for (int i = 0; i < 4; ++i) {
            if (kHard != fTypes[i] && kBad != fTypes[i]) {
                return false;
            }
        }
        return true;
    }
#endif

    bool hasABad() const {
        for (int i = 0; i < 4; ++i) {
            if (kBad == fTypes[i]) {
                return true;
            }
        }
        return false;
    }

#ifdef SK_DEBUG
    void print(const char* label) const {
        SkDebugf("%s: %s (%.1f, %.1f, %.1f, %.1f), L: %s T: %s R: %s B: %s\n",
                 label, fName,
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                 ToStr(fTypes[kLeft]), ToStr(fTypes[kTop]),
                 ToStr(fTypes[kRight]), ToStr(fTypes[kBot]));
    }
#endif

private:
#ifdef SK_DEBUG
    static const char* ToStr(EdgeType type) {
        static const char* names[] = { "soft", "hard", "bad" };
        return names[type];
    }
#endif

    RectInfo operator=(const RectInfo& other); // disallow

    SkRect      fRect;
    EdgeType    fTypes[4];
    const char* fName;

};

static sk_sp<GrTextureProxy> create_proxy(GrContext* ctx,
                                          bool isPowerOfTwo,
                                          bool isExact,
                                          RectInfo* rect) {
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    const GrCaps* caps = ctx->priv().caps();

    int size = isPowerOfTwo ? 128 : 100;
    SkBackingFit fit = isExact ? SkBackingFit::kExact : SkBackingFit::kApprox;

    GrSurfaceDesc desc;
    desc.fWidth = size;
    desc.fHeight = size;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    static const char* name = "proxy";

    // Proxies are always hard on the left and top but can be bad on the right and bottom
    rect->set(SkRect::MakeWH(size, size),
              RectInfo::kHard,
              RectInfo::kHard,
              (isPowerOfTwo || isExact) ? RectInfo::kHard : RectInfo::kBad,
              (isPowerOfTwo || isExact) ? RectInfo::kHard : RectInfo::kBad,
              name);

    return proxyProvider->createProxy(format, desc, GrRenderable::kNo, 1, kTopLeft_GrSurfaceOrigin,
                                      GrMipMapped::kNo, fit, SkBudgeted::kYes, GrProtected::kNo);
}

static RectInfo::EdgeType compute_inset_edgetype(RectInfo::EdgeType previous,
                                                 bool isInsetHard, bool coordsAreLimitedToRect,
                                                 float insetAmount, float halfFilterWidth) {
    if (isInsetHard) {
        if (coordsAreLimitedToRect) {
            SkASSERT(halfFilterWidth >= 0.0f);
            if (0.0f == halfFilterWidth) {
                return RectInfo::kSoft;
            }
        }

        if (0.0f == insetAmount && RectInfo::kHard == previous) {
            return RectInfo::kHard;
        }

        return RectInfo::kBad;
    }

    if (RectInfo::kHard == previous) {
        return RectInfo::kHard;
    }

    if (coordsAreLimitedToRect) {
        SkASSERT(halfFilterWidth >= 0.0f);
        if (0.0 == halfFilterWidth || insetAmount > halfFilterWidth) {
            return RectInfo::kSoft;
        }
    }

    return previous;
}

static const int kInsetLeft_Flag  = 0x1;
static const int kInsetTop_Flag   = 0x2;
static const int kInsetRight_Flag = 0x4;
static const int kInsetBot_Flag   = 0x8;

// If 'isInsetHard' is true we can't sample across the inset boundary.
// If 'areCoordsLimitedToRect' is true the client promises to never sample outside the inset.
static const SkRect* generic_inset(const RectInfo& enclosing,
                                   RectInfo* result,
                                   bool isInsetHard,
                                   bool areCoordsLimitedToRect,
                                   float insetAmount,
                                   float halfFilterWidth,
                                   uint32_t flags,
                                   const char* name) {
    SkRect newR = enclosing.rect();

    RectInfo::EdgeType left = enclosing.edgeType(RectInfo::kLeft);
    if (flags & kInsetLeft_Flag) {
        newR.fLeft += insetAmount;
        left = compute_inset_edgetype(left, isInsetHard, areCoordsLimitedToRect,
                                      insetAmount, halfFilterWidth);
    } else {
        left = compute_inset_edgetype(left, isInsetHard, areCoordsLimitedToRect,
                                      0.0f, halfFilterWidth);
    }

    RectInfo::EdgeType top = enclosing.edgeType(RectInfo::kTop);
    if (flags & kInsetTop_Flag) {
        newR.fTop += insetAmount;
        top = compute_inset_edgetype(top, isInsetHard, areCoordsLimitedToRect,
                                     insetAmount, halfFilterWidth);
    } else {
        top = compute_inset_edgetype(top, isInsetHard, areCoordsLimitedToRect,
                                     0.0f, halfFilterWidth);
    }

    RectInfo::EdgeType right = enclosing.edgeType(RectInfo::kRight);
    if (flags & kInsetRight_Flag) {
        newR.fRight -= insetAmount;
        right = compute_inset_edgetype(right, isInsetHard, areCoordsLimitedToRect,
                                       insetAmount, halfFilterWidth);
    } else {
        right = compute_inset_edgetype(right, isInsetHard, areCoordsLimitedToRect,
                                       0.0f, halfFilterWidth);
    }

    RectInfo::EdgeType bot = enclosing.edgeType(RectInfo::kBot);
    if (flags & kInsetBot_Flag) {
        newR.fBottom -= insetAmount;
        bot = compute_inset_edgetype(bot, isInsetHard, areCoordsLimitedToRect,
                                     insetAmount, halfFilterWidth);
    } else {
        bot = compute_inset_edgetype(bot, isInsetHard, areCoordsLimitedToRect,
                                     0.0f, halfFilterWidth);
    }

    result->set(newR, left, top, right, bot, name);
    return &result->rect();
}

// Make a rect that only touches the enclosing rect on the left.
static const SkRect* left_only(const RectInfo& enclosing,
                               RectInfo* result,
                               bool isInsetHard,
                               bool areCoordsLimitedToRect,
                               float insetAmount,
                               float halfFilterWidth) {
    static const char* name = "left";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth,
                         kInsetTop_Flag|kInsetRight_Flag|kInsetBot_Flag, name);
}

// Make a rect that only touches the enclosing rect on the top.
static const SkRect* top_only(const RectInfo& enclosing,
                               RectInfo* result,
                               bool isInsetHard,
                               bool areCoordsLimitedToRect,
                               float insetAmount,
                               float halfFilterWidth) {
    static const char* name = "top";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetRight_Flag|kInsetBot_Flag, name);
}

// Make a rect that only touches the enclosing rect on the right.
static const SkRect* right_only(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard,
                                bool areCoordsLimitedToRect,
                                float insetAmount,
                                float halfFilterWidth) {
    static const char* name = "right";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetBot_Flag, name);
}

// Make a rect that only touches the enclosing rect on the bottom.
static const SkRect* bot_only(const RectInfo& enclosing,
                              RectInfo* result,
                              bool isInsetHard,
                              bool areCoordsLimitedToRect,
                              float insetAmount,
                              float halfFilterWidth) {
    static const char* name = "bot";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetRight_Flag, name);
}

// Make a rect that is inset all around.
static const SkRect* full_inset(const RectInfo& enclosing,
                                RectInfo* result,
                                bool isInsetHard,
                                bool areCoordsLimitedToRect,
                                float insetAmount,
                                float halfFilterWidth) {
    static const char* name = "all";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth,
                         kInsetLeft_Flag|kInsetTop_Flag|kInsetRight_Flag|kInsetBot_Flag, name);
}

// Make a rect with no inset. This is only used for constraint rect creation.
static const SkRect* no_inset(const RectInfo& enclosing,
                              RectInfo* result,
                              bool isInsetHard,
                              bool areCoordsLimitedToRect,
                              float insetAmount,
                              float halfFilterWidth) {
    static const char* name = "none";
    return generic_inset(enclosing, result, isInsetHard, areCoordsLimitedToRect,
                         insetAmount, halfFilterWidth, 0, name);
}

static void proxy_test(skiatest::Reporter* reporter, GrContext* context) {
    GrTextureProducer_TestAccess::DomainMode actualMode, expectedMode;
    SkRect actualDomainRect;

    static const GrSamplerState::Filter gModes[] = {
            GrSamplerState::Filter::kNearest,
            GrSamplerState::Filter::kBilerp,
            GrSamplerState::Filter::kMipMap,
    };

    static const GrSamplerState::Filter* gModePtrs[] = {&gModes[0], &gModes[1], nullptr,
                                                        &gModes[2]};

    static const float gHalfFilterWidth[] = { 0.0f, 0.5f, 1.5f, 10000.0f };

    for (auto isPowerOfTwoSized : { true, false }) {
        for (auto isExact : { true, false }) {
            RectInfo outermost;

            sk_sp<GrTextureProxy> proxy = create_proxy(context, isPowerOfTwoSized,
                                                       isExact, &outermost);
            SkASSERT(outermost.isHardOrBadAllAround());

            for (auto isConstraintRectHard : { true, false }) {
                for (auto areCoordsLimitedToConstraintRect : { true, false }) {
                    for (int filterMode = 0; filterMode < 4; ++filterMode) {
                        for (auto constraintRectMaker : { left_only, top_only, right_only,
                            bot_only, full_inset, no_inset }) {
                            for (auto insetAmt : { 0.25f, 0.75f, 1.25f, 1.75f, 5.0f }) {
                                RectInfo constraintRectStorage;
                                const SkRect* constraintRect = (*constraintRectMaker)(
                                        outermost,
                                        &constraintRectStorage,
                                        isConstraintRectHard,
                                        areCoordsLimitedToConstraintRect,
                                        insetAmt,
                                        gHalfFilterWidth[filterMode]);
                                SkASSERT(constraintRect); // always need one of these
                                SkASSERT(outermost.rect().contains(*constraintRect));

                                actualMode = GrTextureProducer_TestAccess::DetermineDomainMode(
                                        *constraintRect,
                                        isConstraintRectHard
                                            ? GrTextureProducer::kYes_FilterConstraint
                                            : GrTextureProducer::kNo_FilterConstraint,
                                        areCoordsLimitedToConstraintRect,
                                        proxy.get(),
                                        gModePtrs[filterMode],
                                        &actualDomainRect);

                                expectedMode = DomainMode::kNoDomain_DomainMode;
                                if (constraintRectStorage.hasABad()) {
                                    if (3 == filterMode) {
                                        expectedMode = DomainMode::kTightCopy_DomainMode;
                                    } else {
                                        expectedMode = DomainMode::kDomain_DomainMode;
                                    }
                                }

                                REPORTER_ASSERT(reporter, expectedMode == actualMode);
                                // TODO: add a check that the returned domain rect is correct
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

    proxy_test(reporter, context);
}
