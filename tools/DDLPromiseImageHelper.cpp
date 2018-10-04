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
#include "SkYUVSizeInfo.h"

DDLPromiseImageHelper::PromiseImageCallbackContext::~PromiseImageCallbackContext() {
    GrGpu* gpu = fContext->contextPriv().getGpu();

    if (fBackendTexture.isValid()) {
        gpu->deleteTestingOnlyBackendTexture(fBackendTexture);
    }
}

const GrCaps* DDLPromiseImageHelper::PromiseImageCallbackContext::caps() const {
    return fContext->contextPriv().caps();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLPromiseImageHelper::~DDLPromiseImageHelper() {}

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

void DDLPromiseImageHelper::uploadAllToGPU(GrContext* context) {
    GrGpu* gpu = context->contextPriv().getGpu();
    SkASSERT(gpu);

    for (int i = 0; i < fImageInfo.count(); ++i) {
        const PromiseImageInfo& info = fImageInfo[i];

        // DDL TODO: how can we tell if we need mipmapping!
        if (info.isYUV()) {
            for (int j = 0; j < 3; ++j) {
                const SkPixmap& yuvPixmap = info.yuvPixmap(j);

                sk_sp<PromiseImageCallbackContext> callbackContext(
                                                        new PromiseImageCallbackContext(context));
                callbackContext->setBackendTexture(gpu->createTestingOnlyBackendTexture(
                                                            yuvPixmap.addr(),
                                                            yuvPixmap.width(),
                                                            yuvPixmap.height(),
                                                            yuvPixmap.colorType(),
                                                            false, GrMipMapped::kNo,
                                                            yuvPixmap.rowBytes()));
                SkAssertResult(callbackContext->backendTexture().isValid());

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

    if (!curImage.backendTexture(0).isValid()) {
        SkASSERT(!curImage.isYUV());
        // We weren't able to make a backend texture for this SkImage. In this case we create
        // a separate bitmap-backed image for each thread.
        SkASSERT(curImage.normalBitmap().isImmutable());
        return SkImage::MakeFromBitmap(curImage.normalBitmap());
    }
    SkASSERT(curImage.index() == *indexPtr);

    const GrCaps* caps = curImage.caps();

    sk_sp<SkImage> image;
    if (curImage.isYUV()) {
        GrBackendFormat backendFormats[4];
        void* contexts[4] = { nullptr, nullptr, nullptr, nullptr };

        for (int i = 0; i < 3; ++i) {
            const GrBackendTexture& backendTex = curImage.backendTexture(i);
            backendFormats[i] = caps->createFormatFromBackendTexture(backendTex);

            contexts[i] = curImage.refCallbackContext(i).release();
        }

        SkYUVAIndex yuvaIndices[4] = {
                SkYUVAIndex{0, SkColorChannel::kA},
                SkYUVAIndex{1, SkColorChannel::kA},
                SkYUVAIndex{2, SkColorChannel::kA},
                SkYUVAIndex{-1, SkColorChannel::kA}
        };

        int tempWidth = curImage.backendTexture(0).width();
        int tempHeight = curImage.backendTexture(0).height();

        image = recorder->makeYUVAPromiseTexture(curImage.yuvColorSpace(),
                                                 backendFormats,
                                                 yuvaIndices,
                                                 tempWidth,  //curImage.overallWidth(),
                                                 tempHeight, //curImage.overallHeight(),
                                                 GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                                 curImage.refOverallColorSpace(),
                                                 DDLPromiseImageHelper::PromiseImageFulfillProc,
                                                 DDLPromiseImageHelper::PromiseImageReleaseProc,
                                                 DDLPromiseImageHelper::PromiseImageDoneProc,
                                                 contexts);

    } else {
        const GrBackendTexture& backendTex = curImage.backendTexture(0);
        GrBackendFormat backendFormat = caps->createFormatFromBackendTexture(backendTex);

        // Each DDL recorder gets its own ref on the promise callback context for the
        // promise images it creates.
        // DDL TODO: sort out mipmapping
        image = recorder->makePromiseTexture(backendFormat,
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
                                             (void*) curImage.refCallbackContext(0).release());
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

    SkYUVSizeInfo yuvSizeInfo;
    SkYUVColorSpace yuvColorSpace;
    const void* planes[3];
    sk_sp<SkCachedData> yuvData = ib->getPlanes(&yuvSizeInfo, &yuvColorSpace, planes);
    if (yuvData) {
        newImageInfo.setYUVData(std::move(yuvData), yuvColorSpace);

        for (int i = 0; i < 3; ++i) {
            SkImageInfo planeII = SkImageInfo::MakeA8(yuvSizeInfo.fSizes[i].fWidth,
                                                      yuvSizeInfo.fSizes[i].fHeight);
            newImageInfo.addYUVPlane(i, planeII, planes[i], yuvSizeInfo.fWidthBytes[i]);
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
