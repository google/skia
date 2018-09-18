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
#include "SkCachedData.h"
#include "SkYUVSizeInfo.h"

class GrContext;
class SkCachedData;
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
//       are done, the PromiseImageCallbackContext is freed and its GrBackendTexture removed
//       from VRAM
//
// Note: if DDLs are going to be replayed multiple times, the reset call can be delayed until
// all the replaying is complete. This will pin the GrBackendTextures in VRAM.
class DDLPromiseImageHelper {
public:
    DDLPromiseImageHelper() { }
    ~DDLPromiseImageHelper();

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
    // This class acts as a proxy for the GrBackendTextures representing an image.
    // Whenever a promise image is created for the image, the promise image receives a ref to
    // this object. Once all the promise images receive their done callbacks this object
    // is deleted - removing the GrBackendTextures from VRAM.
    // Note that while the DDLs are being created in the threads, the PromiseImageHelper holds
    // a ref on all the PromiseImageCallbackContexts. However, once all the threads are done
    // it drops all of its refs (via "reset").
    class PromiseImageCallbackContext : public SkRefCnt {
    public:
        PromiseImageCallbackContext(GrContext* context, bool isYUV)
                : fContext(context)
                , fIsYUV(isYUV) {
        }

        ~PromiseImageCallbackContext();

        bool isYUV() const { return fIsYUV; }

        void setBackendTexture(int index, const GrBackendTexture& backendTexture) {
            SkASSERT(index >= 0 && index < (fIsYUV ? 3 : 1));
            SkASSERT(!fBackendTextures[index].isValid());
            fBackendTextures[index] = backendTexture;
        }

        const GrBackendTexture& backendTexture(int index) const {
            SkASSERT(index >= 0 && index < (fIsYUV ? 3 : 1));
            return fBackendTextures[index];
        }

        const GrCaps* caps() const;

    private:
        GrContext*       fContext;
        bool             fIsYUV;
        GrBackendTexture fBackendTextures[3];

        typedef SkRefCnt INHERITED;
    };

    // This is the information extracted into this class from the parsing of the skp file.
    // Once it has all been uploaded to the GPU and distributed to the promise images, it
    // is all dropped via "reset".
    class PromiseImageInfo {
    public:
        PromiseImageInfo();
        ~PromiseImageInfo();

        int                                fIndex;                // index in the 'fImageInfo' array
        uint32_t                           fOriginalUniqueID1;     // original ID for deduping

        // CPU-side cache of a normal SkImage's contents
        SkBitmap                           fBitmap1;

        // CPU-side cache of a YUV SkImage's contents
        sk_sp<SkCachedData>                fYUVData;       // when !null, this is a YUV image
        SkYUVSizeInfo                      fYUVSizeInfo;
        SkYUVColorSpace                    fYUVColorSpace;
        const void*                        fYUVPlanes[3];

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
        SkASSERT(callbackContext->backendTexture(0).isValid());
        *outTexture = callbackContext->backendTexture(0);
    }

    static void PromiseImageFulfillProc(void* textureContext, GrBackendTexture* outTextures[3]) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        SkASSERT(callbackContext->backendTexture(0).isValid());
        SkASSERT(callbackContext->backendTexture(1).isValid());
        SkASSERT(callbackContext->backendTexture(2).isValid());
        *outTextures[0] = callbackContext->backendTexture(0);
        *outTextures[1] = callbackContext->backendTexture(1);
        *outTextures[2] = callbackContext->backendTexture(2);
    }

    static void PromiseImageReleaseProc(void* textureContext) {
#ifdef SK_DEBUG
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        SkASSERT(callbackContext->backendTexture(0).isValid());
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
