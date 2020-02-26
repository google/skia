/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DDLPromiseImageHelper.h"

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuYUVA.h"

DDLPromiseImageHelper::PromiseImageCallbackContext::~PromiseImageCallbackContext() {
    SkASSERT(fDoneCnt == fNumImages);
    SkASSERT(!fUnreleasedFulfills);
    SkASSERT(fTotalReleases == fTotalFulfills);
    SkASSERT(!fTotalFulfills || fDoneCnt);

    if (fPromiseImageTexture) {
        fContext->deleteBackendTexture(fPromiseImageTexture->backendTexture());
    }
}

void DDLPromiseImageHelper::PromiseImageCallbackContext::setBackendTexture(
        const GrBackendTexture& backendTexture) {
    SkASSERT(!fPromiseImageTexture);
    SkASSERT(fBackendFormat == backendTexture.getBackendFormat());
    fPromiseImageTexture = SkPromiseImageTexture::Make(backendTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> DDLPromiseImageHelper::deflateSKP(const SkPicture* inputPicture) {
    SkSerialProcs procs;

    procs.fImageCtx = this;
    procs.fImageProc = [](SkImage* image, void* ctx) -> sk_sp<SkData> {
        auto helper = static_cast<DDLPromiseImageHelper*>(ctx);

        int id = helper->findOrDefineImage(image);

        // Even if 'id' is invalid (i.e., -1) write it to the SKP
        return SkData::MakeWithCopy(&id, sizeof(id));
    };

    return inputPicture->serialize(&procs);
}

static GrBackendTexture create_yuva_texture(GrContext* context, const SkPixmap& pm,
                                            const SkYUVAIndex yuvaIndices[4], int texIndex) {
    SkASSERT(texIndex >= 0 && texIndex <= 3);

#ifdef SK_DEBUG
    int channelCount = 0;
    for (int i = 0; i < SkYUVAIndex::kIndexCount; ++i) {
        if (yuvaIndices[i].fIndex == texIndex) {
            ++channelCount;
        }
    }
    if (2 == channelCount) {
        SkASSERT(kR8G8_unorm_SkColorType == pm.colorType());
    }
#endif

    return context->createBackendTexture(&pm, 1, GrRenderable::kNo, GrProtected::kNo);
}

/*
 * Create backend textures and upload data to them for all the textures required to satisfy
 * a single promise image.
 * For YUV textures this will result in up to 4 actual textures.
 */
void DDLPromiseImageHelper::CreateBETexturesForPromiseImage(GrContext* context,
                                                            PromiseImageInfo* info) {
    SkASSERT(context->priv().asDirectContext());

    // DDL TODO: how can we tell if we need mipmapping!
    if (info->isYUV()) {
        int numPixmaps;
        SkAssertResult(SkYUVAIndex::AreValidIndices(info->yuvaIndices(), &numPixmaps));
        for (int j = 0; j < numPixmaps; ++j) {
            const SkPixmap& yuvPixmap = info->yuvPixmap(j);

            PromiseImageCallbackContext* callbackContext = info->callbackContext(j);
            SkASSERT(callbackContext);

            callbackContext->setBackendTexture(create_yuva_texture(context, yuvPixmap,
                                                                   info->yuvaIndices(), j));
            SkASSERT(callbackContext->promiseImageTexture());
        }
    } else {
        PromiseImageCallbackContext* callbackContext = info->callbackContext(0);
        if (!callbackContext) {
            // This texture would've been too large to fit on the GPU
            return;
        }

        const SkBitmap& bm = info->normalBitmap();

        GrBackendTexture backendTex = context->createBackendTexture(
                                                    &bm.pixmap(), 1, GrRenderable::kNo,
                                                    GrProtected::kNo);
        SkASSERT(backendTex.isValid());

        callbackContext->setBackendTexture(backendTex);
    }
}

void DDLPromiseImageHelper::DeleteBETexturesForPromiseImage(GrContext* context,
                                                            PromiseImageInfo* info) {
    SkASSERT(context->priv().asDirectContext());

    if (info->isYUV()) {
        int numPixmaps;
        SkAssertResult(SkYUVAIndex::AreValidIndices(info->yuvaIndices(), &numPixmaps));
        for (int j = 0; j < numPixmaps; ++j) {
            PromiseImageCallbackContext* callbackContext = info->callbackContext(j);
            SkASSERT(callbackContext);

            callbackContext->destroyBackendTexture();
            SkASSERT(!callbackContext->promiseImageTexture());
        }
    } else {
        PromiseImageCallbackContext* callbackContext = info->callbackContext(0);
        if (!callbackContext) {
            // This texture would've been too large to fit on the GPU
            return;
        }

        callbackContext->destroyBackendTexture();
        SkASSERT(!callbackContext->promiseImageTexture());
    }
}

void DDLPromiseImageHelper::createCallbackContexts(GrContext* context) {
    const GrCaps* caps = context->priv().caps();
    const int maxDimension = caps->maxTextureSize();

    for (int i = 0; i < fImageInfo.count(); ++i) {
        PromiseImageInfo& info = fImageInfo[i];

        if (info.isYUV()) {
            int numPixmaps;
            SkAssertResult(SkYUVAIndex::AreValidIndices(info.yuvaIndices(), &numPixmaps));

            for (int j = 0; j < numPixmaps; ++j) {
                const SkPixmap& yuvPixmap = info.yuvPixmap(j);

                GrBackendFormat backendFormat = context->defaultBackendFormat(yuvPixmap.colorType(),
                                                                              GrRenderable::kNo);

                sk_sp<PromiseImageCallbackContext> callbackContext(
                    new PromiseImageCallbackContext(context, backendFormat));

                info.setCallbackContext(j, std::move(callbackContext));
            }
        } else {
            const SkBitmap& bm = info.normalBitmap();

            // TODO: explicitly mark the PromiseImageInfo as too big and check in uploadAllToGPU
            if (maxDimension < std::max(bm.width(), bm.height())) {
                // This won't fit on the GPU. Fallback to a raster-backed image per tile.
                continue;
            }

            GrBackendFormat backendFormat = context->defaultBackendFormat(bm.pixmap().colorType(),
                                                                          GrRenderable::kNo);
            if (!caps->isFormatTexturable(backendFormat)) {
                continue;
            }


            sk_sp<PromiseImageCallbackContext> callbackContext(
                new PromiseImageCallbackContext(context, backendFormat));

            info.setCallbackContext(0, std::move(callbackContext));
        }
    }
}

void DDLPromiseImageHelper::uploadAllToGPU(SkTaskGroup* taskGroup, GrContext* context) {
    SkASSERT(context->priv().asDirectContext());

    if (taskGroup) {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            PromiseImageInfo* info = &fImageInfo[i];

            taskGroup->add([context, info]() { CreateBETexturesForPromiseImage(context, info); });
        }
    } else {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            CreateBETexturesForPromiseImage(context, &fImageInfo[i]);
        }
    }
}

