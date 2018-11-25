/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipStackClip.h"

#include "GrAppliedClip.h"
#include "GrContextPriv.h"
#include "GrDeferredProxyUploader.h"
#include "GrDrawingManager.h"
#include "GrFixedClip.h"
#include "GrGpuResourcePriv.h"
#include "GrRenderTargetContextPriv.h"
#include "GrProxyProvider.h"
#include "GrStencilAttachment.h"
#include "GrSWMaskHelper.h"
#include "GrTextureProxy.h"
#include "effects/GrConvexPolyEffect.h"
#include "effects/GrRRectEffect.h"
#include "effects/GrTextureDomain.h"
#include "SkClipOpPriv.h"
#include "SkMakeUnique.h"
#include "SkTaskGroup.h"
#include "SkTraceEvent.h"

typedef SkClipStack::Element Element;
typedef GrReducedClip::InitialState InitialState;
typedef GrReducedClip::ElementList ElementList;

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
        *aa = GrAA(isAA);
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
static std::unique_ptr<GrFragmentProcessor> create_fp_for_mask(sk_sp<GrTextureProxy> mask,
                                                               const SkIRect& devBound) {
    SkIRect domainTexels = SkIRect::MakeWH(devBound.width(), devBound.height());
    return GrDeviceSpaceTextureDecalFragmentProcessor::Make(std::move(mask), domainTexels,
                                                            {devBound.fLeft, devBound.fTop});
}

