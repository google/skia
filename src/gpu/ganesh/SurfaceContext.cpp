/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/SurfaceContext.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/ganesh/GrClientMappedBufferManager.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTracing.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrRect.h"

#include <memory>

using namespace skia_private;

#define ASSERT_SINGLE_OWNER         SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_FALSE_IF_ABANDONED   if (this->fContext->abandoned()) { return false;   }
#define RETURN_NULLPTR_IF_ABANDONED if (this->fContext->abandoned()) { return nullptr; }

namespace skgpu::ganesh {

SurfaceContext::SurfaceContext(GrRecordingContext* context,
                               GrSurfaceProxyView readView,
                               const GrColorInfo& info)
        : fContext(context), fReadView(std::move(readView)), fColorInfo(info) {
    SkASSERT(!context->abandoned());
}

const GrCaps* SurfaceContext::caps() const { return fContext->priv().caps(); }

GrDrawingManager* SurfaceContext::drawingManager() {
    return fContext->priv().drawingManager();
}

const GrDrawingManager* SurfaceContext::drawingManager() const {
    return fContext->priv().drawingManager();
}

#ifdef SK_DEBUG
skgpu::SingleOwner* SurfaceContext::singleOwner() const { return fContext->priv().singleOwner(); }
#endif

static bool alpha_types_compatible(SkAlphaType srcAlphaType, SkAlphaType dstAlphaType) {
    // If both alpha types are kUnknown things make sense. If not, it's too underspecified.
    return (srcAlphaType == kUnknown_SkAlphaType) == (dstAlphaType == kUnknown_SkAlphaType);
}

bool SurfaceContext::readPixels(GrDirectContext* dContext, GrPixmap dst, SkIPoint pt) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceContext", "readPixels", fContext);

    if (!fContext->priv().matches(dContext)) {
        return false;
    }

    if (dst.colorType() == GrColorType::kUnknown) {
        return false;
    }

    if (dst.rowBytes() % dst.info().bpp()) {
        return false;
    }

    dst = dst.clip(this->dimensions(), &pt);
    if (!dst.hasPixels()) {
        return false;
    }
    if (!alpha_types_compatible(this->colorInfo().alphaType(), dst.alphaType())) {
        return false;
    }
    // We allow unknown alpha types but only if both src and dst are unknown. Otherwise, it's too
    // weird to reason about what should be expected.

    sk_sp<GrSurfaceProxy> srcProxy = this->asSurfaceProxyRef();

    if (srcProxy->framebufferOnly()) {
        return false;
    }

    // MDB TODO: delay this instantiation until later in the method
    if (!srcProxy->instantiate(dContext->priv().resourceProvider())) {
        return false;
    }

    GrSurface* srcSurface = srcProxy->peekSurface();

    SkColorSpaceXformSteps::Flags flags =
            SkColorSpaceXformSteps{this->colorInfo(), dst.info()}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = dContext->priv().caps();
    bool srcIsCompressed = caps->isFormatCompressed(srcSurface->backendFormat());
    // This is the getImageData equivalent to the canvas2D putImageData fast path. We probably don't
    // care so much about getImageData performance. However, in order to ensure putImageData/
    // getImageData in "legacy" mode are round-trippable we use the GPU to do the complementary
    // unpremul step to writeSurfacePixels's premul step (which is determined empirically in
    // fContext->vaildaPMUPMConversionExists()).
    GrBackendFormat defaultRGBAFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                      GrRenderable::kYes);
    GrColorType srcColorType = this->colorInfo().colorType();
    bool canvas2DFastPath = unpremul && !needColorConversion &&
                            (GrColorType::kRGBA_8888 == dst.colorType() ||
                             GrColorType::kBGRA_8888 == dst.colorType()) &&
                            SkToBool(srcProxy->asTextureProxy()) &&
                            (srcColorType == GrColorType::kRGBA_8888 ||
                             srcColorType == GrColorType::kBGRA_8888) &&
                            defaultRGBAFormat.isValid() &&
                            dContext->priv().validPMUPMConversionExists();

    // Since the validPMUPMConversionExists function actually submits work to the gpu to do its
    // tests, it is possible that during that call we have abandoned the context. Thus, we do
    // another abandoned check here to make sure we are still valid.
    RETURN_FALSE_IF_ABANDONED

    auto readFlag = caps->surfaceSupportsReadPixels(srcSurface);
    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kUnsupported) {
        return false;
    }

    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kCopyToTexture2D || canvas2DFastPath) {
        std::unique_ptr<SurfaceContext> tempCtx;
        if (this->asTextureProxy()) {
            GrColorType colorType = (canvas2DFastPath || srcIsCompressed)
                                            ? GrColorType::kRGBA_8888
                                            : this->colorInfo().colorType();
            SkAlphaType alphaType = canvas2DFastPath ? dst.alphaType()
                                                     : this->colorInfo().alphaType();
            GrImageInfo tempInfo(colorType,
                                 alphaType,
                                 this->colorInfo().refColorSpace(),
                                 dst.dimensions());
            auto sfc = dContext->priv().makeSFC(
                    tempInfo, "SurfaceContext_ReadPixels", SkBackingFit::kApprox);
            if (!sfc) {
                return false;
            }

            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = dContext->priv().createPMToUPMEffect(GrTextureEffect::Make(
                        this->readSurfaceView(), this->colorInfo().alphaType()));
                if (dst.colorType() == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), skgpu::Swizzle::BGRA());
                    dst = GrPixmap(dst.info().makeColorType(GrColorType::kRGBA_8888),
                                   dst.addr(),
                                   dst.rowBytes());
                }
            } else {
                fp = GrTextureEffect::Make(this->readSurfaceView(), this->colorInfo().alphaType());
            }
            if (!fp) {
                return false;
            }
            sfc->fillRectToRectWithFP(SkIRect::MakePtSize(pt, dst.dimensions()),
                                      SkIRect::MakeSize(dst.dimensions()),
                                      std::move(fp));
            pt = {0, 0};
            tempCtx = std::move(sfc);
        } else {
            auto restrictions = this->caps()->getDstCopyRestrictions(this->asRenderTargetProxy(),
                                                                     this->colorInfo().colorType());
            sk_sp<GrSurfaceProxy> copy;
            static constexpr auto kFit = SkBackingFit::kExact;
            static constexpr auto kBudgeted = skgpu::Budgeted::kYes;
            static constexpr auto kMipMapped = GrMipmapped::kNo;
            if (restrictions.fMustCopyWholeSrc) {
                copy = GrSurfaceProxy::Copy(fContext,
                                            std::move(srcProxy),
                                            this->origin(),
                                            kMipMapped,
                                            kFit,
                                            kBudgeted,
                                            /*label=*/"SurfaceContext_ReadPixelsWithCopyWholeSrc");
            } else {
                auto srcRect = SkIRect::MakePtSize(pt, dst.dimensions());
                copy = GrSurfaceProxy::Copy(fContext,
                                            std::move(srcProxy),
                                            this->origin(),
                                            kMipMapped,
                                            srcRect,
                                            kFit,
                                            kBudgeted,
                                            /*label=*/"SurfaceContext_ReadPixels",
                                            restrictions.fRectsMustMatch);
                pt = {0, 0};
            }
            if (!copy) {
                return false;
            }
            GrSurfaceProxyView view{std::move(copy), this->origin(), this->readSwizzle()};
            tempCtx = dContext->priv().makeSC(std::move(view), this->colorInfo());
            SkASSERT(tempCtx);
        }
        return tempCtx->readPixels(dContext, dst, pt);
    }

    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;

    auto supportedRead = caps->supportedReadPixelsColorType(
            this->colorInfo().colorType(), srcProxy->backendFormat(), dst.colorType());

    bool makeTight =
            !caps->readPixelsRowBytesSupport() && dst.rowBytes() != dst.info().minRowBytes();

    bool convert = unpremul || premul || needColorConversion || flip || makeTight ||
                   (dst.colorType() != supportedRead.fColorType);

    std::unique_ptr<char[]> tmpPixels;
    GrPixmap tmp;
    void* readDst = dst.addr();
    size_t readRB = dst.rowBytes();
    if (convert) {
        GrImageInfo tmpInfo(supportedRead.fColorType,
                            this->colorInfo().alphaType(),
                            this->colorInfo().refColorSpace(),
                            dst.dimensions());
        size_t tmpRB = tmpInfo.minRowBytes();
        size_t size = tmpRB * tmpInfo.height();
        // Chrome MSAN bots require the data to be initialized (hence the ()).
        tmpPixels = std::make_unique<char[]>(size);
        tmp = {tmpInfo, tmpPixels.get(), tmpRB};

        readDst = tmpPixels.get();
        readRB = tmpRB;
        pt.fY = flip ? srcSurface->height() - pt.fY - dst.height() : pt.fY;
    }

    dContext->priv().flushSurface(srcProxy.get());
    dContext->submit();
    if (!dContext->priv().getGpu()->readPixels(srcSurface,
                                               SkIRect::MakePtSize(pt, dst.dimensions()),
                                               this->colorInfo().colorType(),
                                               supportedRead.fColorType,
                                               readDst,
                                               readRB)) {
        return false;
    }

    if (tmp.hasPixels()) {
        return GrConvertPixels(dst, tmp, flip);
    }
    return true;
}

