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
#include "SkDeferredDisplayListRecorder.h"

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
        sk_sp<PromiseImageCallbackContext> callbackContext(
                                                new PromiseImageCallbackContext(context));

        const PromiseImageInfo& info = fImageInfo[i];

        // DDL TODO: how can we tell if we need mipmapping!
        callbackContext->setBackendTexture(gpu->createTestingOnlyBackendTexture(
                                                            info.fBitmap.getPixels(),
                                                            info.fBitmap.width(),
                                                            info.fBitmap.height(),
                                                            info.fBitmap.colorType(),
                                                            false, GrMipMapped::kNo));
        // The GMs sometimes request too large an image
        //SkAssertResult(callbackContext->backendTexture().isValid());

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

    if (!curImage.fCallbackContext->backendTexture().isValid()) {
        // We weren't able to make a backend texture for this SkImage. In this case we create
        // a separate bitmap-backed image for each thread.
        // Note: we would like to share the same bitmap between all the threads but
        // SkBitmap is not thread-safe.
        return SkImage::MakeRasterCopy(curImage.fBitmap.pixmap());
    }
    SkASSERT(curImage.fIndex == *indexPtr);

    const GrCaps* caps = curImage.fCallbackContext->caps();
    const GrBackendTexture& backendTex = curImage.fCallbackContext->backendTexture();
    GrBackendFormat backendFormat = caps->createFormatFromBackendTexture(backendTex);

    // Each DDL recorder gets its own ref on the promise callback context for the
    // promise images it creates.
    // DDL TODO: sort out mipmapping
    sk_sp<SkImage> image = recorder->makePromiseTexture(
                                            backendFormat,
                                            curImage.fBitmap.width(),
                                            curImage.fBitmap.height(),
                                            GrMipMapped::kNo,
                                            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                            curImage.fBitmap.colorType(),
                                            curImage.fBitmap.alphaType(),
                                            curImage.fBitmap.refColorSpace(),
                                            DDLPromiseImageHelper::PromiseImageFulfillProc,
                                            DDLPromiseImageHelper::PromiseImageReleaseProc,
                                            DDLPromiseImageHelper::PromiseImageDoneProc,
                                            (void*) SkSafeRef(curImage.fCallbackContext.get()));
    perRecorderContext->fPromiseImages->push_back(image);
    SkASSERT(image);
    return image;
}

int DDLPromiseImageHelper::findImage(SkImage* image) const {
    for (int i = 0; i < fImageInfo.count(); ++i) {
        if (fImageInfo[i].fOriginalUniqueID == image->uniqueID()) { // trying to dedup here
            SkASSERT(fImageInfo[i].fIndex == i);
            SkASSERT(this->isValidID(i) && this->isValidID(fImageInfo[i].fIndex));
            return i;
        }
    }
    return -1;
}

int DDLPromiseImageHelper::addImage(SkImage* image) {
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
    newImageInfo.fOriginalUniqueID = image->uniqueID();
    newImageInfo.fBitmap = bm;
    /* fCallbackContext is filled in by uploadAllToGPU */

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
