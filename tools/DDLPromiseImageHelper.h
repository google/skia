/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PromiseImageHelper_DEFINED
#define PromiseImageHelper_DEFINED

#include "GrBackendSurface.h"
#include "SkBitmap.h"
#include "SkCachedData.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkPromiseImageTexture.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkYUVAIndex.h"
#include "SkYUVASizeInfo.h"

class GrContext;
class SkImage;
class SkPicture;
struct SkYUVAIndex;

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
    DDLPromiseImageHelper() = default;
    ~DDLPromiseImageHelper() = default;

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
    // This class acts as a proxy for a GrBackendTexture that is part of an image.
    // Whenever a promise image is created for the image, the promise image receives a ref to
    // potentially several of these objects. Once all the promise images receive their done
    // callbacks this object is deleted - removing the GrBackendTexture from VRAM.
    // Note that while the DDLs are being created in the threads, the PromiseImageHelper holds
    // a ref on all the PromiseImageCallbackContexts. However, once all the threads are done
    // it drops all of its refs (via "reset").
    class PromiseImageCallbackContext : public SkRefCnt {
    public:
        PromiseImageCallbackContext(GrContext* context) : fContext(context) {}

        ~PromiseImageCallbackContext();

        void setBackendTexture(const GrBackendTexture& backendTexture);

        sk_sp<SkPromiseImageTexture> fulfill() {
            SkASSERT(fPromiseImageTexture);
            SkASSERT(fUnreleasedFulfills >= 0);
            ++fUnreleasedFulfills;
            ++fTotalFulfills;
            return fPromiseImageTexture;
        }

        void release() {
            SkASSERT(fUnreleasedFulfills > 0);
            --fUnreleasedFulfills;
            ++fTotalReleases;
        }

        void done() {
            ++fDoneCnt;
            SkASSERT(fDoneCnt <= fNumImages);
        }

        void wasAddedToImage() { fNumImages++; }

        const SkPromiseImageTexture* promiseImageTexture() const {
          return fPromiseImageTexture.get();
        }

    private:
        GrContext* fContext;
        sk_sp<SkPromiseImageTexture> fPromiseImageTexture;
        int fNumImages = 0;
        int fTotalFulfills = 0;
        int fTotalReleases = 0;
        int fUnreleasedFulfills = 0;
        int fDoneCnt = 0;

        typedef SkRefCnt INHERITED;
    };

    // This is the information extracted into this class from the parsing of the skp file.
    // Once it has all been uploaded to the GPU and distributed to the promise images, it
    // is all dropped via "reset".
    class PromiseImageInfo {
    public:
        PromiseImageInfo(int index, uint32_t originalUniqueID, const SkImageInfo& ii)
                : fIndex(index)
                , fOriginalUniqueID(originalUniqueID)
                , fImageInfo(ii) {
        }
        ~PromiseImageInfo() {}

        int index() const { return fIndex; }
        uint32_t originalUniqueID() const { return fOriginalUniqueID; }
        bool isYUV() const { return SkToBool(fYUVData.get()); }

        int overallWidth() const { return fImageInfo.width(); }
        int overallHeight() const { return fImageInfo.height(); }
        SkColorType overallColorType() const { return fImageInfo.colorType(); }
        SkAlphaType overallAlphaType() const { return fImageInfo.alphaType(); }
        sk_sp<SkColorSpace> refOverallColorSpace() const { return fImageInfo.refColorSpace(); }

        SkYUVColorSpace yuvColorSpace() const {
            SkASSERT(this->isYUV());
            return fYUVColorSpace;
        }
        const SkYUVAIndex* yuvaIndices() const {
            SkASSERT(this->isYUV());
            return fYUVAIndices;
        }
        const SkPixmap& yuvPixmap(int index) const {
            SkASSERT(this->isYUV());
            SkASSERT(index >= 0 && index < SkYUVASizeInfo::kMaxCount);
            return fYUVPlanes[index];
        }
        const SkBitmap& normalBitmap() const {
            SkASSERT(!this->isYUV());
            return fBitmap;
        }

        void setCallbackContext(int index, sk_sp<PromiseImageCallbackContext> callbackContext) {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            fCallbackContexts[index] = callbackContext;
        }
        PromiseImageCallbackContext* callbackContext(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index].get();
        }
        sk_sp<PromiseImageCallbackContext> refCallbackContext(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index];
        }

        const SkPromiseImageTexture* promiseTexture(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index]->promiseImageTexture();
        }

        void setNormalBitmap(const SkBitmap& bm) { fBitmap = bm; }

        void setYUVData(sk_sp<SkCachedData> yuvData,
                        SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                        SkYUVColorSpace cs) {
            fYUVData = yuvData;
            memcpy(fYUVAIndices, yuvaIndices, sizeof(fYUVAIndices));
            fYUVColorSpace = cs;
        }
        void addYUVPlane(int index, const SkImageInfo& ii, const void* plane, size_t widthBytes) {
            SkASSERT(this->isYUV());
            SkASSERT(index >= 0 && index < SkYUVASizeInfo::kMaxCount);
            fYUVPlanes[index].reset(ii, plane, widthBytes);
        }

    private:
        const int                          fIndex;                // index in the 'fImageInfo' array
        const uint32_t                     fOriginalUniqueID;     // original ID for deduping

        const SkImageInfo                  fImageInfo;            // info for the overarching image

        // CPU-side cache of a normal SkImage's contents
        SkBitmap                           fBitmap;

        // CPU-side cache of a YUV SkImage's contents
        sk_sp<SkCachedData>                fYUVData;       // when !null, this is a YUV image
        SkYUVColorSpace                    fYUVColorSpace = kJPEG_SkYUVColorSpace;
        SkYUVAIndex                        fYUVAIndices[SkYUVAIndex::kIndexCount];
        SkPixmap                           fYUVPlanes[SkYUVASizeInfo::kMaxCount];

        // Up to SkYUVASizeInfo::kMaxCount for a YUVA image. Only one for a normal image.
        sk_sp<PromiseImageCallbackContext> fCallbackContexts[SkYUVASizeInfo::kMaxCount];
    };

    // This stack-based context allows each thread to re-inflate the image indices into
    // promise images while still using the same GrBackendTexture.
    struct PerRecorderContext {
        SkDeferredDisplayListRecorder* fRecorder;
        const DDLPromiseImageHelper*   fHelper;
        SkTArray<sk_sp<SkImage>>*      fPromiseImages;
    };

    static sk_sp<SkPromiseImageTexture> PromiseImageFulfillProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        return callbackContext->fulfill();
    }

    static void PromiseImageReleaseProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        callbackContext->release();
    }

    static void PromiseImageDoneProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        callbackContext->done();
        callbackContext->unref();
    }

    static sk_sp<SkImage> PromiseImageCreator(const void* rawData, size_t length, void* ctxIn);

    bool isValidID(int id) const { return id >= 0 && id < fImageInfo.count(); }
    const PromiseImageInfo& getInfo(int id) const { return fImageInfo[id]; }
    void uploadImage(GrContext*, PromiseImageInfo*);

    // returns -1 if not found
    int findImage(SkImage* image) const;

    // returns -1 on failure
    int addImage(SkImage* image);

    // returns -1 on failure
    int findOrDefineImage(SkImage* image);

    SkTArray<PromiseImageInfo> fImageInfo;
};

#endif
