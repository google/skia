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

DDLPromiseImageHelper::PromiseImageInfo::PromiseImageInfo() {}
DDLPromiseImageHelper::PromiseImageInfo::~PromiseImageInfo() {}

DDLPromiseImageHelper::PromiseImageCallbackContext::~PromiseImageCallbackContext() {
    GrGpu* gpu = fContext->contextPriv().getGpu();

    for (int i = 0; i < 3; ++i) {
        if (fBackendTextures[i].isValid()) {
            gpu->deleteTestingOnlyBackendTexture(fBackendTextures[i]);
        }
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

        sk_sp<PromiseImageCallbackContext> callbackContext(
            new PromiseImageCallbackContext(context, SkToBool(info.fYUVData)));

        // DDL TODO: how can we tell if we need mipmapping!
        if (info.fYUVData) {
            for (int i = 0; i < 3; ++i) {
                // TODO: add rowBytes to createTestingOnlyBackendTexture
                callbackContext->setBackendTexture(i, gpu->createTestingOnlyBackendTexture(
                                                            info.fYUVPlanes[i],
                                                            info.fYUVSizeInfo.fSizes[i].fWidth,
                                                            info.fYUVSizeInfo.fSizes[i].fHeight,
                                                            kAlpha_8_SkColorType,
                                                            false, GrMipMapped::kNo));
                SkAssertResult(callbackContext->backendTexture(i).isValid());
            }
        } else {
            callbackContext->setBackendTexture(0, gpu->createTestingOnlyBackendTexture(
                                                                info.fBitmap1.getPixels(),
                                                                info.fBitmap1.width(),
                                                                info.fBitmap1.height(),
                                                                info.fBitmap1.colorType(),
                                                                false, GrMipMapped::kNo));
            // The GMs sometimes request too large an image
            //SkAssertResult(callbackContext->backendTexture().isValid());
        }

        // The fImageInfo array gets the creation ref
        fImageInfo[i].fCallbackContext = std::move(callbackContext);
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

    if (!curImage.fCallbackContext->backendTexture(0).isValid()) {
        SkASSERT(!curImage.fCallbackContext->isYUV());
        // We weren't able to make a backend texture for this SkImage. In this case we create
        // a separate bitmap-backed image for each thread.
        // Note: we would like to share the same bitmap between all the threads but
        // SkBitmap is not thread-safe.
        return SkImage::MakeRasterCopy(curImage.fBitmap1.pixmap());
    }
    SkASSERT(curImage.fIndex == *indexPtr);

    const GrCaps* caps = curImage.fCallbackContext->caps();

    sk_sp<SkImage> image;
    if (curImage.fCallbackContext->isYUV()) {
        GrBackendFormat backendFormats[4];

        for (int i = 0; i < 3; ++i) {
            const GrBackendTexture& backendTex = curImage.fCallbackContext->backendTexture(i);
            backendFormats[i] = caps->createFormatFromBackendTexture(backendTex);
        }

        image = recorder->makeYUVPromiseTexture(backendFormats);

    } else {
        const GrBackendTexture& backendTex = curImage.fCallbackContext->backendTexture(0);
        GrBackendFormat backendFormat = caps->createFormatFromBackendTexture(backendTex);

        // Each DDL recorder gets its own ref on the promise callback context for the
        // promise images it creates.
        // DDL TODO: sort out mipmapping
        image = recorder->makePromiseTexture(backendFormat,
                                             curImage.fBitmap1.width(),
                                             curImage.fBitmap1.height(),
                                             GrMipMapped::kNo,
                                             GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                             curImage.fBitmap1.colorType(),
                                             curImage.fBitmap1.alphaType(),
                                             curImage.fBitmap1.refColorSpace(),
                                             DDLPromiseImageHelper::PromiseImageFulfillProc,
                                             DDLPromiseImageHelper::PromiseImageReleaseProc,
                                             DDLPromiseImageHelper::PromiseImageDoneProc,
                                             (void*) SkSafeRef(curImage.fCallbackContext.get()));
    }
    perRecorderContext->fPromiseImages->push_back(image);
    SkASSERT(image);
    return image;
}

int DDLPromiseImageHelper::findImage(SkImage* image) const {
    for (int i = 0; i < fImageInfo.count(); ++i) {
        if (fImageInfo[i].fOriginalUniqueID1 == image->uniqueID()) { // trying to dedup here
            SkASSERT(fImageInfo[i].fIndex == i);
            SkASSERT(this->isValidID(i) && this->isValidID(fImageInfo[i].fIndex));
            return i;
        }
    }
    return -1;
}

#include "SkImage_Base.h"
#include "SkYUVSizeInfo.h"

int DDLPromiseImageHelper::addImage(SkImage* image) {
    SkImage_Base* ib = as_IB(image);

    SkYUVSizeInfo yuvSizeInfo;
    SkYUVColorSpace yuvColorSpace;
    const void* planes[3];
    sk_sp<SkCachedData> yuvData = ib->getPlanes(&yuvSizeInfo, &yuvColorSpace, planes);
    if (yuvData) {
        PromiseImageInfo& newImageInfo = fImageInfo.push_back();
        newImageInfo.fIndex = fImageInfo.count()-1;
        newImageInfo.fOriginalUniqueID1 = image->uniqueID();
//        newImageInfo.fBitmap = bm;
    } else {
        sk_sp<SkImage> rasterImage = image->makeRasterImage(); // force decoding of lazy images

        SkImageInfo ii = SkImageInfo::Make(rasterImage->width(), rasterImage->height(),
                                           rasterImage->colorType(), rasterImage->alphaType(),
                                           rasterImage->refColorSpace());

        SkBitmap bm;
        bm.allocPixels(ii);

        if (!rasterImage->readPixels(bm.pixmap(), 0, 0)) {
            return -1;
        }

        bm.setImmutable();

        PromiseImageInfo& newImageInfo = fImageInfo.push_back();
        newImageInfo.fIndex = fImageInfo.count()-1;
        newImageInfo.fOriginalUniqueID1 = image->uniqueID();
        newImageInfo.fBitmap1 = bm;
        /* fCallbackContext is filled in by uploadAllToGPU */
    }

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
