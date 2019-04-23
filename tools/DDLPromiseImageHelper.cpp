/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DDLPromiseImageHelper.h"

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkCachedData.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuYUVA.h"

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
    SkASSERT(!fPromiseImageTexture);
    fPromiseImageTexture = SkPromiseImageTexture::Make(backendTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> DDLPromiseImageHelper::deflateSKP(const SkPicture* inputPicture) {
    SkSerialProcs procs;

    procs.fImageCtx = this;
    procs.fImageProc = [](SkImage* image, void* ctx) -> sk_sp<SkData> {
        auto helper = static_cast<DDLPromiseImageHelper*>(ctx);

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
                                                        new PromiseImageCallbackContext(context));

                callbackContext->setBackendTexture(create_yuva_texture(gpu, yuvPixmap,
                                                                       info.yuvaIndices(), j));
                SkASSERT(callbackContext->promiseImageTexture());

                fImageInfo[i].setCallbackContext(j, std::move(callbackContext));
            }
        } else {
            sk_sp<PromiseImageCallbackContext> callbackContext(
                                                    new PromiseImageCallbackContext(context));

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

            fImageInfo[i].setCallbackContext(0, std::move(callbackContext));
        }
    }
}

sk_sp<SkPicture> DDLPromiseImageHelper::reinflateSKP(
                                                   SkDeferredDisplayListRecorder* recorder,
                                                   SkData* compressedPictureData,
                                                   SkTArray<sk_sp<SkImage>>* promiseImages) const {
    PerRecorderContext perRecorderContext { recorder, this, promiseImages };

    SkDeserialProcs procs;
    procs.fImageCtx = (void*) &perRecorderContext;
    procs.fImageProc = PromiseImageCreator;

    return SkPicture::MakeFromData(compressedPictureData, &procs);
}

// This generates promise images to replace the indices in the compressed picture. This
// reconstitution is performed separately in each thread so we end up with multiple
// promise images referring to the same GrBackendTexture.
sk_sp<SkImage> DDLPromiseImageHelper::PromiseImageCreator(const void* rawData,
                                                          size_t length, void* ctxIn) {
    PerRecorderContext* perRecorderContext = static_cast<PerRecorderContext*>(ctxIn);
    const DDLPromiseImageHelper* helper = perRecorderContext->fHelper;
    SkDeferredDisplayListRecorder* recorder = perRecorderContext->fRecorder;

    SkASSERT(length == sizeof(int));

    const int* indexPtr = static_cast<const int*>(rawData);
    SkASSERT(helper->isValidID(*indexPtr));

    const DDLPromiseImageHelper::PromiseImageInfo& curImage = helper->getInfo(*indexPtr);

    if (!curImage.promiseTexture(0)) {
        SkASSERT(!curImage.isYUV());
        // We weren't able to make a backend texture for this SkImage. In this case we create
        // a separate bitmap-backed image for each thread.
        SkASSERT(curImage.normalBitmap().isImmutable());
        return SkImage::MakeFromBitmap(curImage.normalBitmap());
    }
    SkASSERT(curImage.index() == *indexPtr);

    sk_sp<SkImage> image;
    if (curImage.isYUV()) {
        GrBackendFormat backendFormats[SkYUVASizeInfo::kMaxCount];
        void* contexts[SkYUVASizeInfo::kMaxCount] = { nullptr, nullptr, nullptr, nullptr };
        SkISize sizes[SkYUVASizeInfo::kMaxCount];
        // TODO: store this value somewhere?
        int textureCount;
        SkAssertResult(SkYUVAIndex::AreValidIndices(curImage.yuvaIndices(), &textureCount));
        for (int i = 0; i < textureCount; ++i) {
            const GrBackendTexture& backendTex = curImage.promiseTexture(i)->backendTexture();
            backendFormats[i] = backendTex.getBackendFormat();
            SkASSERT(backendFormats[i].isValid());
            contexts[i] = curImage.refCallbackContext(i).release();
            sizes[i].set(curImage.yuvPixmap(i).width(), curImage.yuvPixmap(i).height());
        }
        for (int i = textureCount; i < SkYUVASizeInfo::kMaxCount; ++i) {
            sizes[i] = SkISize::MakeEmpty();
        }

        image = recorder->makeYUVAPromiseTexture(
                curImage.yuvColorSpace(),
                backendFormats,
                sizes,
                curImage.yuvaIndices(),
                curImage.overallWidth(),
                curImage.overallHeight(),
                GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                curImage.refOverallColorSpace(),
                DDLPromiseImageHelper::PromiseImageFulfillProc,
                DDLPromiseImageHelper::PromiseImageReleaseProc,
                DDLPromiseImageHelper::PromiseImageDoneProc,
                contexts,
                SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew);
        for (int i = 0; i < textureCount; ++i) {
            curImage.callbackContext(i)->wasAddedToImage();
        }

#ifdef SK_DEBUG
        {
            // By the peekProxy contract this image should not have a single backing proxy so
            // should return null. The call should also not trigger the conversion to RGBA.
            SkImage_GpuYUVA* yuva = reinterpret_cast<SkImage_GpuYUVA*>(image.get());
            SkASSERT(!yuva->peekProxy());
            SkASSERT(!yuva->peekProxy());  // the first call didn't force a conversion to RGBA
        }
#endif
    } else {
        const GrBackendTexture& backendTex = curImage.promiseTexture(0)->backendTexture();
        GrBackendFormat backendFormat = backendTex.getBackendFormat();
        SkASSERT(backendFormat.isValid());

        // Each DDL recorder gets its own ref on the promise callback context for the
        // promise images it creates.
        // DDL TODO: sort out mipmapping
        image = recorder->makePromiseTexture(
                backendFormat,
                curImage.overallWidth(),
                curImage.overallHeight(),
                GrMipMapped::kNo,
                GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                curImage.overallColorType(),
                curImage.overallAlphaType(),
                curImage.refOverallColorSpace(),
                DDLPromiseImageHelper::PromiseImageFulfillProc,
                DDLPromiseImageHelper::PromiseImageReleaseProc,
                DDLPromiseImageHelper::PromiseImageDoneProc,
                (void*)curImage.refCallbackContext(0).release(),
                SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew);
        curImage.callbackContext(0)->wasAddedToImage();
    }
    perRecorderContext->fPromiseImages->push_back(image);
    SkASSERT(image);
    return image;
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
        // for testing we only ever use A8 or RGBA8888
        SkColorType colorTypes[SkYUVASizeInfo::kMaxCount] = {
            kUnknown_SkColorType, kUnknown_SkColorType,
            kUnknown_SkColorType, kUnknown_SkColorType
        };
        for (int yuvIndex = 0; yuvIndex < SkYUVAIndex::kIndexCount; ++yuvIndex) {
            int texIdx = yuvaIndices[yuvIndex].fIndex;
            if (texIdx < 0) {
                SkASSERT(SkYUVAIndex::kA_Index == yuvIndex);
                continue;
            }
            if (kUnknown_SkColorType == colorTypes[texIdx]) {
                colorTypes[texIdx] = kAlpha_8_SkColorType;
            } else {
                colorTypes[texIdx] = kRGBA_8888_SkColorType;
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
            newImageInfo.addYUVPlane(i, planeII, planes[i], yuvaSizeInfo.fWidthBytes[i]);
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
