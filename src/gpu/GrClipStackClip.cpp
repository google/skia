/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipStackClip.h"

#include "GrAppliedClip.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrRenderTargetContextPriv.h"
#include "GrFixedClip.h"
#include "GrGpuResourcePriv.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrSWMaskHelper.h"
#include "GrTextureProxy.h"
#include "effects/GrConvexPolyEffect.h"
#include "effects/GrRRectEffect.h"
#include "effects/GrTextureDomain.h"
#include "SkClipOpPriv.h"

typedef SkClipStack::Element Element;
typedef GrReducedClip::InitialState InitialState;
typedef GrReducedClip::ElementList ElementList;

static const int kMaxAnalyticElements = 4;
const char GrClipStackClip::kMaskTestTag[] = "clip_mask";

bool GrClipStackClip::quickContains(const SkRect& rect) const {
    if (!fStack || fStack->isWideOpen()) {
        return true;
    }
    return fStack->quickContains(rect);
}

bool GrClipStackClip::quickContains(const SkRRect& rrect) const {
    if (!fStack || fStack->isWideOpen()) {
        return true;
    }
    return fStack->quickContains(rrect);
}

bool GrClipStackClip::isRRect(const SkRect& origRTBounds, SkRRect* rr, GrAA* aa) const {
    if (!fStack) {
        return false;
    }
    const SkRect* rtBounds = &origRTBounds;
    bool isAA;
    if (fStack->isRRect(*rtBounds, rr, &isAA)) {
        *aa = GrBoolToAA(isAA);
        return true;
    }
    return false;
}

void GrClipStackClip::getConservativeBounds(int width, int height, SkIRect* devResult,
                                            bool* isIntersectionOfRects) const {
    if (!fStack) {
        devResult->setXYWH(0, 0, width, height);
        if (isIntersectionOfRects) {
            *isIntersectionOfRects = true;
        }
        return;
    }
    SkRect devBounds;
    fStack->getConservativeBounds(0, 0, width, height, &devBounds, isIntersectionOfRects);
    devBounds.roundOut(devResult);
}

////////////////////////////////////////////////////////////////////////////////
// set up the draw state to enable the aa clipping mask.
static sk_sp<GrFragmentProcessor> create_fp_for_mask(GrResourceProvider* resourceProvider,
                                                     sk_sp<GrTextureProxy> mask,
                                                     const SkIRect &devBound) {
    SkIRect domainTexels = SkIRect::MakeWH(devBound.width(), devBound.height());
    return GrDeviceSpaceTextureDecalFragmentProcessor::Make(resourceProvider,
                                                            std::move(mask), domainTexels,
                                                            {devBound.fLeft, devBound.fTop});
}

// Does the path in 'element' require SW rendering? If so, return true (and,
// optionally, set 'prOut' to NULL. If not, return false (and, optionally, set
// 'prOut' to the non-SW path renderer that will do the job).
bool GrClipStackClip::PathNeedsSWRenderer(GrContext* context,
                                          bool hasUserStencilSettings,
                                          const GrRenderTargetContext* renderTargetContext,
                                          const SkMatrix& viewMatrix,
                                          const Element* element,
                                          GrPathRenderer** prOut,
                                          bool needsStencil) {
    if (Element::kRect_Type == element->getType()) {
        // rects can always be drawn directly w/o using the software path
        // TODO: skip rrects once we're drawing them directly.
        if (prOut) {
            *prOut = nullptr;
        }
        return false;
    } else {
        // We shouldn't get here with an empty clip element.
        SkASSERT(Element::kEmpty_Type != element->getType());

        // the gpu alpha mask will draw the inverse paths as non-inverse to a temp buffer
        SkPath path;
        element->asPath(&path);
        if (path.isInverseFillType()) {
            path.toggleInverseFillType();
        }

        GrPathRendererChain::DrawType type =
                needsStencil ? GrPathRendererChain::DrawType::kStencilAndColor
                             : GrPathRendererChain::DrawType::kColor;

        GrShape shape(path, GrStyle::SimpleFill());
        GrPathRenderer::CanDrawPathArgs canDrawArgs;
        canDrawArgs.fShaderCaps = context->caps()->shaderCaps();
        canDrawArgs.fViewMatrix = &viewMatrix;
        canDrawArgs.fShape = &shape;
        if (!element->isAA()) {
            canDrawArgs.fAAType = GrAAType::kNone;
        } else if (renderTargetContext->isUnifiedMultisampled()) {
            canDrawArgs.fAAType = GrAAType::kMSAA;
        } else if (renderTargetContext->isStencilBufferMultisampled()){
            canDrawArgs.fAAType = GrAAType::kMixedSamples;
        } else {
            canDrawArgs.fAAType = GrAAType::kCoverage;
        }
        canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;

        // the 'false' parameter disallows use of the SW path renderer
        GrPathRenderer* pr =
            context->contextPriv().drawingManager()->getPathRenderer(canDrawArgs, false, type);
        if (prOut) {
            *prOut = pr;
        }
        return SkToBool(!pr);
    }
}

