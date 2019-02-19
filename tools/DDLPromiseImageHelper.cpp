/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DDLPromiseImageHelper.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "SkCachedData.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkImage_Base.h"
#include "SkYUVAIndex.h"
#include "SkYUVASizeInfo.h"
#include "gl/GrGLDefines.h"
#ifdef SK_VULKAN
#include <vulkan/vulkan_core.h>
#endif

sk_sp<SkImage> Foo::makePromiseTexture(const GrBackendFormat& backendFormat,
                                       int width,
                                       int height,
                                       GrMipMapped mipMapped,
                                       GrSurfaceOrigin origin,
                                       SkColorType colorType,
                                       SkAlphaType alphaType,
                                       sk_sp<SkColorSpace> colorSpace,
                                       PromiseImageTextureFulfillProc textureFulfillProc,
                                       PromiseImageTextureReleaseProc textureReleaseProc,
                                       PromiseImageTextureDoneProc textureDoneProc,
                                       PromiseImageTextureContext textureContext,
                                       DelayReleaseCallback delayReleaseCallback) {
    return nullptr;
}

sk_sp<SkImage> Foo::makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
                                           const GrBackendFormat yuvaFormats[],
                                           const SkISize yuvaSizes[],
                                           const SkYUVAIndex yuvaIndices[4],
                                           int imageWidth,
                                           int imageHeight,
                                           GrSurfaceOrigin imageOrigin,
                                           sk_sp<SkColorSpace> imageColorSpace,
                                           PromiseImageTextureFulfillProc textureFulfillProc,
                                           PromiseImageTextureReleaseProc textureReleaseProc,
                                           PromiseImageTextureDoneProc textureDoneProc,
                                           PromiseImageTextureContext textureContexts[],
                                           DelayReleaseCallback delayReleaseCallback) {
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLPromiseImageHelper::PromiseImageCallbackContext::~PromiseImageCallbackContext() {
    SkASSERT(fDoneCnt == fNumImages);
    SkASSERT(!fUnreleasedFulfills);
    SkASSERT(fTotalReleases == fTotalFulfills);
    SkASSERT(!fTotalFulfills || fDoneCnt);

    if (fPromiseImageTexture) {
        GrGpu* gpu = fContext->priv().getGpu();
        gpu->deleteTestingOnlyBackendTexture(fPromiseImageTexture->backendTexture());
    }
}

void DDLPromiseImageHelper::PromiseImageCallbackContext::setBackendTexture(
        const GrBackendTexture& backendTexture) {
    SkASSERT(!fUnreleasedFulfills);
    if (fPromiseImageTexture) {
        GrGpu* gpu = fContext->priv().getGpu();
        gpu->deleteTestingOnlyBackendTexture(fPromiseImageTexture->backendTexture());
    }
    fPromiseImageTexture = SkPromiseImageTexture::Make(backendTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLPromiseImageHelper::~DDLPromiseImageHelper() {}

sk_sp<SkData> DDLPromiseImageHelper::deflateSKP(const SkPicture* inputPicture) {
    SkSerialProcs procs;

    procs.fImageCtx = this;
    procs.fImageProc = [](SkImage* image, void* imageCtx) -> sk_sp<SkData> {
        auto helper = static_cast<DDLPromiseImageHelper*>(imageCtx);

        int id = helper->findOrDefineImage(image);
        if (id >= 0) {
            SkASSERT(helper->isValidID(id));
            return SkData::MakeWithCopy(&id, sizeof(id));
        }

        return nullptr;
    };

    return inputPicture->serialize(&procs);
}

// needed until we have SkRG_88_ColorType;
static GrBackendTexture create_yuva_texture(GrGpu* gpu, const SkPixmap& pm,
                                            const SkYUVAIndex yuvaIndices[4], int texIndex) {
    SkASSERT(texIndex >= 0 && texIndex <= 3);
    int channelCount = 0;
    for (int i = 0; i < SkYUVAIndex::kIndexCount; ++i) {
        if (yuvaIndices[i].fIndex == texIndex) {
            ++channelCount;
        }
    }
    // Need to create an RG texture for two-channel planes
    GrBackendTexture tex;
    if (2 == channelCount) {
        SkASSERT(kRGBA_8888_SkColorType == pm.colorType());
        SkAutoTMalloc<char> pixels(2 * pm.width()*pm.height());
        char* currPixel = pixels;
        for (int y = 0; y < pm.height(); ++y) {
            for (int x = 0; x < pm.width(); ++x) {
                SkColor color = pm.getColor(x, y);
                currPixel[0] = SkColorGetR(color);
                currPixel[1] = SkColorGetG(color);
                currPixel += 2;
            }
        }
        tex = gpu->createTestingOnlyBackendTexture(
            pixels,
            pm.width(),
            pm.height(),
            GrColorType::kRG_88,
            false,
            GrMipMapped::kNo,
            2 * pm.width());
    } else {
        tex = gpu->createTestingOnlyBackendTexture(
            pm.addr(),
            pm.width(),
            pm.height(),
            pm.colorType(),
            false,
            GrMipMapped::kNo,
            pm.rowBytes());
    }
    return tex;
}

void DDLPromiseImageHelper::createPromiseImages(Foo* foo) {
    for (int i = 0; i < fImageInfo.count(); ++i) {
        PromiseImageInfo& info = fImageInfo[i];

        info.createPromiseImage(foo, fDelayReleaseCallback);
    }
}

void DDLPromiseImageHelper::uploadAllToGPU(GrContext* context) {
    GrGpu* gpu = context->priv().getGpu();
    SkASSERT(gpu);

    for (int i = 0; i < fImageInfo.count(); ++i) {
        const PromiseImageInfo& info = fImageInfo[i];

        // DDL TODO: how can we tell if we need mipmapping!
        if (info.isYUV()) {
            int numPixmaps;
            SkAssertResult(SkYUVAIndex::AreValidIndices(info.yuvaIndices(), &numPixmaps));
            for (int j = 0; j < numPixmaps; ++j) {
                const SkPixmap& yuvPixmap = info.yuvPixmap(j);

                sk_sp<PromiseImageCallbackContext> callbackContext(
                                                        new PromiseImageCallbackContext(context, true));

                callbackContext->setBackendTexture(create_yuva_texture(gpu, yuvPixmap,
                                                                       info.yuvaIndices(), j));
                SkASSERT(callbackContext->promiseImageTexture());

                fImageInfo[i].setCallbackContext1(j, std::move(callbackContext));
            }
        } else {
            sk_sp<PromiseImageCallbackContext> callbackContext(
                                                    new PromiseImageCallbackContext(context, true));

            const SkBitmap& bm = info.normalBitmap();

            callbackContext->setBackendTexture(gpu->createTestingOnlyBackendTexture(
                                                                bm.getPixels(),
                                                                bm.width(),
                                                                bm.height(),
                                                                bm.colorType(),
                                                                false, GrMipMapped::kNo,
                                                                bm.rowBytes()));
            // The GMs sometimes request too large an image
            //SkAssertResult(callbackContext->backendTexture().isValid());

            fImageInfo[i].setCallbackContext1(0, std::move(callbackContext));
        }
    }
}

void DDLPromiseImageHelper::replaceEveryOtherPromiseTexture(GrContext* context) {
    GrGpu* gpu = context->priv().getGpu();
    SkASSERT(gpu);

    for (int i = 0; i < fImageInfo.count(); i += 2) {
        PromiseImageInfo& info = fImageInfo[i];

        // DDL TODO: how can we tell if we need mipmapping!
        if (info.isYUV()) {
            int numPixmaps;
            SkAssertResult(SkYUVAIndex::AreValidIndices(info.yuvaIndices(), &numPixmaps));
            for (int j = 0; j < numPixmaps; ++j) {
                const SkPixmap& yuvPixmap = info.yuvPixmap(j);
                info.callbackContext1(j)->setBackendTexture(
                        create_yuva_texture(gpu, yuvPixmap, info.yuvaIndices(), j));
                SkASSERT(info.callbackContext1(j)->promiseImageTexture());
            }
        } else {
            const SkBitmap& bm = info.normalBitmap();
            info.callbackContext1(0)->setBackendTexture(gpu->createTestingOnlyBackendTexture(
                    bm.getPixels(), bm.width(), bm.height(), bm.colorType(), false,
                    GrMipMapped::kNo, bm.rowBytes()));
            // The GMs sometimes request too large an image
            // SkAssertResult(callbackContext->backendTexture().isValid());
        }
    }
}

sk_sp<SkPicture> DDLPromiseImageHelper::reinflateSKP(SkData* compressedPictureData) const {
    SkDeserialProcs procs;
    procs.fImageCtx = (void*) this;
    procs.fImageProc = FindOrCreatePromiseImage;

    return SkPicture::MakeFromData(compressedPictureData, &procs);
}

// This method finds the promise image that was created to replace a given image index
// in the compressed picture. If the promise image wasn't created, it will create a raster-backed
// image to fill in.
sk_sp<SkImage> DDLPromiseImageHelper::FindOrCreatePromiseImage(const void* rawData,
                                                               size_t length,
                                                               void* ctxIn) {
    const DDLPromiseImageHelper* helper = static_cast<const DDLPromiseImageHelper*>(ctxIn);

    SkASSERT(length == sizeof(int));

    const int* indexPtr = static_cast<const int*>(rawData);
    SkASSERT(helper->isValidID(*indexPtr));

    const DDLPromiseImageHelper::PromiseImageInfo& curImage = helper->getInfo(*indexPtr);
    SkASSERT(curImage.index() == *indexPtr);

    if (!curImage.promiseImage()) {
        SkASSERT(!curImage.isYUV());
        // The GMs sometimes request too large an image for it to be GPU backed.
        // In this case we create a separate bitmap-backed image for each thread.
        SkASSERT(curImage.normalBitmap().isImmutable());
        return SkImage::MakeFromBitmap(curImage.normalBitmap());
    }

    return curImage.promiseImage();
}

static GrBackendFormat create_backend_format(Foo* foo, int numChannels) {
    switch (foo->backend()) {
    case GrBackendApi::kOpenGL:
        switch (numChannels) {
        case 1:
            return GrBackendFormat::MakeGL(GR_GL_R8, GR_GL_TEXTURE_2D);
            break;
        case 2:
            return GrBackendFormat::MakeGL(GR_GL_RG8, GR_GL_TEXTURE_2D);
            break;
        case 3:
            return GrBackendFormat::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_2D);
            break;
        case 4:
            return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
            break;
        }
        break;
    case GrBackendApi::kVulkan:
        switch (numChannels) {
        case 1:
            return GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM);
            break;
        case 2:
            return GrBackendFormat::MakeVk(VK_FORMAT_R8G8_UNORM);
            break;
        case 3:
            return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8_UNORM);
            break;
        case 4:
            return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
            break;
        }
        break;
#ifdef SK_METAL
    case GrBackendApi::kMetal:
        return GrBackendFormat();
        break;
#endif
    case GrBackendApi::kMock:
        switch (numChannels) {
        case 1:
            return GrBackendFormat::MakeMock(kAlpha_8_as_Red_GrPixelConfig);
            break;
        case 2:
            return GrBackendFormat::MakeMock(kRG_88_GrPixelConfig);
            break;
        case 3:
            return GrBackendFormat::MakeMock(kRGB_888_GrPixelConfig);
            break;
        case 4:
            return GrBackendFormat::MakeMock(kRGBA_8888_GrPixelConfig);
            break;
        }
        break;
    }

    return GrBackendFormat();
}

