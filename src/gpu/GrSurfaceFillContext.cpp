/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceFillContext.h"

#include "include/private/GrImageContext.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ops/GrClearOp.h"
#include "src/gpu/ops/GrFillRectOp.h"

#define ASSERT_SINGLE_OWNER        GR_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED        if (fContext->abandoned()) { return; }

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) {
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

static inline GrColorType color_type_fallback(GrColorType ct) {
    switch (ct) {
        // kRGBA_8888 is our default fallback for many color types that may not have renderable
        // backend formats.
        case GrColorType::kAlpha_8:
        case GrColorType::kBGR_565:
        case GrColorType::kABGR_4444:
        case GrColorType::kBGRA_8888:
        case GrColorType::kRGBA_1010102:
        case GrColorType::kBGRA_1010102:
        case GrColorType::kRGBA_F16:
        case GrColorType::kRGBA_F16_Clamped:
            return GrColorType::kRGBA_8888;
        case GrColorType::kAlpha_F16:
            return GrColorType::kRGBA_F16;
        case GrColorType::kGray_8:
            return GrColorType::kRGB_888x;
        default:
            return GrColorType::kUnknown;
    }
}

std::tuple<GrColorType, GrBackendFormat> GrSurfaceFillContext::GetFallbackColorTypeAndFormat(
        GrImageContext* context, GrColorType colorType, int sampleCnt) {
    auto caps = context->priv().caps();
    do {
        auto format = caps->getDefaultBackendFormat(colorType, GrRenderable::kYes);
        // We continue to the fallback color type if there no default renderable format or we
        // requested msaa and the format doesn't support msaa.
        if (format.isValid() && caps->isFormatRenderable(format, sampleCnt)) {
            return {colorType, format};
        }
        colorType = color_type_fallback(colorType);
    } while (colorType != GrColorType::kUnknown);
    return {GrColorType::kUnknown, {}};
}

std::unique_ptr<GrSurfaceFillContext> GrSurfaceFillContext::Make(GrRecordingContext* context,
                                                                 SkAlphaType alphaType,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 SkISize dimensions,
                                                                 SkBackingFit fit,
                                                                 const GrBackendFormat& format,
                                                                 int sampleCount,
                                                                 GrMipmapped mipmapped,
                                                                 GrProtected isProtected,
                                                                 GrSwizzle readSwizzle,
                                                                 GrSwizzle writeSwizzle,
                                                                 GrSurfaceOrigin origin,
                                                                 SkBudgeted budgeted) {
    SkASSERT(context);
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCount >= 1);
    SkASSERT(format.isValid() && format.backend() == context->backend());
    if (alphaType == kPremul_SkAlphaType || alphaType == kOpaque_SkAlphaType) {
        return GrSurfaceDrawContext::Make(context,
                                          std::move(colorSpace),
                                          fit,
                                          dimensions,
                                          format,
                                          sampleCount,
                                          mipmapped,
                                          isProtected,
                                          readSwizzle,
                                          writeSwizzle,
                                          origin,
                                          budgeted,
                                          /* surface props*/ nullptr);
    }

    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(format,
                                                                               dimensions,
                                                                               GrRenderable::kYes,
                                                                               sampleCount,
                                                                               mipmapped,
                                                                               fit,
                                                                               budgeted,
                                                                               isProtected);
    if (!proxy) {
        return nullptr;
    }
    GrImageInfo info(GrColorType::kUnknown, alphaType, std::move(colorSpace), dimensions);
    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
    auto fillContext = std::make_unique<GrSurfaceFillContext>(context,
                                                              std::move(readView),
                                                              std::move(writeView),
                                                              info.colorInfo());
    fillContext->discard();
    return fillContext;
}