/*
 * This method traverses the clip stack to see if the GrSoftwarePathRenderer
 * will be used on any element. If so, it returns true to indicate that the
 * entire clip should be rendered in SW and then uploaded en masse to the gpu.
 */
bool GrClipStackClip::UseSWOnlyPath(GrContext* context,
                                    bool hasUserStencilSettings,
                                    const GrRenderTargetContext* renderTargetContext,
                                    const GrReducedClip& reducedClip) {
    // TODO: generalize this function so that when
    // a clip gets complex enough it can just be done in SW regardless
    // of whether it would invoke the GrSoftwarePathRenderer.

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-reducedClip.left()), SkIntToScalar(-reducedClip.top()));

    for (ElementList::Iter iter(reducedClip.elements()); iter.get(); iter.next()) {
        const Element* element = iter.get();

        SkClipOp op = element->getOp();
        bool invert = element->isInverseFilled();
        bool needsStencil = invert ||
                            kIntersect_SkClipOp == op || kReverseDifference_SkClipOp == op;

        if (PathNeedsSWRenderer(context, hasUserStencilSettings,
                                renderTargetContext, translate, element, nullptr, needsStencil)) {
            return true;
        }
    }
    return false;
}

static bool get_analytic_clip_processor(const ElementList& elements,
                                        bool abortIfAA,
                                        const SkRect& drawDevBounds,
                                        sk_sp<GrFragmentProcessor>* resultFP) {
    SkASSERT(elements.count() <= kMaxAnalyticElements);
    SkSTArray<kMaxAnalyticElements, sk_sp<GrFragmentProcessor>> fps;
    ElementList::Iter iter(elements);
    while (iter.get()) {
        SkClipOp op = iter.get()->getOp();
        bool invert;
        bool skip = false;
        switch (op) {
            case kReplace_SkClipOp:
                SkASSERT(iter.get() == elements.head());
                // Fallthrough, handled same as intersect.
            case kIntersect_SkClipOp:
                invert = false;
                if (iter.get()->contains(drawDevBounds)) {
                    skip = true;
                }
                break;
            case kDifference_SkClipOp:
                invert = true;
                // We don't currently have a cheap test for whether a rect is fully outside an
                // element's primitive, so don't attempt to set skip.
                break;
            default:
                return false;
        }
        if (!skip) {
            GrPrimitiveEdgeType edgeType;
            if (iter.get()->isAA()) {
                if (abortIfAA) {
                    return false;
                }
                edgeType =
                    invert ? kInverseFillAA_GrProcessorEdgeType : kFillAA_GrProcessorEdgeType;
            } else {
                edgeType =
                    invert ? kInverseFillBW_GrProcessorEdgeType : kFillBW_GrProcessorEdgeType;
            }

            switch (iter.get()->getType()) {
                case SkClipStack::Element::kPath_Type:
                    fps.emplace_back(GrConvexPolyEffect::Make(edgeType, iter.get()->getPath()));
                    break;
                case SkClipStack::Element::kRRect_Type: {
                    fps.emplace_back(GrRRectEffect::Make(edgeType, iter.get()->getRRect()));
                    break;
                }
                case SkClipStack::Element::kRect_Type: {
                    fps.emplace_back(GrConvexPolyEffect::Make(edgeType, iter.get()->getRect()));
                    break;
                }
                default:
                    break;
            }
            if (!fps.back()) {
                return false;
            }
        }
        iter.next();
    }

    *resultFP = nullptr;
    if (fps.count()) {
        *resultFP = GrFragmentProcessor::RunInSeries(fps.begin(), fps.count());
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
bool GrClipStackClip::apply(GrContext* context, GrRenderTargetContext* renderTargetContext,
                            bool useHWAA, bool hasUserStencilSettings, GrAppliedClip* out,
                            SkRect* bounds) const {
    SkRect devBounds = SkRect::MakeIWH(renderTargetContext->width(), renderTargetContext->height());
    if (!devBounds.intersect(*bounds)) {
        return false;
    }

    if (!fStack || fStack->isWideOpen()) {
        return true;
    }

    const GrReducedClip reducedClip(*fStack, devBounds,
                                    renderTargetContext->priv().maxWindowRectangles());

    if (reducedClip.hasIBounds() && !GrClip::IsInsideClip(reducedClip.ibounds(), devBounds)) {
        out->addScissor(reducedClip.ibounds(), bounds);
    }

    if (!reducedClip.windowRectangles().empty()) {
        out->addWindowRectangles(reducedClip.windowRectangles(),
                                 GrWindowRectsState::Mode::kExclusive);
    }

    if (reducedClip.elements().isEmpty()) {
        return InitialState::kAllIn == reducedClip.initialState();
    }

#ifdef SK_DEBUG
    SkASSERT(reducedClip.hasIBounds());
    SkIRect rtIBounds = SkIRect::MakeWH(renderTargetContext->width(),
                                        renderTargetContext->height());
    const SkIRect& clipIBounds = reducedClip.ibounds();
    SkASSERT(rtIBounds.contains(clipIBounds)); // Mask shouldn't be larger than the RT.
#endif

    // An element count of 4 was chosen because of the common pattern in Blink of:
    //   isect RR
    //   diff  RR
    //   isect convex_poly
    //   isect convex_poly
    // when drawing rounded div borders. This could probably be tuned based on a
    // configuration's relative costs of switching RTs to generate a mask vs
    // longer shaders.
    if (reducedClip.elements().count() <= kMaxAnalyticElements) {
        // When there are multiple samples we want to do per-sample clipping, not compute a
        // fractional pixel coverage.
        bool disallowAnalyticAA = renderTargetContext->isStencilBufferMultisampled();
        if (disallowAnalyticAA && !renderTargetContext->numColorSamples()) {
            // With a single color sample, any coverage info is lost from color once it hits the
            // color buffer anyway, so we may as well use coverage AA if nothing else in the pipe
            // is multisampled.
            disallowAnalyticAA = useHWAA || hasUserStencilSettings;
        }
        sk_sp<GrFragmentProcessor> clipFP;
        if (reducedClip.requiresAA() &&
            get_analytic_clip_processor(reducedClip.elements(), disallowAnalyticAA, devBounds,
                                        &clipFP)) {
            out->addCoverageFP(std::move(clipFP));
            return true;
        }
    }

    // If the stencil buffer is multisampled we can use it to do everything.
    if (!renderTargetContext->isStencilBufferMultisampled() && reducedClip.requiresAA()) {
        sk_sp<GrTextureProxy> result;
        if (UseSWOnlyPath(context, hasUserStencilSettings, renderTargetContext, reducedClip)) {
            // The clip geometry is complex enough that it will be more efficient to create it
            // entirely in software
            result = this->createSoftwareClipMask(context, reducedClip);
        } else {
            result = this->createAlphaClipMask(context, reducedClip);
        }

        if (result) {
            // The mask's top left coord should be pinned to the rounded-out top left corner of
            // the clip's device space bounds.
            out->addCoverageFP(create_fp_for_mask(context->resourceProvider(), std::move(result),
                                                  reducedClip.ibounds()));
            return true;
        }
        // if alpha clip mask creation fails fall through to the non-AA code paths
    }

    GrRenderTarget* rt = renderTargetContext->accessRenderTarget();
    if (!rt) {
        return true;
    }

    // use the stencil clip if we can't represent the clip as a rectangle.
    if (!context->resourceProvider()->attachStencilAttachment(rt)) {
        SkDebugf("WARNING: failed to attach stencil buffer for clip mask. Clip will be ignored.\n");
        return true;
    }

    // This relies on the property that a reduced sub-rect of the last clip will contain all the
    // relevant window rectangles that were in the last clip. This subtle requirement will go away
    // after clipping is overhauled.
    if (renderTargetContext->priv().mustRenderClip(reducedClip.elementsGenID(),
                                                   reducedClip.ibounds())) {
        reducedClip.drawStencilClipMask(context, renderTargetContext);
        renderTargetContext->priv().setLastClip(reducedClip.elementsGenID(), reducedClip.ibounds());
    }
    out->addStencilClip();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static void create_clip_mask_key(int32_t clipGenID, const SkIRect& bounds, GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 3, GrClipStackClip::kMaskTestTag);
    builder[0] = clipGenID;
    // SkToS16 because image filters outset layers to a size indicated by the filter, which can
    // sometimes result in negative coordinates from device space.
    builder[1] = SkToS16(bounds.fLeft) | (SkToS16(bounds.fRight) << 16);
    builder[2] = SkToS16(bounds.fTop) | (SkToS16(bounds.fBottom) << 16);
}

static void add_invalidate_on_pop_message(const SkClipStack& stack, int32_t clipGenID,
                                          const GrUniqueKey& clipMaskKey) {
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
    while (const Element* element = iter.prev()) {
        if (element->getGenID() == clipGenID) {
            std::unique_ptr<GrUniqueKeyInvalidatedMessage> msg(
                    new GrUniqueKeyInvalidatedMessage(clipMaskKey));
            element->addResourceInvalidationMessage(std::move(msg));
            return;
        }
    }
    SkDEBUGFAIL("Gen ID was not found in stack.");
}

sk_sp<GrTextureProxy> GrClipStackClip::createAlphaClipMask(GrContext* context,
                                                           const GrReducedClip& reducedClip) const {
    GrResourceProvider* resourceProvider = context->resourceProvider();
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.elementsGenID(), reducedClip.ibounds(), &key);

    sk_sp<GrTextureProxy> proxy(resourceProvider->findProxyByUniqueKey(key));
    if (proxy) {
        return proxy;
    }

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContextWithFallback(
                                                                             SkBackingFit::kApprox,
                                                                             reducedClip.width(),
                                                                             reducedClip.height(),
                                                                             kAlpha_8_GrPixelConfig,
                                                                             nullptr));
    if (!rtc) {
        return nullptr;
    }

    if (!reducedClip.drawAlphaClipMask(rtc.get())) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> result(rtc->asTextureProxyRef());
    if (!result) {
        return nullptr;
    }

    resourceProvider->assignUniqueKeyToProxy(key, result.get());
    // MDB TODO (caching): this has to play nice with the GrSurfaceProxy's caching
    add_invalidate_on_pop_message(*fStack, reducedClip.elementsGenID(), key);

    return result;
}