// Does the path in 'element' require SW rendering? If so, return true (and,
// optionally, set 'prOut' to NULL. If not, return false (and, optionally, set
// 'prOut' to the non-SW path renderer that will do the job).
bool GrClipStackClip::PathNeedsSWRenderer(GrContext* context,
                                          const SkIRect& scissorRect,
                                          bool hasUserStencilSettings,
                                          const GrRenderTargetContext* renderTargetContext,
                                          const SkMatrix& viewMatrix,
                                          const Element* element,
                                          GrPathRenderer** prOut,
                                          bool needsStencil) {
    if (Element::DeviceSpaceType::kRect == element->getDeviceSpaceType()) {
        // rects can always be drawn directly w/o using the software path
        // TODO: skip rrects once we're drawing them directly.
        if (prOut) {
            *prOut = nullptr;
        }
        return false;
    } else {
        // We shouldn't get here with an empty clip element.
        SkASSERT(Element::DeviceSpaceType::kEmpty != element->getDeviceSpaceType());

        // the gpu alpha mask will draw the inverse paths as non-inverse to a temp buffer
        SkPath path;
        element->asDeviceSpacePath(&path);
        if (path.isInverseFillType()) {
            path.toggleInverseFillType();
        }

        GrPathRendererChain::DrawType type =
                needsStencil ? GrPathRendererChain::DrawType::kStencilAndColor
                             : GrPathRendererChain::DrawType::kColor;

        GrShape shape(path, GrStyle::SimpleFill());
        GrPathRenderer::CanDrawPathArgs canDrawArgs;
        canDrawArgs.fCaps = context->caps();
        canDrawArgs.fClipConservativeBounds = &scissorRect;
        canDrawArgs.fViewMatrix = &viewMatrix;
        canDrawArgs.fShape = &shape;
        canDrawArgs.fAAType = GrChooseAAType(GrAA(element->isAA()),
                                             renderTargetContext->fsaaType(),
                                             GrAllowMixedSamples::kYes,
                                             *context->caps());
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

    // If we're avoiding stencils, always use SW:
    if (context->caps()->avoidStencilBuffers())
        return true;

    // Set the matrix so that rendered clip elements are transformed to mask space from clip
    // space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-reducedClip.left()), SkIntToScalar(-reducedClip.top()));

    for (ElementList::Iter iter(reducedClip.maskElements()); iter.get(); iter.next()) {
        const Element* element = iter.get();

        SkClipOp op = element->getOp();
        bool invert = element->isInverseFilled();
        bool needsStencil = invert ||
                            kIntersect_SkClipOp == op || kReverseDifference_SkClipOp == op;

        if (PathNeedsSWRenderer(context, reducedClip.scissor(), hasUserStencilSettings,
                                renderTargetContext, translate, element, nullptr, needsStencil)) {
            return true;
        }
    }
    return false;
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

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    const auto* caps = context->caps()->shaderCaps();
    int maxWindowRectangles = renderTargetContext->priv().maxWindowRectangles();
    int maxAnalyticFPs = context->caps()->maxClipAnalyticFPs();
    if (GrFSAAType::kNone != renderTargetContext->fsaaType()) {
        // With mixed samples (non-msaa color buffer), any coverage info is lost from color once it
        // hits the color buffer anyway, so we may as well use coverage AA if nothing else in the
        // pipe is multisampled.
        if (renderTargetContext->numColorSamples() > 0 || useHWAA || hasUserStencilSettings) {
            maxAnalyticFPs = 0;
        }
        SkASSERT(!context->caps()->avoidStencilBuffers()); // We disable MSAA when avoiding stencil.
    }
    auto* ccpr = context->contextPriv().drawingManager()->getCoverageCountingPathRenderer();

    GrReducedClip reducedClip(*fStack, devBounds, caps, maxWindowRectangles, maxAnalyticFPs, ccpr);
    if (InitialState::kAllOut == reducedClip.initialState() &&
        reducedClip.maskElements().isEmpty()) {
        return false;
    }

    if (reducedClip.hasScissor() && !GrClip::IsInsideClip(reducedClip.scissor(), devBounds)) {
        out->hardClip().addScissor(reducedClip.scissor(), bounds);
    }

    if (!reducedClip.windowRectangles().empty()) {
        out->hardClip().addWindowRectangles(reducedClip.windowRectangles(),
                                            GrWindowRectsState::Mode::kExclusive);
    }

    if (!reducedClip.maskElements().isEmpty()) {
        if (!this->applyClipMask(context, renderTargetContext, reducedClip, hasUserStencilSettings,
                                 out)) {
            return false;
        }
    }

    // The opList ID must not be looked up until AFTER producing the clip mask (if any). That step
    // can cause a flush or otherwise change which opList our draw is going into.
    uint32_t opListID = renderTargetContext->getOpList()->uniqueID();
    int rtWidth = renderTargetContext->width(), rtHeight = renderTargetContext->height();
    if (auto clipFPs = reducedClip.finishAndDetachAnalyticFPs(proxyProvider, opListID,
                                                              rtWidth, rtHeight)) {
        out->addCoverageFP(std::move(clipFPs));
    }

    return true;
}

bool GrClipStackClip::applyClipMask(GrContext* context, GrRenderTargetContext* renderTargetContext,
                                    const GrReducedClip& reducedClip, bool hasUserStencilSettings,
                                    GrAppliedClip* out) const {
#ifdef SK_DEBUG
    SkASSERT(reducedClip.hasScissor());
    SkIRect rtIBounds = SkIRect::MakeWH(renderTargetContext->width(),
                                        renderTargetContext->height());
    const SkIRect& scissor = reducedClip.scissor();
    SkASSERT(rtIBounds.contains(scissor)); // Mask shouldn't be larger than the RT.
#endif

    // If the stencil buffer is multisampled we can use it to do everything.
    if ((GrFSAAType::kNone == renderTargetContext->fsaaType() && reducedClip.maskRequiresAA()) ||
        context->caps()->avoidStencilBuffers()) {
        sk_sp<GrTextureProxy> result;
        if (UseSWOnlyPath(context, hasUserStencilSettings, renderTargetContext, reducedClip)) {
            // The clip geometry is complex enough that it will be more efficient to create it
            // entirely in software
            result = this->createSoftwareClipMask(context, reducedClip, renderTargetContext);
        } else {
            result = this->createAlphaClipMask(context, reducedClip);
        }

        if (result) {
            // The mask's top left coord should be pinned to the rounded-out top left corner of
            // the clip's device space bounds.
            out->addCoverageFP(create_fp_for_mask(std::move(result), reducedClip.scissor()));
            return true;
        }

        // If alpha or software clip mask creation fails, fall through to the stencil code paths,
        // unless stencils are disallowed.
        if (context->caps()->avoidStencilBuffers()) {
            SkDebugf("WARNING: Clip mask requires stencil, but stencil unavailable. "
                     "Clip will be ignored.\n");
            return false;
        }
    }

    renderTargetContext->setNeedsStencil();

    // This relies on the property that a reduced sub-rect of the last clip will contain all the
    // relevant window rectangles that were in the last clip. This subtle requirement will go away
    // after clipping is overhauled.
    if (renderTargetContext->priv().mustRenderClip(reducedClip.maskGenID(), reducedClip.scissor(),
                                                   reducedClip.numAnalyticFPs())) {
        reducedClip.drawStencilClipMask(context, renderTargetContext);
        renderTargetContext->priv().setLastClip(reducedClip.maskGenID(), reducedClip.scissor(),
                                                reducedClip.numAnalyticFPs());
    }
    // GrAppliedClip doesn't need to figure numAnalyticFPs into its key (used by operator==) because
    // it verifies the FPs are also equal.
    out->hardClip().addStencilClip(reducedClip.maskGenID());
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static void create_clip_mask_key(uint32_t clipGenID, const SkIRect& bounds, int numAnalyticFPs,
                                 GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 4, GrClipStackClip::kMaskTestTag);
    builder[0] = clipGenID;
    // SkToS16 because image filters outset layers to a size indicated by the filter, which can
    // sometimes result in negative coordinates from device space.
    builder[1] = SkToS16(bounds.fLeft) | (SkToS16(bounds.fRight) << 16);
    builder[2] = SkToS16(bounds.fTop) | (SkToS16(bounds.fBottom) << 16);
    builder[3] = numAnalyticFPs;
}

static void add_invalidate_on_pop_message(const SkClipStack& stack, uint32_t clipGenID,
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
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.maskGenID(), reducedClip.scissor(),
                         reducedClip.numAnalyticFPs(), &key);

    sk_sp<GrTextureProxy> proxy(proxyProvider->findOrCreateProxyByUniqueKey(
                                                                key, kBottomLeft_GrSurfaceOrigin));
    if (proxy) {
        return proxy;
    }

    sk_sp<GrRenderTargetContext> rtc(context->makeDeferredRenderTargetContextWithFallback(
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

    SkASSERT(result->origin() == kBottomLeft_GrSurfaceOrigin);
    proxyProvider->assignUniqueKeyToProxy(key, result.get());
    add_invalidate_on_pop_message(*fStack, reducedClip.maskGenID(), key);

    return result;
}

namespace {

/**
 * Payload class for use with GrTDeferredProxyUploader. The clip mask code renders multiple
 * elements, each storing their own AA setting (and already transformed into device space). This
 * stores all of the information needed by the worker thread to draw all clip elements (see below,
 * in createSoftwareClipMask).
 */
class ClipMaskData {
public:
    ClipMaskData(const GrReducedClip& reducedClip)
            : fScissor(reducedClip.scissor())
            , fInitialState(reducedClip.initialState()) {
        for (ElementList::Iter iter(reducedClip.maskElements()); iter.get(); iter.next()) {
            fElements.addToTail(*iter.get());
        }
    }

    const SkIRect& scissor() const { return fScissor; }
    InitialState initialState() const { return fInitialState; }
    const ElementList& elements() const { return fElements; }

private:
    SkIRect fScissor;
    InitialState fInitialState;
    ElementList fElements;
};

}

static void draw_clip_elements_to_mask_helper(GrSWMaskHelper& helper, const ElementList& elements,
                                              const SkIRect& scissor, InitialState initialState) {
    // Set the matrix so that rendered clip elements are transformed to mask space from clip space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-scissor.left()), SkIntToScalar(-scissor.top()));

    helper.clear(InitialState::kAllIn == initialState ? 0xFF : 0x00);

    for (ElementList::Iter iter(elements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkClipOp op = element->getOp();
        GrAA aa = GrAA(element->isAA());

        if (kIntersect_SkClipOp == op || kReverseDifference_SkClipOp == op) {
            // Intersect and reverse difference require modifying pixels outside of the geometry
            // that is being "drawn". In both cases we erase all the pixels outside of the geometry
            // but leave the pixels inside the geometry alone. For reverse difference we invert all
            // the pixels before clearing the ones outside the geometry.
            if (kReverseDifference_SkClipOp == op) {
                SkRect temp = SkRect::Make(scissor);
                // invert the entire scene
                helper.drawRect(temp, translate, SkRegion::kXOR_Op, GrAA::kNo, 0xFF);
            }
            SkPath clipPath;
            element->asDeviceSpacePath(&clipPath);
            clipPath.toggleInverseFillType();
            GrShape shape(clipPath, GrStyle::SimpleFill());
            helper.drawShape(shape, translate, SkRegion::kReplace_Op, aa, 0x00);
            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (Element::DeviceSpaceType::kRect == element->getDeviceSpaceType()) {
            helper.drawRect(element->getDeviceSpaceRect(), translate, (SkRegion::Op)op, aa, 0xFF);
        } else {
            SkPath path;
            element->asDeviceSpacePath(&path);
            GrShape shape(path, GrStyle::SimpleFill());
            helper.drawShape(shape, translate, (SkRegion::Op)op, aa, 0xFF);
        }
    }
}

sk_sp<GrTextureProxy> GrClipStackClip::createSoftwareClipMask(
        GrContext* context, const GrReducedClip& reducedClip,
        GrRenderTargetContext* renderTargetContext) const {
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.maskGenID(), reducedClip.scissor(),
                         reducedClip.numAnalyticFPs(), &key);

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    sk_sp<GrTextureProxy> proxy(proxyProvider->findOrCreateProxyByUniqueKey(
                                                                  key, kTopLeft_GrSurfaceOrigin));
    if (proxy) {
        return proxy;
    }

    // The mask texture may be larger than necessary. We round out the clip bounds and pin the top
    // left corner of the resulting rect to the top left of the texture.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(reducedClip.width(), reducedClip.height());

    SkTaskGroup* taskGroup = context->contextPriv().getTaskGroup();
    if (taskGroup && renderTargetContext) {
        // Create our texture proxy
        GrSurfaceDesc desc;
        desc.fOrigin = kTopLeft_GrSurfaceOrigin;
        desc.fWidth = maskSpaceIBounds.width();
        desc.fHeight = maskSpaceIBounds.height();
        desc.fConfig = kAlpha_8_GrPixelConfig;
        // MDB TODO: We're going to fill this proxy with an ASAP upload (which is out of order wrt
        // to ops), so it can't have any pending IO.
        proxy = proxyProvider->createProxy(desc, SkBackingFit::kApprox, SkBudgeted::kYes,
                                           GrResourceProvider::kNoPendingIO_Flag);

        auto uploader = skstd::make_unique<GrTDeferredProxyUploader<ClipMaskData>>(reducedClip);
        GrTDeferredProxyUploader<ClipMaskData>* uploaderRaw = uploader.get();
        auto drawAndUploadMask = [uploaderRaw, maskSpaceIBounds] {
            TRACE_EVENT0("skia", "Threaded SW Clip Mask Render");
            GrSWMaskHelper helper(uploaderRaw->getPixels());
            if (helper.init(maskSpaceIBounds)) {
                draw_clip_elements_to_mask_helper(helper, uploaderRaw->data().elements(),
                                                  uploaderRaw->data().scissor(),
                                                  uploaderRaw->data().initialState());
            } else {
                SkDEBUGFAIL("Unable to allocate SW clip mask.");
            }
            uploaderRaw->signalAndFreeData();
        };

        taskGroup->add(std::move(drawAndUploadMask));
        proxy->texPriv().setDeferredUploader(std::move(uploader));
    } else {
        GrSWMaskHelper helper;
        if (!helper.init(maskSpaceIBounds)) {
            return nullptr;
        }

        draw_clip_elements_to_mask_helper(helper, reducedClip.maskElements(), reducedClip.scissor(),
                                          reducedClip.initialState());

        proxy = helper.toTextureProxy(context, SkBackingFit::kApprox);
    }

    SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
    proxyProvider->assignUniqueKeyToProxy(key, proxy.get());
    add_invalidate_on_pop_message(*fStack, reducedClip.maskGenID(), key);
    return proxy;
}
