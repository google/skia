/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PromiseImageHelper_DEFINED
#define PromiseImageHelper_DEFINED

#include "SkBitmap.h"
#include "SkTArray.h"

#include "GrBackendSurface.h"

class GrContext;
class SkDeferredDisplayListRecorder;
class SkImage;
class SkPicture;

// This class consolidates tracking & extraction of the original image data from an skp,
// the upload of said data to the GPU and the fulfillment of promise images.
//
// The way this works is:
//    the original skp is converted to SkData and all its image info is extracted into this
//       class and only indices into this class are left in the SkData (via deflateSKP)
//
//    Prior to replaying in threads, all the images stored in this class are uploaded to the
//       gpu and PromiseImageCallbackContexts are created for them (via uploadAllToGPU)
//
//    Each thread reinflates the SkData into an SkPicture replacing all the indices w/
//       promise images (all using the same GrBackendTexture and getting a ref to the
//       appropriate PromiseImageCallbackContext) (via reinflateSKP).
//
//    This class is then reset - dropping all of its refs on the PromiseImageCallbackContexts
//
//    Each done callback unrefs its PromiseImageCallbackContext so, once all the promise images
//       are done the PromiseImageCallbackContext is freed and its GrBackendTexture removed
//       from VRAM
//
// Note: if DDLs are going to be replayed multiple times, the reset call can be delayed until
// all the replaying is complete. This will pin the GrBackendTextures in VRAM.
class DDLPromiseImageHelper {
public:
    DDLPromiseImageHelper() { }

    // Convert the SkPicture into SkData replacing all the SkImages with an index.
    sk_sp<SkData> deflateSKP(const SkPicture* inputPicture);

    void uploadAllToGPU(GrContext* context);

    // reinflate a deflated SKP, replacing all the indices with promise images.
    sk_sp<SkPicture> reinflateSKP(SkDeferredDisplayListRecorder*,
                                  SkData* compressedPicture,
                                  SkTArray<sk_sp<SkImage>>* promiseImages) const;

    // Remove this class' refs on the PromiseImageCallbackContexts
    void reset() { fImageInfo.reset(); }

private:
    // This class acts as a proxy for the single GrBackendTexture representing an image.
    // Whenever a promise image is created for the image, the promise image receives a ref to
    // this object. Once all the promise images receive their done callbacks this object
    // is deleted - removing the GrBackendTexture from VRAM.
    // Note that while the DDLs are being created in the threads, the PromiseImageHelper holds
    // a ref on all the PromiseImageCallbackContexts. However, once all the threads are done
    // it drops all of its refs (via "reset").
    class PromiseImageCallbackContext : public SkRefCnt {
    public:
        PromiseImageCallbackContext(GrContext* context) : fContext(context) {}

        ~PromiseImageCallbackContext();

        void setBackendTexture(const GrBackendTexture& backendTexture) {
            fBackendTexture = backendTexture;
        }

        const GrBackendTexture& backendTexture() const { return fBackendTexture; }

        const GrCaps* caps() const;

    private:
        GrContext*       fContext;
        GrBackendTexture fBackendTexture;

        typedef SkRefCnt INHERITED;
    };

    // This is the information extracted into this class from the parsing of the skp file.
    // Once it has all been uploaded to the GPU and distributed to the promise images, it
    // is all dropped via "reset".
    class PromiseImageInfo {
    public:
        int                                fIndex;                // index in the 'fImageInfo' array
        uint32_t                           fOriginalUniqueID;     // original ID for deduping
        SkBitmap                           fBitmap;               // CPU-side cache of the contents
        sk_sp<PromiseImageCallbackContext> fCallbackContext;
    };

    // This stack-based context allows each thread to re-inflate the image indices into
    // promise images while still using the same GrBackendTexture.
    struct PerRecorderContext {
        SkDeferredDisplayListRecorder* fRecorder;
        const DDLPromiseImageHelper*   fHelper;
        SkTArray<sk_sp<SkImage>>*      fPromiseImages;
    };

    static void PromiseImageFulfillProc(void* textureContext, GrBackendTexture* outTexture) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        SkASSERT(callbackContext->backendTexture().isValid());
        *outTexture = callbackContext->backendTexture();
    }

    static void PromiseImageReleaseProc(void* textureContext) {
#ifdef SK_DEBUG
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        SkASSERT(callbackContext->backendTexture().isValid());
#endif
    }

    static void PromiseImageDoneProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        callbackContext->unref();
    }

    static sk_sp<SkImage> PromiseImageCreator(const void* rawData, size_t length, void* ctxIn);

    bool isValidID(int id) const { return id >= 0 && id < fImageInfo.count(); }
    const PromiseImageInfo& getInfo(int id) const { return fImageInfo[id]; }

    // returns -1 if not found
    int findImage(SkImage* image) const;

    // returns -1 on failure
    int addImage(SkImage* image);

    // returns -1 on failure
    int findOrDefineImage(SkImage* image);

    SkTArray<PromiseImageInfo> fImageInfo;
};

#endif