sk_sp<GrTextureProxy> GrClipStackClip::createSoftwareClipMask(
                                                          GrContext* context,
                                                          const GrReducedClip& reducedClip) const {
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.elementsGenID(), reducedClip.ibounds(), &key);

    sk_sp<GrTextureProxy> proxy(context->resourceProvider()->findProxyByUniqueKey(key));
    if (proxy) {
        return proxy;
    }

    // The mask texture may be larger than necessary. We round out the clip bounds and pin the top
    // left corner of the resulting rect to the top left of the texture.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(reducedClip.width(), reducedClip.height());

    GrSWMaskHelper helper;

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-reducedClip.left()), SkIntToScalar(-reducedClip.top()));

    if (!helper.init(maskSpaceIBounds, &translate)) {
        return nullptr;
    }
    helper.clear(InitialState::kAllIn == reducedClip.initialState() ? 0xFF : 0x00);

    for (ElementList::Iter iter(reducedClip.elements()); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkClipOp op = element->getOp();
        GrAA aa = GrBoolToAA(element->isAA());

        if (kIntersect_SkClipOp == op || kReverseDifference_SkClipOp == op) {
            // Intersect and reverse difference require modifying pixels outside of the geometry
            // that is being "drawn". In both cases we erase all the pixels outside of the geometry
            // but leave the pixels inside the geometry alone. For reverse difference we invert all
            // the pixels before clearing the ones outside the geometry.
            if (kReverseDifference_SkClipOp == op) {
                SkRect temp = SkRect::Make(reducedClip.ibounds());
                // invert the entire scene
                helper.drawRect(temp, SkRegion::kXOR_Op, GrAA::kNo, 0xFF);
            }
            SkPath clipPath;
            element->asPath(&clipPath);
            clipPath.toggleInverseFillType();
            GrShape shape(clipPath, GrStyle::SimpleFill());
            helper.drawShape(shape, SkRegion::kReplace_Op, aa, 0x00);
            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (Element::kRect_Type == element->getType()) {
            helper.drawRect(element->getRect(), (SkRegion::Op)op, aa, 0xFF);
        } else {
            SkPath path;
            element->asPath(&path);
            GrShape shape(path, GrStyle::SimpleFill());
            helper.drawShape(shape, (SkRegion::Op)op, aa, 0xFF);
        }
    }

    sk_sp<GrTextureProxy> result(helper.toTextureProxy(context, SkBackingFit::kApprox));

    context->resourceProvider()->assignUniqueKeyToProxy(key, result.get());
    // MDB TODO (caching): this has to play nice with the GrSurfaceProxy's caching
    add_invalidate_on_pop_message(*fStack, reducedClip.elementsGenID(), key);
    return result;
}
