/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClipStackClip.h"

#include "include/gpu/GrDirectContext.h"
#include "include/private/SkTo.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSWMaskHelper.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

typedef SkClipStack::Element Element;
typedef GrReducedClip::InitialState InitialState;
typedef GrReducedClip::ElementList ElementList;

const char GrClipStackClip::kMaskTestTag[] = "clip_mask";

GrClip::PreClipResult GrClipStackClip::preApply(const SkRect& drawBounds, GrAA aa) const {
    SkIRect deviceRect = SkIRect::MakeSize(fDeviceSize);
    SkRect rect = SkRect::Make(deviceRect);
    if (!rect.intersect(drawBounds) || (fStack && fStack->isEmpty(deviceRect))) {
        return Effect::kClippedOut;
    } else if (!fStack || fStack->isWideOpen()) {
        return Effect::kUnclipped;
    }

    PreClipResult result(Effect::kClipped);
    bool isAA;
    // SkClipStack does not have a way to distinguish "not a rrect" vs. "rrect that doesn't
    // intersect the draw", so pass in the device bounds and then check the returned shape for
    // intersection afterwards.
    if (fStack->isRRect(SkRect::Make(deviceRect), &result.fRRect, &isAA)) {
        if (!result.fRRect.getBounds().intersects(rect)) {
            return Effect::kClippedOut;
        }
        result.fIsRRect = true;
        result.fAA = GrAA(isAA);
    }
    return result;
}

SkIRect GrClipStackClip::getConservativeBounds() const {
    if (fStack) {
        SkRect devBounds;
        fStack->getConservativeBounds(0, 0, fDeviceSize.fWidth, fDeviceSize.fHeight, &devBounds);
        return devBounds.roundOut();
    } else {
        return SkIRect::MakeSize(fDeviceSize);
    }
}

////////////////////////////////////////////////////////////////////////////////
// set up the draw state to enable the aa clipping mask.
static std::unique_ptr<GrFragmentProcessor> create_fp_for_mask(GrSurfaceProxyView mask,
                                                               const SkIRect& devBound,
                                                               const GrCaps& caps) {
    GrSamplerState samplerState(GrSamplerState::WrapMode::kClampToBorder,
                                GrSamplerState::Filter::kNearest);
    auto m = SkMatrix::Translate(-devBound.fLeft, -devBound.fTop);
    auto subset = SkRect::Make(devBound.size());
    // We scissor to devBounds. The mask's texel centers are aligned to device space
    // pixel centers. Hence this domain of texture coordinates.
    auto domain = subset.makeInset(0.5, 0.5);
    auto fp = GrTextureEffect::MakeSubset(std::move(mask), kPremul_SkAlphaType, m, samplerState,
                                          subset, domain, caps);
    fp = GrBlendFragmentProcessor::Make(std::move(fp), nullptr, SkBlendMode::kDstIn);
    return GrFragmentProcessor::DeviceSpace(std::move(fp));
}

/*
 * This method traverses the clip stack to see if the GrSoftwarePathRenderer
 * will be used on any element. If so, it returns true to indicate that the
 * entire clip should be rendered in SW and then uploaded en masse to the gpu.
 */