// This method creates a promise image for the data extracted from the skp.
void DDLPromiseImageHelper::PromiseImageInfo::createPromiseImage(
        Foo* foo, DelayReleaseCallback delayReleaseCallback) {

    if (this->isYUV()) {
        GrBackendFormat backendFormats[SkYUVASizeInfo::kMaxCount];
        void* contexts[SkYUVASizeInfo::kMaxCount] = { nullptr, nullptr, nullptr, nullptr };
        SkISize sizes[SkYUVASizeInfo::kMaxCount];
        // TODO: store this value somewhere?
        int textureCount;
        SkAssertResult(SkYUVAIndex::AreValidIndices(this->yuvaIndices(), &textureCount));
        for (int i = 0; i < textureCount; ++i) {
            backendFormats[i] = create_backend_format(foo, this->numChannelsPerPlane(i));
            SkASSERT(backendFormats[i].isValid());
            contexts[i] = this->refCallbackContext1(i).release();
            sizes[i].set(this->yuvPixmap(i).width(), this->yuvPixmap(i).height());
        }
        for (int i = textureCount; i < SkYUVASizeInfo::kMaxCount; ++i) {
            sizes[i] = SkISize::MakeEmpty();
        }

        fPromiseImage1 = foo->makeYUVAPromiseTexture(
                                            this->yuvColorSpace(),
                                            backendFormats,
                                            sizes,
                                            this->yuvaIndices(),
                                            this->overallWidth(),
                                            this->overallHeight(),
                                            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                            this->refOverallColorSpace(),
                                            DDLPromiseImageHelper::PromiseImageFulfillProc,
                                            DDLPromiseImageHelper::PromiseImageReleaseProc,
                                            DDLPromiseImageHelper::PromiseImageDoneProc,
                                            contexts,
                                            delayReleaseCallback);
        for (int i = 0; i < textureCount; ++i) {
            this->callbackContext1(i)->wasAddedToImage();
        }
    } else {
        GrBackendFormat backendFormat = create_backend_format(foo, this->numChannelsPerPlane(0));
        SkASSERT(backendFormat.isValid());

        // Each DDL recorder gets its own ref on the promise callback context for the
        // promise images it creates.
        // DDL TODO: sort out mipmapping
        fPromiseImage1 = foo->makePromiseTexture(
                                        backendFormat,
                                        this->overallWidth(),
                                        this->overallHeight(),
                                        GrMipMapped::kNo,
                                        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                        this->overallColorType(),
                                        this->overallAlphaType(),
                                        this->refOverallColorSpace(),
                                        DDLPromiseImageHelper::PromiseImageFulfillProc,
                                        DDLPromiseImageHelper::PromiseImageReleaseProc,
                                        DDLPromiseImageHelper::PromiseImageDoneProc,
                                        (void*)this->refCallbackContext1(0).release(),
                                        delayReleaseCallback);
        this->callbackContext1(0)->wasAddedToImage();
    }

}