bool SurfaceContext::writePixels(GrDirectContext* dContext,
                                 GrCPixmap src,
                                 SkIPoint dstPt) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)

    src = src.clip(this->dimensions(), &dstPt);
    if (!src.hasPixels()) {
        return false;
    }
    if (!src.info().bpp() || src.rowBytes() % src.info().bpp()) {
        return false;
    }
    return this->internalWritePixels(dContext, &src, 1, dstPt);
}

bool SurfaceContext::writePixels(GrDirectContext* dContext,
                                 const GrCPixmap src[],
                                 int numLevels) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)

    SkASSERT(dContext);
    SkASSERT(numLevels >= 1);
    SkASSERT(src);

    if (numLevels == 1) {
        if (src->dimensions() != this->dimensions()) {
            return false;
        }
        return this->writePixels(dContext, src[0], {0, 0});
    }
    if (!this->asTextureProxy() || this->asTextureProxy()->proxyMipmapped() == GrMipmapped::kNo) {
        return false;
    }

    SkISize dims = this->dimensions();
    if (numLevels != SkMipmap::ComputeLevelCount(dims) + 1) {
        return false;
    }
    for (int i = 0; i < numLevels; ++i) {
        if (src[i].colorInfo() != src[0].colorInfo()) {
            return false;
        }
        if (dims != src[i].dimensions()) {
            return false;
        }
        if (!src[i].info().bpp() || src[i].rowBytes() % src[i].info().bpp()) {
            return false;
        }
        dims = {std::max(1, dims.width()/2), std::max(1, dims.height()/2)};
    }
    return this->internalWritePixels(dContext, src, numLevels, {0, 0});
}