bool GrClipStackClip::UseSWOnlyPath(GrRecordingContext*,
                                    const skgpu::v1::SurfaceDrawContext*,
                                    const GrReducedClip& /* reducedClip */) {
    // TODO: right now it appears that GPU clip masks are strictly slower than software. We may
    // want to revisit this assumption once we can test with render target sorting.
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// sort out what kind of clip mask needs to be created: alpha, stencil,
// scissor, or entirely software
GrClip::Effect GrClipStackClip::apply(GrRecordingContext* rContext,
                                      skgpu::v1::SurfaceDrawContext* sdc,
                                      GrDrawOp*,
                                      GrAAType aa,
                                      GrAppliedClip* out,
                                      SkRect* bounds) const {
    SkASSERT(sdc->width() == fDeviceSize.fWidth &&
             sdc->height() == fDeviceSize.fHeight);
    SkRect devBounds = SkRect::MakeIWH(fDeviceSize.fWidth, fDeviceSize.fHeight);
    if (!devBounds.intersect(*bounds)) {
        return Effect::kClippedOut;
    }

    if (!fStack || fStack->isWideOpen()) {
        return Effect::kUnclipped;
    }

    // An default count of 4 was chosen because of the common pattern in Blink of:
    //   isect RR
    //   diff  RR
    //   isect convex_poly
    //   isect convex_poly
    // when drawing rounded div borders.
    constexpr int kMaxAnalyticElements = 4;

    int maxWindowRectangles = sdc->maxWindowRectangles();
    int maxAnalyticElements = kMaxAnalyticElements;
    if (sdc->numSamples() > 1 || aa == GrAAType::kMSAA) {
        // Disable analytic clips when we have MSAA. In MSAA we never conflate coverage and opacity.
        maxAnalyticElements = 0;
        // We disable MSAA when stencil isn't supported.
        SkASSERT(sdc->asRenderTargetProxy()->canUseStencil(*rContext->priv().caps()));
    }

    GrReducedClip reducedClip(*fStack, devBounds, rContext->priv().caps(), maxWindowRectangles,
                              maxAnalyticElements);
    if (InitialState::kAllOut == reducedClip.initialState() &&
        reducedClip.maskElements().isEmpty()) {
        return Effect::kClippedOut;
    }

    Effect effect = Effect::kUnclipped;
    if (reducedClip.hasScissor() && !GrClip::IsInsideClip(reducedClip.scissor(), devBounds)) {
        out->hardClip().addScissor(reducedClip.scissor(), bounds);
        effect = Effect::kClipped;
    }

    if (!reducedClip.windowRectangles().empty()) {
        out->hardClip().addWindowRectangles(reducedClip.windowRectangles(),
                                            GrWindowRectsState::Mode::kExclusive);
        effect = Effect::kClipped;
    }

    if (!reducedClip.maskElements().isEmpty()) {
        if (!this->applyClipMask(rContext, sdc, reducedClip, out)) {
            return Effect::kClippedOut;
        }
        effect = Effect::kClipped;
    }

    // The opsTask ID must not be looked up until AFTER producing the clip mask (if any). That step
    // can cause a flush or otherwise change which opstask our draw is going into.
    uint32_t opsTaskID = sdc->getOpsTask()->uniqueID();
    auto [success, clipFPs] = reducedClip.finishAndDetachAnalyticElements(rContext,
                                                                          *fMatrixProvider,
                                                                          opsTaskID);
    if (success) {
        out->addCoverageFP(std::move(clipFPs));
        effect = Effect::kClipped;
    } else {
        effect = Effect::kClippedOut;
    }

    return effect;
}

bool GrClipStackClip::applyClipMask(GrRecordingContext* rContext,
                                    skgpu::v1::SurfaceDrawContext* sdc,
                                    const GrReducedClip& reducedClip,
                                    GrAppliedClip* out) const {
#ifdef SK_DEBUG
    SkASSERT(reducedClip.hasScissor());
    SkIRect rtIBounds = SkIRect::MakeWH(sdc->width(), sdc->height());
    const SkIRect& scissor = reducedClip.scissor();
    SkASSERT(rtIBounds.contains(scissor)); // Mask shouldn't be larger than the RT.
#endif

    if ((sdc->numSamples() <= 1 && reducedClip.maskRequiresAA()) ||
        !sdc->asRenderTargetProxy()->canUseStencil(*rContext->priv().caps())) {
        GrSurfaceProxyView result;
        if (UseSWOnlyPath(rContext, sdc, reducedClip)) {
            // The clip geometry is complex enough that it will be more efficient to create it
            // entirely in software
            result = this->createSoftwareClipMask(rContext, reducedClip, sdc);
        } else {
            result = this->createAlphaClipMask(rContext, reducedClip);
        }

        if (result) {
            // The mask's top left coord should be pinned to the rounded-out top left corner of
            // the clip's device space bounds.
            out->addCoverageFP(create_fp_for_mask(std::move(result), reducedClip.scissor(),
                                                  *rContext->priv().caps()));
            return true;
        }

        // If alpha or software clip mask creation fails, fall through to the stencil code paths,
        // unless stencils are disallowed.
        if (!sdc->asRenderTargetProxy()->canUseStencil(*rContext->priv().caps())) {
            SkDebugf("WARNING: Clip mask requires stencil, but stencil unavailable. "
                     "Clip will be ignored.\n");
            return false;
        }
    }

    reducedClip.drawStencilClipMask(rContext, sdc);
    out->hardClip().addStencilClip(reducedClip.maskGenID());
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static void create_clip_mask_key(uint32_t clipGenID, const SkIRect& bounds, int numAnalyticElements,
                                 GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 4, GrClipStackClip::kMaskTestTag);
    builder[0] = clipGenID;
    // SkToS16 because image filters outset layers to a size indicated by the filter, which can
    // sometimes result in negative coordinates from device space.
    builder[1] = SkToS16(bounds.fLeft) | (SkToS16(bounds.fRight) << 16);
    builder[2] = SkToS16(bounds.fTop) | (SkToS16(bounds.fBottom) << 16);
    builder[3] = numAnalyticElements;
}

static void add_invalidate_on_pop_message(GrRecordingContext* context,
                                          const SkClipStack& stack, uint32_t clipGenID,
                                          const GrUniqueKey& clipMaskKey) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
    while (const Element* element = iter.prev()) {
        if (element->getGenID() == clipGenID) {
            element->addResourceInvalidationMessage(proxyProvider, clipMaskKey);
            return;
        }
    }
    SkDEBUGFAIL("Gen ID was not found in stack.");
}

