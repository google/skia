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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuYUVA.h"

DDLPromiseImageHelper::PromiseImageInfo::PromiseImageInfo(int index,
                                                          uint32_t originalUniqueID,
                                                          const SkImageInfo& ii)
        : fIndex(index)
        , fOriginalUniqueID(originalUniqueID)
        , fImageInfo(ii) {
}

DDLPromiseImageHelper::PromiseImageInfo::PromiseImageInfo(PromiseImageInfo&& other)
        : fIndex(other.fIndex)
        , fOriginalUniqueID(other.fOriginalUniqueID)
        , fImageInfo(other.fImageInfo)
        , fBaseLevel(other.fBaseLevel)
        , fMipLevels(std::move(other.fMipLevels))
        , fYUVAPixmaps(std::move(other.fYUVAPixmaps)) {
    for (int i = 0; i < SkYUVAInfo::kMaxPlanes; ++i) {
        fCallbackContexts[i] = std::move(other.fCallbackContexts[i]);
    }
}

DDLPromiseImageHelper::PromiseImageInfo::~PromiseImageInfo() {}

std::unique_ptr<SkPixmap[]> DDLPromiseImageHelper::PromiseImageInfo::normalMipLevels() const {
    SkASSERT(!this->isYUV());
    std::unique_ptr<SkPixmap[]> pixmaps(new SkPixmap[this->numMipLevels()]);
    pixmaps[0] = fBaseLevel.pixmap();
    if (fMipLevels) {
        for (int i = 0; i < fMipLevels->countLevels(); ++i) {
            SkMipmap::Level mipLevel;
            fMipLevels->getLevel(i, &mipLevel);
            pixmaps[i+1] = mipLevel.fPixmap;
        }
    }
    return pixmaps;
}

int DDLPromiseImageHelper::PromiseImageInfo::numMipLevels() const {
    SkASSERT(!this->isYUV());
    return fMipLevels ? fMipLevels->countLevels()+1 : 1;
}