bool SurfaceContext::internalWritePixels(GrDirectContext* dContext,
                                         const GrCPixmap src[],
                                         int numLevels,
                                         SkIPoint pt) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceContext", "internalWritePixels", fContext);

    SkASSERT(numLevels >= 1);
    SkASSERT(src);

    // We can either write to a subset or write MIP levels, but not both.
    SkASSERT((src[0].dimensions() == this->dimensions() && pt.isZero()) || numLevels == 1);
    SkASSERT(numLevels == 1 ||
             (this->asTextureProxy() && this->asTextureProxy()->mipmapped() == GrMipmapped::kYes));
    // Our public caller should have clipped to the bounds of the surface already.
    SkASSERT(SkIRect::MakeSize(this->dimensions()).contains(
            SkIRect::MakePtSize(pt, src[0].dimensions())));

    if (!dContext) {
        return false;
    }

    if (this->asSurfaceProxy()->readOnly()) {
        return false;
    }

    if (src[0].colorType() == GrColorType::kUnknown) {
        return false;
    }

    if (!alpha_types_compatible(src[0].alphaType(), this->colorInfo().alphaType())) {
        return false;
    }

    GrSurfaceProxy* dstProxy = this->asSurfaceProxy();

    if (dstProxy->framebufferOnly()) {
        return false;
    }

    if (!dstProxy->instantiate(dContext->priv().resourceProvider())) {
        return false;
    }

    GrSurface* dstSurface = dstProxy->peekSurface();

    SkColorSpaceXformSteps::Flags flags =
            SkColorSpaceXformSteps{src[0].colorInfo(), this->colorInfo()}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = dContext->priv().caps();

    auto rgbaDefaultFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    GrColorType dstColorType = this->colorInfo().colorType();
    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath = !caps->avoidWritePixelsFastPath() && premul && !needColorConversion &&
                            (src[0].colorType() == GrColorType::kRGBA_8888 ||
                             src[0].colorType() == GrColorType::kBGRA_8888) &&
                            this->asFillContext() &&
                            (dstColorType == GrColorType::kRGBA_8888 ||
                             dstColorType == GrColorType::kBGRA_8888) &&
                            rgbaDefaultFormat.isValid() &&
                            dContext->priv().validPMUPMConversionExists();

    // Since the validPMUPMConversionExists function actually submits work to the gpu to do its
    // tests, it is possible that during that call we have abanoned the context. Thus we do an
    // abanoned check here to make sure we are still valid.
    RETURN_FALSE_IF_ABANDONED

    // Drawing code path doesn't support writing to levels and doesn't support inserting layout
    // transitions.
    if ((!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) && numLevels == 1) {
        GrColorInfo tempColorInfo;
        GrBackendFormat format;
        skgpu::Swizzle tempReadSwizzle;
        if (canvas2DFastPath) {
            tempColorInfo = {GrColorType::kRGBA_8888,
                             kUnpremul_SkAlphaType,
                             this->colorInfo().refColorSpace()};
            format = rgbaDefaultFormat;
        } else {
            tempColorInfo = this->colorInfo();
            format = dstProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
            tempReadSwizzle = this->readSwizzle();
        }

        // It is more efficient for us to write pixels into a top left origin so we prefer that.
        // However, if the final proxy isn't a render target then we must use a copy to move the
        // data into it which requires the origins to match. If the final proxy is a render target
        // we can use a draw instead which doesn't have this origin restriction. Thus for render
        // targets we will use top left and otherwise we will make the origins match.
        GrSurfaceOrigin tempOrigin =
                this->asFillContext() ? kTopLeft_GrSurfaceOrigin : this->origin();
        auto tempProxy = dContext->priv().proxyProvider()->createProxy(
                format,
                src[0].dimensions(),
                GrRenderable::kNo,
                1,
                GrMipmapped::kNo,
                SkBackingFit::kApprox,
                skgpu::Budgeted::kYes,
                GrProtected::kNo,
                /*label=*/"SurfaceContext_InternalWritePixels");
        if (!tempProxy) {
            return false;
        }
        GrSurfaceProxyView tempView(tempProxy, tempOrigin, tempReadSwizzle);
        SurfaceContext tempCtx(dContext, tempView, tempColorInfo);

        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        GrCPixmap origSrcBase = src[0];
        GrCPixmap srcBase = origSrcBase;
        if (canvas2DFastPath) {
            srcBase = GrCPixmap(origSrcBase.info().makeColorType(GrColorType::kRGBA_8888),
                                origSrcBase.addr(),
                                origSrcBase.rowBytes());
        }
        if (!tempCtx.writePixels(dContext, srcBase, {0, 0})) {
            return false;
        }

        if (this->asFillContext()) {
            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = dContext->priv().createUPMToPMEffect(
                        GrTextureEffect::Make(std::move(tempView), tempColorInfo.alphaType()));
                // Important: check the original src color type here!
                if (origSrcBase.colorType() == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), skgpu::Swizzle::BGRA());
                }
            } else {
                fp = GrTextureEffect::Make(std::move(tempView), tempColorInfo.alphaType());
            }
            if (!fp) {
                return false;
            }
            this->asFillContext()->fillRectToRectWithFP(
                    SkIRect::MakeSize(srcBase.dimensions()),
                    SkIRect::MakePtSize(pt, srcBase.dimensions()),
                    std::move(fp));
        } else {
            SkIRect srcRect = SkIRect::MakeSize(srcBase.dimensions());
            SkIPoint dstPoint = SkIPoint::Make(pt.fX, pt.fY);
            if (!this->copy(std::move(tempProxy), srcRect, dstPoint)) {
                return false;
            }
        }
        return true;
    }

    GrColorType srcColorType = src[0].colorType();
    auto [allowedColorType, _] =
            caps->supportedWritePixelsColorType(this->colorInfo().colorType(),
                                                dstProxy->backendFormat(),
                                                srcColorType);
    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;

    bool convertAll = premul              ||
                      unpremul            ||
                      needColorConversion ||
                      flip                ||
                      (srcColorType != allowedColorType);
    bool mustBeTight = !caps->writePixelsRowBytesSupport();
    size_t tmpSize = 0;
    if (mustBeTight || convertAll) {
        for (int i = 0; i < numLevels; ++i) {
            if (convertAll || (mustBeTight && src[i].rowBytes() != src[i].info().minRowBytes())) {
                tmpSize += src[i].info().makeColorType(allowedColorType).minRowBytes()*
                           src[i].height();
            }
        }
    }

    auto tmpData = tmpSize ? SkData::MakeUninitialized(tmpSize) : nullptr;
    void*    tmp = tmpSize ? tmpData->writable_data()           : nullptr;
    AutoSTArray<15, GrMipLevel> srcLevels(numLevels);
    bool ownAllStorage = true;
    for (int i = 0; i < numLevels; ++i) {
        if (convertAll || (mustBeTight && src[i].rowBytes() != src[i].info().minRowBytes())) {
            GrImageInfo tmpInfo(allowedColorType,
                                this->colorInfo().alphaType(),
                                this->colorInfo().refColorSpace(),
                                src[i].dimensions());
            auto tmpRB = tmpInfo.minRowBytes();
            GrPixmap tmpPM(tmpInfo, tmp, tmpRB);
            SkAssertResult(GrConvertPixels(tmpPM, src[i], flip));
            srcLevels[i] = {tmpPM.addr(), tmpPM.rowBytes(), tmpData};
            tmp = SkTAddOffset<void>(tmp, tmpRB*tmpPM.height());
        } else {
            srcLevels[i] = {src[i].addr(), src[i].rowBytes(), src[i].pixelStorage()};
            ownAllStorage &= src[i].ownsPixels();
        }
    }
    pt.fY = flip ? dstSurface->height() - pt.fY - src[0].height() : pt.fY;

    if (!dContext->priv().drawingManager()->newWritePixelsTask(
                sk_ref_sp(dstProxy),
                SkIRect::MakePtSize(pt, src[0].dimensions()),
                allowedColorType,
                this->colorInfo().colorType(),
                srcLevels.begin(),
                numLevels)) {
        return false;
    }
    if (numLevels > 1) {
        dstProxy->asTextureProxy()->markMipmapsClean();
    }
    if (!ownAllStorage) {
        // If any pixmap doesn't own its pixels then we must flush so that the pixels are pushed to
        // the GPU before we return.
        dContext->priv().flushSurface(dstProxy);
    }
    return true;
}