static constexpr auto kMaskOrigin = kTopLeft_GrSurfaceOrigin;

static GrSurfaceProxyView find_mask(GrProxyProvider* provider, const GrUniqueKey& key) {
    return provider->findCachedProxyWithColorTypeFallback(key, kMaskOrigin, GrColorType::kAlpha_8,
                                                          1);
}

GrSurfaceProxyView GrClipStackClip::createAlphaClipMask(GrRecordingContext* rContext,
                                                        const GrReducedClip& reducedClip) const {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.maskGenID(), reducedClip.scissor(),
                         reducedClip.numAnalyticElements(), &key);

    if (auto cachedView = find_mask(rContext->priv().proxyProvider(), key)) {
        return cachedView;
    }

    auto sdc = skgpu::v1::SurfaceDrawContext::MakeWithFallback(
            rContext, GrColorType::kAlpha_8, nullptr, SkBackingFit::kApprox,
            {reducedClip.width(), reducedClip.height()}, SkSurfaceProps(), 1, GrMipmapped::kNo,
            GrProtected::kNo, kMaskOrigin);
    if (!sdc) {
        return {};
    }

    if (!reducedClip.drawAlphaClipMask(sdc.get())) {
        return {};
    }

    GrSurfaceProxyView result = sdc->readSurfaceView();
    if (!result || !result.asTextureProxy()) {
        return {};
    }

    SkASSERT(result.origin() == kMaskOrigin);
    proxyProvider->assignUniqueKeyToProxy(key, result.asTextureProxy());
    add_invalidate_on_pop_message(rContext, *fStack, reducedClip.maskGenID(), key);

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

}  // namespace