void DDLPromiseImageHelper::PromiseImageInfo::setMipLevels(const SkBitmap& baseLevel,
                                                           std::unique_ptr<SkMipmap> mipLevels) {
    fBaseLevel = baseLevel;
    fMipLevels = std::move(mipLevels);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PromiseImageCallbackContext::~PromiseImageCallbackContext() {
    SkASSERT(fDoneCnt == fNumImages);
    SkASSERT(!fTotalFulfills || fDoneCnt);

    if (fPromiseImageTexture) {
        fContext->deleteBackendTexture(fPromiseImageTexture->backendTexture());
    }
}

void PromiseImageCallbackContext::setBackendTexture(const GrBackendTexture& backendTexture) {
    SkASSERT(!fPromiseImageTexture);
    SkASSERT(fBackendFormat == backendTexture.getBackendFormat());
    fPromiseImageTexture = SkPromiseImageTexture::Make(backendTexture);
}

void PromiseImageCallbackContext::destroyBackendTexture() {
    SkASSERT(!fPromiseImageTexture || fPromiseImageTexture->unique());

    if (fPromiseImageTexture) {
        fContext->deleteBackendTexture(fPromiseImageTexture->backendTexture());
    }
    fPromiseImageTexture = nullptr;
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

static GrBackendTexture create_yuva_texture(GrDirectContext* direct,
                                            const SkPixmap& pm,
                                            int texIndex) {
    SkASSERT(texIndex >= 0 && texIndex <= 3);

    bool finishedBECreate = false;
    auto markFinished = [](void* context) {
        *(bool*)context = true;
    };
    auto beTex = direct->createBackendTexture(pm,
                                              kTopLeft_GrSurfaceOrigin,
                                              GrRenderable::kNo,
                                              GrProtected::kNo,
                                              markFinished,
                                              &finishedBECreate);
    if (beTex.isValid()) {
        direct->submit();
        while (!finishedBECreate) {
            direct->checkAsyncWorkCompletion();
        }
    }
    return beTex;
}

/*
 * Create backend textures and upload data to them for all the textures required to satisfy
 * a single promise image.
 * For YUV textures this will result in up to 4 actual textures.
 */
void DDLPromiseImageHelper::CreateBETexturesForPromiseImage(GrDirectContext* direct,
                                                            PromiseImageInfo* info) {
    if (info->isYUV()) {
        int numPixmaps = info->yuvaInfo().numPlanes();
        for (int j = 0; j < numPixmaps; ++j) {
            const SkPixmap& yuvPixmap = info->yuvPixmap(j);

            PromiseImageCallbackContext* callbackContext = info->callbackContext(j);
            SkASSERT(callbackContext);

            // DDL TODO: what should we do with mipmapped YUV images
            callbackContext->setBackendTexture(create_yuva_texture(direct, yuvPixmap, j));
            SkASSERT(callbackContext->promiseImageTexture());
        }
    } else {
        PromiseImageCallbackContext* callbackContext = info->callbackContext(0);
        if (!callbackContext) {
            // This texture would've been too large to fit on the GPU
            return;
        }

        std::unique_ptr<SkPixmap[]> mipLevels = info->normalMipLevels();

        bool finishedBECreate = false;
        auto markFinished = [](void* context) {
            *(bool*)context = true;
        };
        auto backendTex = direct->createBackendTexture(mipLevels.get(),
                                                       info->numMipLevels(),
                                                       kTopLeft_GrSurfaceOrigin,
                                                       GrRenderable::kNo,
                                                       GrProtected::kNo,
                                                       markFinished,
                                                       &finishedBECreate);
        SkASSERT(backendTex.isValid());
        direct->submit();
        while (!finishedBECreate) {
            direct->checkAsyncWorkCompletion();
        }

        callbackContext->setBackendTexture(backendTex);
    }
}

void DDLPromiseImageHelper::DeleteBETexturesForPromiseImage(PromiseImageInfo* info) {
    if (info->isYUV()) {
        int numPixmaps = info->yuvaInfo().numPlanes();
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

void DDLPromiseImageHelper::createCallbackContexts(GrDirectContext* direct) {
    const GrCaps* caps = direct->priv().caps();
    const int maxDimension = caps->maxTextureSize();

    for (int i = 0; i < fImageInfo.count(); ++i) {
        PromiseImageInfo& info = fImageInfo[i];

        if (info.isYUV()) {
            int numPixmaps = info.yuvaInfo().numPlanes();

            for (int j = 0; j < numPixmaps; ++j) {
                const SkPixmap& yuvPixmap = info.yuvPixmap(j);

                GrBackendFormat backendFormat = direct->defaultBackendFormat(yuvPixmap.colorType(),
                                                                             GrRenderable::kNo);

                sk_sp<PromiseImageCallbackContext> callbackContext(
                    new PromiseImageCallbackContext(direct, backendFormat));

                info.setCallbackContext(j, std::move(callbackContext));
            }
        } else {
            const SkBitmap& baseLevel = info.baseLevel();

            // TODO: explicitly mark the PromiseImageInfo as too big and check in uploadAllToGPU
            if (maxDimension < std::max(baseLevel.width(), baseLevel.height())) {
                // This won't fit on the GPU. Fallback to a raster-backed image per tile.
                continue;
            }

            GrBackendFormat backendFormat = direct->defaultBackendFormat(baseLevel.colorType(),
                                                                         GrRenderable::kNo);
            if (!caps->isFormatTexturable(backendFormat)) {
                continue;
            }

            sk_sp<PromiseImageCallbackContext> callbackContext(
                new PromiseImageCallbackContext(direct, backendFormat));

            info.setCallbackContext(0, std::move(callbackContext));
        }
    }
}

void DDLPromiseImageHelper::uploadAllToGPU(SkTaskGroup* taskGroup, GrDirectContext* direct) {
    if (taskGroup) {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            PromiseImageInfo* info = &fImageInfo[i];

            taskGroup->add([direct, info]() { CreateBETexturesForPromiseImage(direct, info); });
        }
    } else {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            CreateBETexturesForPromiseImage(direct, &fImageInfo[i]);
        }
    }
}

void DDLPromiseImageHelper::deleteAllFromGPU(SkTaskGroup* taskGroup, GrDirectContext* direct) {
    if (taskGroup) {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            PromiseImageInfo* info = &fImageInfo[i];

            taskGroup->add([info]() { DeleteBETexturesForPromiseImage(info); });
        }
    } else {
        for (int i = 0; i < fImageInfo.count(); ++i) {
            DeleteBETexturesForPromiseImage(&fImageInfo[i]);
        }
    }
}

sk_sp<SkPicture> DDLPromiseImageHelper::reinflateSKP(
                                                   sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                                                   SkData* compressedPictureData,
                                                   SkTArray<sk_sp<SkImage>>* promiseImages) const {
    DeserialImageProcContext procContext { std::move(threadSafeProxy), this, promiseImages };

    SkDeserialProcs procs;
    procs.fImageCtx = (void*) &procContext;
    procs.fImageProc = CreatePromiseImages;

    return SkPicture::MakeFromData(compressedPictureData, &procs);
}

// This generates promise images to replace the indices in the compressed picture. This
// reconstitution is performed separately in each thread so we end up with multiple
// promise images referring to the same GrBackendTexture.
sk_sp<SkImage> DDLPromiseImageHelper::CreatePromiseImages(const void* rawData,
                                                          size_t length, void* ctxIn) {
    DeserialImageProcContext* procContext = static_cast<DeserialImageProcContext*>(ctxIn);
    const DDLPromiseImageHelper* helper = procContext->fHelper;

    SkASSERT(length == sizeof(int));

    const int* indexPtr = static_cast<const int*>(rawData);
    if (!helper->isValidID(*indexPtr)) {
        return nullptr;
    }

    const DDLPromiseImageHelper::PromiseImageInfo& curImage = helper->getInfo(*indexPtr);

    // If there is no callback context that means 'createCallbackContexts' determined the
    // texture wouldn't fit on the GPU. Create a separate bitmap-backed image for each thread.
    if (!curImage.isYUV() && !curImage.callbackContext(0)) {
        SkASSERT(curImage.baseLevel().isImmutable());
        return curImage.baseLevel().asImage();
    }

    SkASSERT(curImage.index() == *indexPtr);

    sk_sp<SkImage> image;
    if (curImage.isYUV()) {
        GrBackendFormat backendFormats[SkYUVAInfo::kMaxPlanes];
        const SkYUVAInfo& yuvaInfo = curImage.yuvaInfo();
        void* contexts[SkYUVAInfo::kMaxPlanes] = {nullptr, nullptr, nullptr, nullptr};
        int textureCount = yuvaInfo.numPlanes();
        for (int i = 0; i < textureCount; ++i) {
            backendFormats[i] = curImage.backendFormat(i);
            contexts[i] = curImage.refCallbackContext(i).release();
        }
        GrYUVABackendTextureInfo yuvaBackendTextures(yuvaInfo,
                                                     backendFormats,
                                                     GrMipmapped::kNo,
                                                     kTopLeft_GrSurfaceOrigin);
        image = SkImage::MakePromiseYUVATexture(
                                            procContext->fThreadSafeProxy,
                                            yuvaBackendTextures,
                                            curImage.refOverallColorSpace(),
                                            PromiseImageCallbackContext::PromiseImageFulfillProc,
                                            PromiseImageCallbackContext::PromiseImageReleaseProc,
                                            contexts);
        if (!image) {
            return nullptr;
        }
        for (int i = 0; i < textureCount; ++i) {
            curImage.callbackContext(i)->wasAddedToImage();
        }

    } else {
        const GrBackendFormat& backendFormat = curImage.backendFormat(0);
        SkASSERT(backendFormat.isValid());

        image = SkImage::MakePromiseTexture(procContext->fThreadSafeProxy,
                                            backendFormat,
                                            curImage.overallDimensions(),
                                            curImage.mipMapped(0),
                                            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                            curImage.overallColorType(),
                                            curImage.overallAlphaType(),
                                            curImage.refOverallColorSpace(),
                                            PromiseImageCallbackContext::PromiseImageFulfillProc,
                                            PromiseImageCallbackContext::PromiseImageReleaseProc,
                                            (void*)curImage.refCallbackContext(0).release());
        curImage.callbackContext(0)->wasAddedToImage();
    }
    procContext->fPromiseImages->push_back(image);
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

    auto codec = SkCodecImageGenerator::MakeFromEncodedCodec(ib->refEncodedData());
    SkYUVAPixmapInfo yuvaInfo;
    if (codec && codec->queryYUVAInfo(fSupportedYUVADataTypes, &yuvaInfo)) {
        auto yuvaPixmaps = SkYUVAPixmaps::Allocate(yuvaInfo);
        if (!codec->getYUVAPlanes(yuvaPixmaps)) {
            return -1;
        }
        SkASSERT(yuvaPixmaps.isValid());
        newImageInfo.setYUVPlanes(std::move(yuvaPixmaps));
    } else {
        sk_sp<SkImage> rasterImage = image->makeRasterImage(); // force decoding of lazy images
        if (!rasterImage) {
            return -1;
        }

        SkBitmap tmp;
        tmp.allocPixels(overallII);

        if (!rasterImage->readPixels(nullptr, tmp.pixmap(), 0, 0)) {
            return -1;
        }

        tmp.setImmutable();

        // Given how the DDL testing harness works (i.e., only modifying the SkImages w/in an
        // SKP) we don't know if a given SkImage will require mipmapping. To work around this
        // we just try to create all the backend textures as mipmapped but, failing that, fall
        // back to un-mipped.
        std::unique_ptr<SkMipmap> mipmaps(SkMipmap::Build(tmp.pixmap(), nullptr));

        newImageInfo.setMipLevels(tmp, std::move(mipmaps));
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