std::unique_ptr<GrSurfaceFillContext> GrSurfaceFillContext::Make(GrRecordingContext* context,
                                                                 GrImageInfo info,
                                                                 SkBackingFit fit,
                                                                 int sampleCount,
                                                                 GrMipmapped mipmapped,
                                                                 GrProtected isProtected,
                                                                 GrSurfaceOrigin origin,
                                                                 SkBudgeted budgeted) {
    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return GrSurfaceDrawContext::Make(context,
                                          info.colorType(),
                                          info.refColorSpace(),
                                          fit,
                                          info.dimensions(),
                                          sampleCount,
                                          mipmapped,
                                          isProtected,
                                          origin,
                                          budgeted,
                                          nullptr);
    }
    GrBackendFormat format = context->priv().caps()->getDefaultBackendFormat(info.colorType(),
                                                                             GrRenderable::kYes);
    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(format,
                                                                               info.dimensions(),
                                                                               GrRenderable::kYes,
                                                                               sampleCount,
                                                                               mipmapped,
                                                                               fit,
                                                                               budgeted,
                                                                               isProtected);
    if (!proxy) {
        return nullptr;
    }
    GrSwizzle readSwizzle  = context->priv().caps()->getReadSwizzle (format, info.colorType());
    GrSwizzle writeSwizzle = context->priv().caps()->getWriteSwizzle(format, info.colorType());

    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
    auto fillContext = std::make_unique<GrSurfaceFillContext>(context,
                                                              std::move(readView),
                                                              std::move(writeView),
                                                              info.colorInfo());
    fillContext->discard();
    return fillContext;
}

std::unique_ptr<GrSurfaceFillContext> GrSurfaceFillContext::MakeWithFallback(
        GrRecordingContext* context,
        GrImageInfo info,
        SkBackingFit fit,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {
    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return GrSurfaceDrawContext::MakeWithFallback(context,
                                                      info.colorType(),
                                                      info.refColorSpace(),
                                                      fit,
                                                      info.dimensions(),
                                                      sampleCount,
                                                      mipmapped,
                                                      isProtected,
                                                      origin,
                                                      budgeted,
                                                      nullptr);
    }
    auto [ct, _] = GetFallbackColorTypeAndFormat(context, info.colorType(), sampleCount);
    if (ct == GrColorType::kUnknown) {
        return nullptr;
    }
    info = info.makeColorType(ct);
    return GrSurfaceFillContext::Make(context,
                                      info,
                                      fit,
                                      sampleCount,
                                      mipmapped,
                                      isProtected,
                                      origin,
                                      budgeted);
}

std::unique_ptr<GrSurfaceFillContext> GrSurfaceFillContext::MakeFromBackendTexture(
        GrRecordingContext* context,
        GrColorInfo info,
        const GrBackendTexture& tex,
        int sampleCount,
        GrSurfaceOrigin origin,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(sampleCount > 0);

    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return GrSurfaceDrawContext::MakeFromBackendTexture(context,
                                                            info.colorType(),
                                                            info.refColorSpace(),
                                                            tex,
                                                            sampleCount,
                                                            origin,
                                                            nullptr,
                                                            std::move(releaseHelper));
    }
    const GrBackendFormat& format = tex.getBackendFormat();
    GrSwizzle readSwizzle, writeSwizzle;
    if (info.colorType() != GrColorType::kUnknown) {
        if (!context->priv().caps()->areColorTypeAndFormatCompatible(info.colorType(), format)) {
            return nullptr;
        }
        readSwizzle  = context->priv().caps()->getReadSwizzle (format, info.colorType());
        writeSwizzle = context->priv().caps()->getWriteSwizzle(format, info.colorType());
    }

    sk_sp<GrTextureProxy> proxy(context->priv().proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCount, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    return std::make_unique<GrSurfaceFillContext>(context,
                                                  std::move(readView),
                                                  std::move(writeView),
                                                  std::move(info));

}

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// GrOpsTask to be picked up and added to by GrSurfaceFillContext lower in the call
// stack. When this occurs with a closed GrOpsTask, a new one will be allocated
// when the GrSurfaceFillContext attempts to use it (via getOpsTask).
GrSurfaceFillContext::GrSurfaceFillContext(GrRecordingContext* context,
                                           GrSurfaceProxyView readView,
                                           GrSurfaceProxyView writeView,
                                           const GrColorInfo& colorInfo,
                                           bool flushTimeOpsTask)
        : GrSurfaceContext(context, std::move(readView), std::move(colorInfo))
        , fWriteView(std::move(writeView))
        , fFlushTimeOpsTask(flushTimeOpsTask) {
    fOpsTask = sk_ref_sp(context->priv().drawingManager()->getLastOpsTask(this->asSurfaceProxy()));
    SkASSERT(this->asSurfaceProxy() == fWriteView.proxy());
    SkASSERT(this->origin() == fWriteView.origin());

    SkDEBUGCODE(this->validate();)
}

void GrSurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                          std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceFillContext", "fillRectWithFP", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto op = GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                          SkRect::Make(dstRect));
    this->addDrawOp(std::move(op));
}

void GrSurfaceFillContext::addDrawOp(GrOp::Owner owner) {
    GrDrawOp* op = static_cast<GrDrawOp*>(owner.get());
    GrClampType clampType = GrColorTypeClampType(this->colorInfo().colorType());
    auto clip = GrAppliedClip::Disabled();
    const GrCaps& caps = *this->caps();
    GrProcessorSet::Analysis analysis = op->finalize(caps,
                                                     &clip,
                                                     /*mixed sample coverage*/ false,
                                                     clampType);
    SkASSERT(!(op->fixedFunctionFlags() & GrDrawOp::FixedFunctionFlags::kUsesStencil));
    SkASSERT(!analysis.requiresDstTexture());
    SkRect bounds = owner->bounds();
    // We shouldn't have coverage AA or hairline draws in fill contexts.
    SkASSERT(!op->hasAABloat() && !op->hasZeroArea());
    if (!bounds.intersect(this->asSurfaceProxy()->getBoundsRect())) {
        return;
    }
    op->setClippedBounds(op->bounds());
    SkDEBUGCODE(op->fAddDrawOpCalled = true;)

    GrXferProcessor::DstProxyView dstProxyView;
    this->getOpsTask()->addDrawOp(fContext->priv().drawingManager(),
                                  std::move(owner),
                                  analysis,
                                  std::move(clip),
                                  dstProxyView,
                                  GrTextureResolveManager(this->drawingManager()),
                                  caps);
}

void GrSurfaceFillContext::ClearToGrPaint(std::array<float, 4> color, GrPaint* paint) {
    paint->setColor4f({color[0], color[1], color[2], color[3]});
    if (color[3] == 1.f) {
        // Can just rely on the src-over blend mode to do the right thing.
        // This may improve batching.
        paint->setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    } else {
        // A clear overwrites the prior color, so even if it's transparent, it behaves as if it
        // were src blended
        paint->setPorterDuffXPFactory(SkBlendMode::kSrc);
    }
}

void GrSurfaceFillContext::addOp(GrOp::Owner op) {
    GrDrawingManager* drawingMgr = this->drawingManager();
    this->getOpsTask()->addOp(drawingMgr,
                              std::move(op),
                              GrTextureResolveManager(drawingMgr),
                              *this->caps());
}

GrOpsTask* GrSurfaceFillContext::getOpsTask() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpsTask || fOpsTask->isClosed()) {
        sk_sp<GrOpsTask> newOpsTask = this->drawingManager()->newOpsTask(this->writeSurfaceView(),
                                                                         fFlushTimeOpsTask);
        if (fOpsTask) {
            this->willReplaceOpsTask(fOpsTask.get(), newOpsTask.get());
        }
        fOpsTask = std::move(newOpsTask);
    }
    SkASSERT(!fOpsTask->isClosed());
    return fOpsTask.get();
}

#ifdef SK_DEBUG
void GrSurfaceFillContext::onValidate() const {
    if (fOpsTask && !fOpsTask->isClosed()) {
        SkASSERT(this->drawingManager()->getLastRenderTask(fWriteView.proxy()) == fOpsTask.get());
    }
}
#endif

void GrSurfaceFillContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "discard", fContext);

    AutoCheckFlush acf(this->drawingManager());

    this->getOpsTask()->discard();
}

void GrSurfaceFillContext::internalClear(const SkIRect* scissor,
                                         std::array<float, 4> color,
                                         bool upgradePartialToFull) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "clear", fContext);

    // There are three ways clears are handled: load ops, native clears, and draws. Load ops are
    // only for fullscreen clears; native clears can be fullscreen or with scissors if the backend
    // supports then. Drawing an axis-aligned rect is the fallback path.
    GrScissorState scissorState(this->asSurfaceProxy()->backingStoreDimensions());
    if (scissor && !scissorState.set(*scissor)) {
        // The clear is offscreen, so skip it (normally this would be handled by addDrawOp,
        // except clear ops are not draw ops).
        return;
    }

    // If we have a scissor but it's okay to clear beyond it for performance reasons, then disable
    // the test. We only do this when the clear would be handled by a load op or natively.
    if (scissorState.enabled() && !this->caps()->performColorClearsAsDraws()) {
        if (upgradePartialToFull && (this->caps()->preferFullscreenClears() ||
                                     this->caps()->shouldInitializeTextures())) {
            // TODO: wrt the shouldInitializeTextures path, it would be more performant to
            // only clear the entire target if we knew it had not been cleared before. As
            // is this could end up doing a lot of redundant clears.
            scissorState.setDisabled();
        } else {
            // Unlike with stencil clears, we also allow clears up to the logical dimensions of the
            // render target to overflow into any approx-fit padding of the backing store dimensions
            scissorState.relaxTest(this->dimensions());
        }
    }

    if (!scissorState.enabled()) {
        // This is a fullscreen clear, so could be handled as a load op. Regardless, we can also
        // discard all prior ops in the current task since the color buffer will be overwritten.
        GrOpsTask* opsTask = this->getOpsTask();
        if (opsTask->resetForFullscreenClear(this->canDiscardPreviousOpsOnFullClear()) &&
            !this->caps()->performColorClearsAsDraws()) {
            color = this->writeSurfaceView().swizzle().applyTo(color);
            // The op list was emptied and native clears are allowed, so just use the load op
            opsTask->setColorLoadOp(GrLoadOp::kClear, color);
            return;
        } else {
            // Will use an op for the clear, reset the load op to discard since the op will
            // blow away the color buffer contents
            opsTask->setColorLoadOp(GrLoadOp::kDiscard);
        }
    }

    // At this point we are either a partial clear or a fullscreen clear that couldn't be applied
    // as a load op.
    bool clearAsDraw = this->caps()->performColorClearsAsDraws() ||
                       (scissorState.enabled() && this->caps()->performPartialClearsAsDraws());
    if (clearAsDraw) {
        GrPaint paint;
        ClearToGrPaint(color, &paint);
        auto op = GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                              SkRect::Make(scissorState.rect()));
        this->addDrawOp(std::move(op));
    } else {
        color = this->writeSurfaceView().swizzle().applyTo(color);
        this->addOp(GrClearOp::MakeColor(fContext, scissorState, color));
    }
}

bool GrSurfaceFillContext::blitTexture(GrSurfaceProxyView view,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    SkASSERT(view.asTextureProxy());
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    if (!GrClipSrcRectAndDstPoint(this->dimensions(),
                                  view.dimensions(),
                                  srcRect,
                                  dstPoint,
                                  &clippedSrcRect,
                                  &clippedDstPoint)) {
        return false;
    }

    auto fp = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
    auto dstRect = SkIRect::MakePtSize(clippedDstPoint, clippedSrcRect.size());
    auto srcRectF = SkRect::Make(clippedSrcRect);
    this->fillRectToRectWithFP(srcRectF, dstRect, std::move(fp));
    return true;
}