void DDLPromiseImageHelper::deleteAllFromGPU(SkTaskGroup* taskGroup, GrContext* context) {
    SkASSERT(context->priv().asDirectContext());

    if (taskGroup) {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            PromiseImageInfo* info = &fImageInfo[i];

            taskGroup->add([context, info]() { DeleteBETexturesForPromiseImage(context, info); });
        }
    } else {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            DeleteBETexturesForPromiseImage(context, &fImageInfo[i]);
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
    procs.fImageProc = CreatePromiseImages;

    return SkPicture::MakeFromData(compressedPictureData, &procs);
}

// This generates promise images to replace the indices in the compressed picture. This
// reconstitution is performed separately in each thread so we end up with multiple
// promise images referring to the same GrBackendTexture.
sk_sp<SkImage> DDLPromiseImageHelper::CreatePromiseImages(const void* rawData,
                                                          size_t length, void* ctxIn) {
    PerRecorderContext* perRecorderContext = static_cast<PerRecorderContext*>(ctxIn);
    const DDLPromiseImageHelper* helper = perRecorderContext->fHelper;
    SkDeferredDisplayListRecorder* recorder = perRecorderContext->fRecorder;

    SkASSERT(length == sizeof(int));

    const int* indexPtr = static_cast<const int*>(rawData);
    if (!helper->isValidID(*indexPtr)) {
        return nullptr;
    }

    const DDLPromiseImageHelper::PromiseImageInfo& curImage = helper->getInfo(*indexPtr);

    // If there is no callback context that means 'createCallbackContexts' determined the
    // texture wouldn't fit on the GPU. Create a separate bitmap-backed image for each thread.
    if (!curImage.isYUV() && !curImage.callbackContext(0)) {
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
            backendFormats[i] = curImage.backendFormat(i);
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
        GrBackendFormat backendFormat = curImage.backendFormat(0);
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
                                              image->colorType() == kBGRA_8888_SkColorType
                                                        ? kRGBA_8888_SkColorType
                                                        : image->colorType(),
                                              image->alphaType(),
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
        // for testing we only ever use A8, RG_88
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
                colorTypes[texIdx] = kR8G8_unorm_SkColorType;
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
        if (!rasterImage) {
            return -1;
        }

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
    return newID;
}