int DDLPromiseImageHelper::findImage(SkImage* image) const {
    for (int i = 0; i < fImageInfo.count(); ++i) {
        if (fImageInfo[i].originalUniqueID() == image->uniqueID()) { // trying to dedup here
            SkASSERT(fImageInfo[i].index() == i);
            SkASSERT(this->isValidID(i) && this->isValidID(fImageInfo[i].index()));
            return i;
        }
    }
    return -1;
}

int DDLPromiseImageHelper::addImage(SkImage* image) {
    SkImage_Base* ib = as_IB(image);

    SkImageInfo overallII = SkImageInfo::Make(image->width(), image->height(),
                                              image->colorType(), image->alphaType(),
                                              image->refColorSpace());

    PromiseImageInfo& newImageInfo = fImageInfo.emplace_back(fImageInfo.count(),
                                                             image->uniqueID(),
                                                             overallII);

    SkYUVASizeInfo yuvaSizeInfo;
    SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount];
    SkYUVColorSpace yuvColorSpace;
    const void* planes[SkYUVASizeInfo::kMaxCount];
    sk_sp<SkCachedData> yuvData = ib->getPlanes(&yuvaSizeInfo, yuvaIndices, &yuvColorSpace, planes);
    if (yuvData) {
        newImageInfo.setYUVData(std::move(yuvData), yuvaIndices, yuvColorSpace);

        // determine colortypes from index data
        int numChannels = 0;
        int channels[SkYUVASizeInfo::kMaxCount] = { 0, 0, 0, 0 };
        for (int yuvIndex = 0; yuvIndex < SkYUVAIndex::kIndexCount; ++yuvIndex) {
            int texIdx = yuvaIndices[yuvIndex].fIndex;
            if (texIdx < 0) {
                SkASSERT(SkYUVAIndex::kA_Index == yuvIndex);
                continue;
            }

            SkASSERT(texIdx < SkYUVASizeInfo::kMaxCount);
            channels[texIdx]++;
            numChannels++;
        }

        SkASSERT(numChannels == 3 || numChannels == 4); // either YUV or YUVA

        SkColorType colorTypes[SkYUVASizeInfo::kMaxCount] = {
            kUnknown_SkColorType, kUnknown_SkColorType,
            kUnknown_SkColorType, kUnknown_SkColorType
        };
        for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
            switch (channels[i]) {
            case 0:   // unused channel
                break;
            case 1:
                colorTypes[i] = kAlpha_8_SkColorType;
                break;
            case 2:   // fall through
#if 0
                colorTypes[i] = kRG_88_SkColorType; // missing color type!
                break;
#endif
            case 3:
                colorTypes[i] = kRGB_888x_SkColorType;
                break;
            case 4:
                colorTypes[i] = kRGBA_8888_SkColorType;
                break;
            }
        }

        for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
            if (yuvaSizeInfo.fSizes[i].isEmpty()) {
                SkASSERT(!yuvaSizeInfo.fWidthBytes[i] && kUnknown_SkColorType == colorTypes[i]);
                continue;
            }

            SkImageInfo planeII = SkImageInfo::Make(yuvaSizeInfo.fSizes[i].fWidth,
                                                    yuvaSizeInfo.fSizes[i].fHeight,
                                                    colorTypes[i],
                                                    kUnpremul_SkAlphaType);
            newImageInfo.addYUVPlane(i, planeII, planes[i], yuvaSizeInfo.fWidthBytes[i],
                                     channels[i]);
        }
    } else {
        sk_sp<SkImage> rasterImage = image->makeRasterImage(); // force decoding of lazy images

        SkBitmap tmp;
        tmp.allocPixels(overallII);

        if (!rasterImage->readPixels(tmp.pixmap(), 0, 0)) {
            return -1;
        }

        tmp.setImmutable();
        newImageInfo.setNormalBitmap(tmp);
    }
    // In either case newImageInfo's PromiseImageCallbackContext is filled in by uploadAllToGPU

    return fImageInfo.count()-1;
}

int DDLPromiseImageHelper::findOrDefineImage(SkImage* image) {
    int preExistingID = this->findImage(image);
    if (preExistingID >= 0) {
        SkASSERT(this->isValidID(preExistingID));
        return preExistingID;
    }

    int newID = this->addImage(image);
    SkASSERT(this->isValidID(newID));
    return newID;
}