static void draw_clip_elements_to_mask_helper(GrSWMaskHelper& helper, const ElementList& elements,
                                              const SkIRect& scissor, InitialState initialState) {
    // Set the matrix so that rendered clip elements are transformed to mask space from clip space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-scissor.left()), SkIntToScalar(-scissor.top()));

    helper.clear(InitialState::kAllIn == initialState ? 0xFF : 0x00);

    for (ElementList::Iter iter(elements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = element->getRegionOp();
        GrAA aa = GrAA(element->isAA());

        if (SkRegion::kIntersect_Op == op) {
            // Intersect and reverse difference require modifying pixels outside of the geometry
            // that is being "drawn". In both cases we erase all the pixels outside of the geometry
            // but leave the pixels inside the geometry alone. For reverse difference we invert all
            // the pixels before clearing the ones outside the geometry.
            if (SkRegion::kReverseDifference_Op == op) {
                SkRect temp = SkRect::Make(scissor);
                // invert the entire scene
                helper.drawRect(temp, translate, SkRegion::kXOR_Op, GrAA::kNo, 0xFF);
            }
            SkPath clipPath;
            element->asDeviceSpacePath(&clipPath);
            clipPath.toggleInverseFillType();
            helper.drawShape(GrShape(clipPath), translate, SkRegion::kReplace_Op, aa, 0x00);
            continue;
        }

        // The other ops (union, xor, diff) only affect pixels inside
        // the geometry so they can just be drawn normally
        if (Element::DeviceSpaceType::kRect == element->getDeviceSpaceType()) {
            helper.drawRect(element->getDeviceSpaceRect(), translate, op, aa, 0xFF);
        } else if (Element::DeviceSpaceType::kRRect == element->getDeviceSpaceType()) {
            helper.drawRRect(element->getDeviceSpaceRRect(), translate, op, aa, 0xFF);
        } else {
            SkPath path;
            element->asDeviceSpacePath(&path);
            helper.drawShape(GrShape(path), translate, op, aa, 0xFF);
        }
    }
}

GrSurfaceProxyView GrClipStackClip::createSoftwareClipMask(
        GrRecordingContext* rContext,
        const GrReducedClip& reducedClip,
        skgpu::v1::SurfaceDrawContext* sdc) const {
    GrUniqueKey key;
    create_clip_mask_key(reducedClip.maskGenID(), reducedClip.scissor(),
                         reducedClip.numAnalyticElements(), &key);

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    if (auto cachedView = find_mask(proxyProvider, key)) {
        return cachedView;
    }

    // The mask texture may be larger than necessary. We round out the clip bounds and pin the top
    // left corner of the resulting rect to the top left of the texture.
    SkIRect maskSpaceIBounds = SkIRect::MakeWH(reducedClip.width(), reducedClip.height());

    SkTaskGroup* taskGroup = nullptr;
    if (auto direct = rContext->asDirectContext()) {
        taskGroup = direct->priv().getTaskGroup();
    }

    GrSurfaceProxyView view;
    if (taskGroup && sdc) {
        const GrCaps* caps = rContext->priv().caps();
        // Create our texture proxy
        GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                               GrRenderable::kNo);

        GrSwizzle swizzle = rContext->priv().caps()->getReadSwizzle(format, GrColorType::kAlpha_8);

        // MDB TODO: We're going to fill this proxy with an ASAP upload (which is out of order wrt
        // to ops), so it can't have any pending IO.
        auto proxy = proxyProvider->createProxy(format,
                                                maskSpaceIBounds.size(),
                                                GrRenderable::kNo,
                                                1,
                                                GrMipmapped::kNo,
                                                SkBackingFit::kApprox,
                                                SkBudgeted::kYes,
                                                GrProtected::kNo);

        auto uploader = std::make_unique<GrTDeferredProxyUploader<ClipMaskData>>(reducedClip);
        GrTDeferredProxyUploader<ClipMaskData>* uploaderRaw = uploader.get();
        auto drawAndUploadMask = [uploaderRaw, maskSpaceIBounds] {
            TRACE_EVENT0("skia.gpu", "Threaded SW Clip Mask Render");
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

        view = {std::move(proxy), kMaskOrigin, swizzle};
    } else {
        GrSWMaskHelper helper;
        if (!helper.init(maskSpaceIBounds)) {
            return {};
        }

        draw_clip_elements_to_mask_helper(helper, reducedClip.maskElements(), reducedClip.scissor(),
                                          reducedClip.initialState());

        view = helper.toTextureView(rContext, SkBackingFit::kApprox);
    }

    SkASSERT(view);
    SkASSERT(view.origin() == kMaskOrigin);
    proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    add_invalidate_on_pop_message(rContext, *fStack, reducedClip.maskGenID(), key);
    return view;
}