void SurfaceContext::asyncRescaleAndReadPixels(GrDirectContext* dContext,
                                               const SkImageInfo& info,
                                               const SkIRect& srcRect,
                                               RescaleGamma rescaleGamma,
                                               RescaleMode rescaleMode,
                                               ReadPixelsCallback callback,
                                               ReadPixelsContext callbackContext) {
    if (!dContext) {
        callback(callbackContext, nullptr);
        return;
    }
    auto rt = this->asRenderTargetProxy();
    if (rt && rt->wrapsVkSecondaryCB()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (rt && rt->framebufferOnly()) {
        callback(callbackContext, nullptr);
        return;
    }
    auto dstCT = SkColorTypeToGrColorType(info.colorType());
    if (dstCT == GrColorType::kUnknown) {
        callback(callbackContext, nullptr);
        return;
    }
    bool needsRescale = srcRect.size() != info.dimensions()               ||
                        this->origin() == kBottomLeft_GrSurfaceOrigin     ||
                        this->colorInfo().alphaType() != info.alphaType() ||
                        !SkColorSpace::Equals(this->colorInfo().colorSpace(), info.colorSpace());
    auto surfaceBackendFormat = this->asSurfaceProxy()->backendFormat();
    auto readInfo = this->caps()->supportedReadPixelsColorType(this->colorInfo().colorType(),
                                                               surfaceBackendFormat,
                                                               dstCT);
    // Fail if we can't read from the source surface's color type.
    if (readInfo.fColorType == GrColorType::kUnknown) {
        callback(callbackContext, nullptr);
        return;
    }
    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = GrColorTypeChannelFlags(dstCT);
    uint32_t legalReadChannels = GrColorTypeChannelFlags(readInfo.fColorType);
    uint32_t srcChannels = GrColorTypeChannelFlags(this->colorInfo().colorType());
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        callback(callbackContext, nullptr);
        return;
    }

    std::unique_ptr<SurfaceFillContext> tempFC;
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    if (needsRescale) {
        auto tempInfo = GrImageInfo(info).makeColorType(this->colorInfo().colorType());
        tempFC = this->rescale(tempInfo, kTopLeft_GrSurfaceOrigin, srcRect,
                               rescaleGamma, rescaleMode);
        if (!tempFC) {
            callback(callbackContext, nullptr);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempFC->colorInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempFC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
    }
    auto srcCtx = tempFC ? tempFC.get() : this;
    return srcCtx->asyncReadPixels(dContext,
                                   SkIRect::MakePtSize({x, y}, info.dimensions()),
                                   info.colorType(),
                                   callback,
                                   callbackContext);
}

void SurfaceContext::asyncReadPixels(GrDirectContext* dContext,
                                     const SkIRect& rect,
                                     SkColorType colorType,
                                     ReadPixelsCallback callback,
                                     ReadPixelsContext callbackContext) {
    using AsyncReadResult = skgpu::TAsyncReadResult<GrGpuBuffer, GrDirectContext::DirectContextID,
                                                    PixelTransferResult>;

    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());

    if (!dContext || this->asSurfaceProxy()->isProtected() == GrProtected::kYes) {
        callback(callbackContext, nullptr);
        return;
    }

    auto mappedBufferManager = dContext->priv().clientMappedBufferManager();

    auto transferResult = this->transferPixels(SkColorTypeToGrColorType(colorType), rect);

    if (!transferResult.fTransferBuffer) {
        auto ii = SkImageInfo::Make(rect.size(), colorType, this->colorInfo().alphaType(),
                                    this->colorInfo().refColorSpace());
        static const GrDirectContext::DirectContextID kInvalid;
        auto result = std::make_unique<AsyncReadResult>(kInvalid);
        GrPixmap pm = GrPixmap::Allocate(ii);
        result->addCpuPlane(pm.pixelStorage(), pm.rowBytes());

        SkIPoint pt{rect.fLeft, rect.fTop};
        if (!this->readPixels(dContext, pm, pt)) {
            callback(callbackContext, nullptr);
            return;
        }
        callback(callbackContext, std::move(result));
        return;
    }

    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        SkISize fSize;
        GrClientMappedBufferManager* fMappedBufferManager;
        PixelTransferResult fTransferResult;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            rect.size(),
                                            mappedBufferManager,
                                            std::move(transferResult)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        auto manager = context->fMappedBufferManager;
        auto result = std::make_unique<AsyncReadResult>(manager->ownerID());
        if (!result->addTransferResult(context->fTransferResult,
                                       context->fSize,
                                       context->fTransferResult.fRowBytes,
                                       manager)) {
            result.reset();
        }
        (*context->fClientCallback)(context->fClientContext, std::move(result));
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;

    dContext->priv().flushSurface(
            this->asSurfaceProxy(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
}

void SurfaceContext::asyncRescaleAndReadPixelsYUV420(GrDirectContext* dContext,
                                                     SkYUVColorSpace yuvColorSpace,
                                                     bool readAlpha,
                                                     sk_sp<SkColorSpace> dstColorSpace,
                                                     const SkIRect& srcRect,
                                                     SkISize dstSize,
                                                     RescaleGamma rescaleGamma,
                                                     RescaleMode rescaleMode,
                                                     ReadPixelsCallback callback,
                                                     ReadPixelsContext callbackContext) {
    using AsyncReadResult = skgpu::TAsyncReadResult<GrGpuBuffer, GrDirectContext::DirectContextID,
                                                    PixelTransferResult>;

    SkASSERT(srcRect.fLeft >= 0 && srcRect.fRight <= this->width());
    SkASSERT(srcRect.fTop >= 0 && srcRect.fBottom <= this->height());
    SkASSERT(!dstSize.isZero());
    SkASSERT((dstSize.width() % 2 == 0) && (dstSize.height() % 2 == 0));

    if (!dContext) {
        callback(callbackContext, nullptr);
        return;
    }
    auto rt = this->asRenderTargetProxy();
    if (rt && rt->wrapsVkSecondaryCB()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (rt && rt->framebufferOnly()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (this->asSurfaceProxy()->isProtected() == GrProtected::kYes) {
        callback(callbackContext, nullptr);
        return;
    }
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    bool needsRescale = srcRect.size() != dstSize ||
                        !SkColorSpace::Equals(this->colorInfo().colorSpace(), dstColorSpace.get());
    GrSurfaceProxyView srcView = this->readSurfaceView();
    if (needsRescale) {
        auto info = SkImageInfo::Make(dstSize,
                                      kRGBA_8888_SkColorType,
                                      this->colorInfo().alphaType(),
                                      dstColorSpace);
        // TODO: Incorporate the YUV conversion into last pass of rescaling.
        auto tempFC = this->rescale(info,
                                    kTopLeft_GrSurfaceOrigin,
                                    srcRect,
                                    rescaleGamma,
                                    rescaleMode);
        if (!tempFC) {
            callback(callbackContext, nullptr);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempFC->colorInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempFC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
        srcView = tempFC->readSurfaceView();
    } else if (!srcView.asTextureProxy()) {
        srcView = GrSurfaceProxyView::Copy(
                fContext,
                std::move(srcView),
                GrMipmapped::kNo,
                srcRect,
                SkBackingFit::kApprox,
                skgpu::Budgeted::kYes,
                /*label=*/"SurfaceContext_AsyncRescaleAndReadPixelsYUV420");
        if (!srcView) {
            // If we can't get a texture copy of the contents then give up.
            callback(callbackContext, nullptr);
            return;
        }
        SkASSERT(srcView.asTextureProxy());
        x = y = 0;
    }

    auto yaInfo = SkImageInfo::MakeA8(dstSize);
    auto yFC = dContext->priv().makeSFCWithFallback(yaInfo, SkBackingFit::kApprox,
                                                    /* sampleCount= */ 1,
                                                    skgpu::Mipmapped::kNo, skgpu::Protected::kNo);
    std::unique_ptr<SurfaceFillContext> aFC;
    if (readAlpha) {
        aFC = dContext->priv().makeSFCWithFallback(yaInfo, SkBackingFit::kApprox,
                                                   /* sampleCount= */ 1,
                                                   skgpu::Mipmapped::kNo, skgpu::Protected::kNo);
    }

    auto uvInfo = yaInfo.makeWH(yaInfo.width()/2, yaInfo.height()/2);
    auto uFC = dContext->priv().makeSFCWithFallback(uvInfo, SkBackingFit::kApprox,
                                                    /* sampleCount= */ 1,
                                                    skgpu::Mipmapped::kNo, skgpu::Protected::kNo);
    auto vFC = dContext->priv().makeSFCWithFallback(uvInfo, SkBackingFit::kApprox,
                                                    /* sampleCount= */ 1,
                                                    skgpu::Mipmapped::kNo, skgpu::Protected::kNo);

    if (!yFC || !uFC || !vFC || (readAlpha && !aFC)) {
        callback(callbackContext, nullptr);
        return;
    }

    float baseM[20];
    SkColorMatrix_RGB2YUV(yuvColorSpace, baseM);

    // TODO: Use one transfer buffer for all three planes to reduce map/unmap cost?

    auto texMatrix = SkMatrix::Translate(x, y);

    auto [readCT, offsetAlignment] =
            this->caps()->supportedReadPixelsColorType(yFC->colorInfo().colorType(),
                                                       yFC->asSurfaceProxy()->backendFormat(),
                                                       GrColorType::kAlpha_8);
    if (readCT == GrColorType::kUnknown) {
        callback(callbackContext, nullptr);
        return;
    }
    bool doSynchronousRead = !this->caps()->transferFromSurfaceToBufferSupport() ||
                             !offsetAlignment;
    PixelTransferResult yTransfer, aTransfer, uTransfer, vTransfer;

    // This matrix generates (r,g,b,a) = (0, 0, 0, y)
    float yM[20];
    std::fill_n(yM, 15, 0.f);
    std::copy_n(baseM + 0, 5, yM + 15);

    auto yFP = GrTextureEffect::Make(srcView, this->colorInfo().alphaType(), texMatrix);
    yFP = GrFragmentProcessor::ColorMatrix(std::move(yFP),
                                           yM,
                                           /*unpremulInput=*/false,
                                           /*clampRGBOutput=*/true,
                                           /*premulOutput=*/false);
    yFC->fillWithFP(std::move(yFP));
    if (!doSynchronousRead) {
        yTransfer = yFC->transferPixels(GrColorType::kAlpha_8,
                                        SkIRect::MakeSize(yFC->dimensions()));
        if (!yTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    if (readAlpha) {
        auto aFP = GrTextureEffect::Make(srcView, this->colorInfo().alphaType(), texMatrix);
        SkASSERT(baseM[15] == 0 &&
                 baseM[16] == 0 &&
                 baseM[17] == 0 &&
                 baseM[18] == 1 &&
                 baseM[19] == 0);
        aFC->fillWithFP(std::move(aFP));
        if (!doSynchronousRead) {
            aTransfer = aFC->transferPixels(GrColorType::kAlpha_8,
                                            SkIRect::MakeSize(aFC->dimensions()));
            if (!aTransfer.fTransferBuffer) {
                callback(callbackContext, nullptr);
                return;
            }
        }
    }

    texMatrix.preScale(2.f, 2.f);
    // This matrix generates (r,g,b,a) = (0, 0, 0, u)
    float uM[20];
    std::fill_n(uM, 15, 0.f);
    std::copy_n(baseM + 5, 5, uM + 15);

    auto uFP = GrTextureEffect::Make(srcView,
                                     this->colorInfo().alphaType(),
                                     texMatrix,
                                     GrSamplerState::Filter::kLinear);
    uFP = GrFragmentProcessor::ColorMatrix(std::move(uFP),
                                           uM,
                                           /*unpremulInput=*/false,
                                           /*clampRGBOutput=*/true,
                                           /*premulOutput=*/false);
    uFC->fillWithFP(std::move(uFP));
    if (!doSynchronousRead) {
        uTransfer = uFC->transferPixels(GrColorType::kAlpha_8,
                                        SkIRect::MakeSize(uFC->dimensions()));
        if (!uTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    // This matrix generates (r,g,b,a) = (0, 0, 0, v)
    float vM[20];
    std::fill_n(vM, 15, 0.f);
    std::copy_n(baseM + 10, 5, vM + 15);
    auto vFP = GrTextureEffect::Make(std::move(srcView),
                                     this->colorInfo().alphaType(),
                                     texMatrix,
                                     GrSamplerState::Filter::kLinear);
    vFP = GrFragmentProcessor::ColorMatrix(std::move(vFP),
                                           vM,
                                           /*unpremulInput=*/false,
                                           /*clampRGBOutput=*/true,
                                           /*premulOutput=*/false);
    vFC->fillWithFP(std::move(vFP));

    if (!doSynchronousRead) {
        vTransfer = vFC->transferPixels(GrColorType::kAlpha_8,
                                         SkIRect::MakeSize(vFC->dimensions()));
        if (!vTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    if (doSynchronousRead) {
        GrPixmap yPmp = GrPixmap::Allocate(yaInfo);
        GrPixmap uPmp = GrPixmap::Allocate(uvInfo);
        GrPixmap vPmp = GrPixmap::Allocate(uvInfo);
        GrPixmap aPmp;
        if (readAlpha) {
            aPmp = GrPixmap::Allocate(yaInfo);
        }
        if (!yFC->readPixels(dContext, yPmp, {0, 0}) ||
            !uFC->readPixels(dContext, uPmp, {0, 0}) ||
            !vFC->readPixels(dContext, vPmp, {0, 0}) ||
            (readAlpha && !aFC->readPixels(dContext, aPmp, {0, 0}))) {
            callback(callbackContext, nullptr);
            return;
        }
        auto result = std::make_unique<AsyncReadResult>(dContext->directContextID());
        result->addCpuPlane(yPmp.pixelStorage(), yPmp.rowBytes());
        result->addCpuPlane(uPmp.pixelStorage(), uPmp.rowBytes());
        result->addCpuPlane(vPmp.pixelStorage(), vPmp.rowBytes());
        if (readAlpha) {
            result->addCpuPlane(aPmp.pixelStorage(), aPmp.rowBytes());
        }
        callback(callbackContext, std::move(result));
        return;
    }

    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        GrClientMappedBufferManager* fMappedBufferManager;
        SkISize fSize;
        PixelTransferResult fYTransfer;
        PixelTransferResult fUTransfer;
        PixelTransferResult fVTransfer;
        PixelTransferResult fATransfer;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            dContext->priv().clientMappedBufferManager(),
                                            dstSize,
                                            std::move(yTransfer),
                                            std::move(uTransfer),
                                            std::move(vTransfer),
                                            std::move(aTransfer)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        auto manager = context->fMappedBufferManager;
        auto result = std::make_unique<AsyncReadResult>(manager->ownerID());
        if (!result->addTransferResult(context->fYTransfer,
                                       context->fSize,
                                       context->fYTransfer.fRowBytes,
                                       manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        SkISize uvSize = {context->fSize.width() / 2, context->fSize.height() / 2};
        if (!result->addTransferResult(context->fUTransfer,
                                       uvSize,
                                       context->fUTransfer.fRowBytes,
                                       manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        if (!result->addTransferResult(context->fVTransfer,
                                       uvSize,
                                       context->fVTransfer.fRowBytes,
                                       manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        if (context->fATransfer.fTransferBuffer &&
            !result->addTransferResult(context->fATransfer,
                                       context->fSize,
                                       context->fATransfer.fRowBytes,
                                       manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        (*context->fClientCallback)(context->fClientContext, std::move(result));
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;
    dContext->priv().flushSurface(
            this->asSurfaceProxy(), SkSurfaces::BackendSurfaceAccess::kNoAccess, flushInfo);
}

sk_sp<GrRenderTask> SurfaceContext::copy(sk_sp<GrSurfaceProxy> src,
                                         SkIRect srcRect,
                                         SkIPoint dstPoint) {
    if (!GrClipSrcRectAndDstPoint(this->dimensions(), &dstPoint,
                                  src->dimensions(), &srcRect)) {
        return nullptr;
    }

    SkIRect dstRect = SkIRect::MakePtSize(dstPoint, srcRect.size());
    return this->copyScaled(src, srcRect, dstRect, GrSamplerState::Filter::kNearest);
}

sk_sp<GrRenderTask> SurfaceContext::copyScaled(sk_sp<GrSurfaceProxy> src,
                                               SkIRect srcRect,
                                               SkIRect dstRect,
                                               GrSamplerState::Filter filter) {
    ASSERT_SINGLE_OWNER
    RETURN_NULLPTR_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceContext", "copyScaled", fContext);

    const GrCaps* caps = fContext->priv().caps();

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return nullptr;
    }

    // canCopySurface() verifies that src and dst rects are contained in their surfaces.
    if (!caps->canCopySurface(this->asSurfaceProxy(), dstRect, src.get(), srcRect)) {
        return nullptr;
    }

    if (filter == GrSamplerState::Filter::kLinear && !src->isFunctionallyExact()) {
        // If we're linear filtering an image that is approx-sized, there are cases where the filter
        // could sample outside the logical dimensions. Specifically if we're upscaling along an
        // axis where we are copying up to the logical dimension, but that dimension is less than
        // the actual backing store dimension, the linear filter will access one texel beyond the
        // logical size, potentially incorporating undefined values.
        const bool upscalingXAtApproxEdge =
            dstRect.width() > srcRect.width() &&
            srcRect.fRight == src->width() &&
            src->width() < src->backingStoreDimensions().width();
        const bool upscalingYAtApproxEdge =
            dstRect.height() > srcRect.height() &&
            srcRect.height() == src->height() &&
            srcRect.height() < src->backingStoreDimensions().height();
        if (upscalingXAtApproxEdge || upscalingYAtApproxEdge) {
            return nullptr;
        }

        // NOTE: Any upscaling with the linear filter will include content that's 1px outside the
        // src rect, but as long as that's still within the logical dimensions we assume it's okay.
    }

    SkASSERT(src->backendFormat().textureType() != GrTextureType::kExternal);
    SkASSERT(src->backendFormat() == this->asSurfaceProxy()->backendFormat());
    return this->drawingManager()->newCopyRenderTask(this->asSurfaceProxyRef(),
                                                     dstRect,
                                                     std::move(src),
                                                     srcRect,
                                                     filter,
                                                     this->origin());
}

std::unique_ptr<SurfaceFillContext> SurfaceContext::rescale(const GrImageInfo& info,
                                                            GrSurfaceOrigin origin,
                                                            SkIRect srcRect,
                                                            RescaleGamma rescaleGamma,
                                                            RescaleMode rescaleMode) {
    auto sfc = fContext->priv().makeSFCWithFallback(info,
                                                    SkBackingFit::kExact,
                                                    /* sampleCount= */ 1,
                                                    GrMipmapped::kNo,
                                                    this->asSurfaceProxy()->isProtected(),
                                                    origin);
    if (!sfc || !this->rescaleInto(sfc.get(),
                                   SkIRect::MakeSize(sfc->dimensions()),
                                   srcRect,
                                   rescaleGamma,
                                   rescaleMode)) {
        return nullptr;
    }
    return sfc;
}

bool SurfaceContext::rescaleInto(SurfaceFillContext* dst,
                                 SkIRect dstRect,
                                 SkIRect srcRect,
                                 RescaleGamma rescaleGamma,
                                 RescaleMode rescaleMode) {
    SkASSERT(dst);
    if (!SkIRect::MakeSize(dst->dimensions()).contains((dstRect))) {
        return false;
    }

    auto rtProxy = this->asRenderTargetProxy();
    if (rtProxy && rtProxy->wrapsVkSecondaryCB()) {
        return false;
    }

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return false;
    }

    GrSurfaceProxyView texView = this->readSurfaceView();
    // If we perform scaling as draws, texView must be texturable; if it's not already, we have to
    // make a copy. However, if the scaling can use copyScaled(), we can avoid this copy.
    auto ensureTexturable = [this](GrSurfaceProxyView texView, SkIRect srcRect) {
        if (!texView.asTextureProxy()) {
            // TODO: If copying supported specifying a renderable copy then we could return the copy
            // when there are no other conversions.
            texView = GrSurfaceProxyView::Copy(fContext,
                                               std::move(texView),
                                               GrMipmapped::kNo,
                                               srcRect,
                                               SkBackingFit::kApprox,
                                               skgpu::Budgeted::kNo,
                                               "SurfaceContext_RescaleInto");
            if (texView) {
                SkASSERT(texView.asTextureProxy());
                srcRect = SkIRect::MakeSize(srcRect.size());
            }
        }
        return std::make_pair(std::move(texView), srcRect);
    };

    SkISize finalSize = dstRect.size();
    if (finalSize == srcRect.size()) {
        rescaleGamma = RescaleGamma::kSrc;
        rescaleMode = RescaleMode::kNearest;
    }

    // Within a rescaling pass A is the input (if not null) and B is the output. At the end of the
    // pass B is moved to A. If 'this' is the input on the first pass then tempA is null.
    std::unique_ptr<SurfaceFillContext> tempA;
    std::unique_ptr<SurfaceFillContext> tempB;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == RescaleGamma::kLinear && this->colorInfo().colorSpace() &&
        !this->colorInfo().colorSpace()->gammaIsLinear()) {
        // Colorspace transformations are always handled by drawing so we need to be texturable
        std::tie(texView, srcRect) = ensureTexturable(texView, srcRect);
        if (!texView) {
            return false;
        }
        auto cs = this->colorInfo().colorSpace()->makeLinearGamma();
        // We'll fall back to kRGBA_8888 if half float not supported.
        GrImageInfo ii(GrColorType::kRGBA_F16,
                       dst->colorInfo().alphaType(),
                       std::move(cs),
                       srcRect.size());
        auto linearRTC = fContext->priv().makeSFCWithFallback(std::move(ii),
                                                              SkBackingFit::kApprox,
                                                              /* sampleCount= */ 1,
                                                              GrMipmapped::kNo,
                                                              texView.proxy()->isProtected(),
                                                              dst->origin());
        if (!linearRTC) {
            return false;
        }
        auto fp = GrTextureEffect::Make(std::move(texView),
                                        this->colorInfo().alphaType(),
                                        SkMatrix::Translate(srcRect.topLeft()),
                                        GrSamplerState::Filter::kNearest,
                                        GrSamplerState::MipmapMode::kNone);
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           this->colorInfo(),
                                           linearRTC->colorInfo());
        linearRTC->fillWithFP(std::move(fp));
        texView = linearRTC->readSurfaceView();
        SkASSERT(texView.asTextureProxy());
        tempA = std::move(linearRTC);
        srcRect = SkIRect::MakeSize(srcRect.size());
    }

    do {
        SkISize nextDims = finalSize;
        if (rescaleMode != RescaleMode::kNearest && rescaleMode != RescaleMode::kLinear) {
            if (srcRect.width() > finalSize.width()) {
                nextDims.fWidth = std::max((srcRect.width() + 1)/2, finalSize.width());
            } else if (srcRect.width() < finalSize.width()) {
                nextDims.fWidth = std::min(srcRect.width()*2, finalSize.width());
            }
            if (srcRect.height() > finalSize.height()) {
                nextDims.fHeight = std::max((srcRect.height() + 1)/2, finalSize.height());
            } else if (srcRect.height() < finalSize.height()) {
                nextDims.fHeight = std::min(srcRect.height()*2, finalSize.height());
            }
        }
        auto input = tempA ? tempA.get() : this;
        sk_sp<GrColorSpaceXform> xform;
        SurfaceFillContext* stepDst;
        SkIRect stepDstRect;
        if (nextDims == finalSize) {
            stepDst = dst;
            stepDstRect = dstRect;
            xform = GrColorSpaceXform::Make(input->colorInfo(), dst->colorInfo());
        } else {
            GrImageInfo nextInfo(input->colorInfo(), nextDims);

            tempB = fContext->priv().makeSFCWithFallback(nextInfo, SkBackingFit::kApprox,
                                                         /* sampleCount= */ 1,
                                                         skgpu::Mipmapped::kNo,
                                                         texView.proxy()->isProtected());
            if (!tempB) {
                return false;
            }
            stepDst = tempB.get();
            stepDstRect = SkIRect::MakeSize(tempB->dimensions());
        }
        std::unique_ptr<GrFragmentProcessor> fp;
        if (rescaleMode == RescaleMode::kRepeatedCubic) {
            // Cubic sampling is always handled by drawing with a shader, so we must be texturable
            std::tie(texView, srcRect) = ensureTexturable(texView, srcRect);
            if (!texView) {
                return false;
            }
            auto dir = GrBicubicEffect::Direction::kXY;
            if (nextDims.width() == srcRect.width()) {
                dir = GrBicubicEffect::Direction::kY;
            } else if (nextDims.height() == srcRect.height()) {
                dir = GrBicubicEffect::Direction::kX;
            }
            static constexpr auto kWM     = GrSamplerState::WrapMode::kClamp;
            static constexpr auto kKernel = GrBicubicEffect::gCatmullRom;
            fp = GrBicubicEffect::MakeSubset(std::move(texView),
                                             input->colorInfo().alphaType(),
                                             SkMatrix::I(),
                                             kWM,
                                             kWM,
                                             SkRect::Make(srcRect),
                                             kKernel,
                                             dir,
                                             *this->caps());
        } else {
            auto filter = rescaleMode == RescaleMode::kNearest ? GrSamplerState::Filter::kNearest
                                                               : GrSamplerState::Filter::kLinear;
            if (xform ||
                texView.origin() != stepDst->origin() ||
                !stepDst->copyScaled(texView.refProxy(), srcRect, stepDstRect, filter)) {
                // We could not or were unable to successful perform a scaling blit (which can be
                // much faster if texView isn't already texturable). Scale by drawing instead.
                std::tie(texView, srcRect) = ensureTexturable(texView, srcRect);
                if (!texView) {
                    return false;
                }
                auto srcRectF = SkRect::Make(srcRect);
                fp = GrTextureEffect::MakeSubset(std::move(texView),
                                                 this->colorInfo().alphaType(),
                                                 SkMatrix::I(),
                                                 {filter, GrSamplerState::MipmapMode::kNone},
                                                 srcRectF,
                                                 srcRectF,
                                                 *this->caps());
            }
        }
        if (xform) {
            SkASSERT(SkToBool(fp)); // shouldn't have done a copy if there was a color xform
            fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(xform));
        }
        if (fp) {
            // When fp is not null, we scale by drawing; if it is null, presumably the src has
            // already been copied into stepDst.
            stepDst->fillRectToRectWithFP(srcRect, stepDstRect, std::move(fp));
        }
        texView = stepDst->readSurfaceView();
        tempA = std::move(tempB);
        srcRect = SkIRect::MakeSize(nextDims);
    } while (srcRect.size() != finalSize);
    return true;
}

SurfaceContext::PixelTransferResult SurfaceContext::transferPixels(GrColorType dstCT,
                                                                   const SkIRect& rect) {
    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());
    auto direct = fContext->asDirectContext();
    if (!direct) {
        return {};
    }
    auto rtProxy = this->asRenderTargetProxy();
    if (rtProxy && rtProxy->wrapsVkSecondaryCB()) {
        return {};
    }

    auto proxy = this->asSurfaceProxy();
    auto supportedRead = this->caps()->supportedReadPixelsColorType(this->colorInfo().colorType(),
                                                                    proxy->backendFormat(), dstCT);
    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = GrColorTypeChannelFlags(dstCT);
    uint32_t legalReadChannels = GrColorTypeChannelFlags(supportedRead.fColorType);
    uint32_t srcChannels = GrColorTypeChannelFlags(this->colorInfo().colorType());
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        return {};
    }

    if (!this->caps()->transferFromSurfaceToBufferSupport() ||
        !supportedRead.fOffsetAlignmentForTransferBuffer) {
        return {};
    }

    size_t rowBytes = GrColorTypeBytesPerPixel(supportedRead.fColorType) * rect.width();
    rowBytes = SkAlignTo(rowBytes, this->caps()->transferBufferRowBytesAlignment());
    size_t size = rowBytes * rect.height();
    // By using kStream_GrAccessPattern here, we are not able to cache and reuse the buffer for
    // multiple reads. Switching to kDynamic_GrAccessPattern would allow for this, however doing
    // so causes a crash in a chromium test. See skbug.com/11297
    auto buffer = direct->priv().resourceProvider()->createBuffer(
            size,
            GrGpuBufferType::kXferGpuToCpu,
            GrAccessPattern::kStream_GrAccessPattern,
            GrResourceProvider::ZeroInit::kNo);
    if (!buffer) {
        return {};
    }
    auto srcRect = rect;
    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;
    if (flip) {
        srcRect = SkIRect::MakeLTRB(rect.fLeft, this->height() - rect.fBottom, rect.fRight,
                                    this->height() - rect.fTop);
    }
    this->drawingManager()->newTransferFromRenderTask(this->asSurfaceProxyRef(), srcRect,
                                                      this->colorInfo().colorType(),
                                                      supportedRead.fColorType, buffer, 0);
    PixelTransferResult result;
    result.fTransferBuffer = std::move(buffer);
    auto at = this->colorInfo().alphaType();
    if (supportedRead.fColorType != dstCT || flip) {
        int w = rect.width(), h = rect.height();
        GrImageInfo srcInfo(supportedRead.fColorType, at, nullptr, w, h);
        GrImageInfo dstInfo(dstCT, at, nullptr, w, h);
        result.fRowBytes = dstInfo.minRowBytes();
        result.fPixelConverter = [dstInfo, srcInfo, rowBytes](
                void* dst, const void* src) {
            GrConvertPixels( GrPixmap(dstInfo, dst, dstInfo.minRowBytes()),
                            GrCPixmap(srcInfo, src, rowBytes));
        };
    } else {
        result.fRowBytes = rowBytes;
    }
    return result;
}

#ifdef SK_DEBUG
void SurfaceContext::validate() const {
    SkASSERT(fReadView.proxy());
    fReadView.proxy()->validate(fContext);
    if (this->colorInfo().colorType() != GrColorType::kUnknown) {
        SkASSERT(fContext->priv().caps()->areColorTypeAndFormatCompatible(
                this->colorInfo().colorType(), fReadView.proxy()->backendFormat()));
    }
    this->onValidate();
}
#endif

}  // namespace skgpu::ganesh
