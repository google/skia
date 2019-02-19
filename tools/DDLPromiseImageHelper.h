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

class Foo {
public:
    using PromiseImageTextureContext = SkDeferredDisplayListRecorder::PromiseImageTextureContext;
    using PromiseImageTextureFulfillProc = SkDeferredDisplayListRecorder::PromiseImageTextureFulfillProc;
    using PromiseImageTextureReleaseProc = SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc;
    using PromiseImageTextureDoneProc = SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc;
    using DelayReleaseCallback = SkDeferredDisplayListRecorder::DelayReleaseCallback;

    GrBackendApi backend() const { return GrBackendApi::kOpenGL; }

    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
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
                                      DelayReleaseCallback delayReleaseCallback);

    sk_sp<SkImage> makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
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
                                          DelayReleaseCallback delayReleaseCallback);

private:
};

// This class consolidates tracking & extraction of the original image data from an skp,
// the creation of promise images, the upload of the images to the GPU and the fulfillment
// of the promise images.
//
// The way this works is:
//    1) the original skp is converted to SkData and all its image info is extracted into this
//       class and only indices into this class are left in the SkData (via deflateSKP)
//
//    2) Prior to threaded DDL creation, a promise image is created for each image that was
//        extracted from the skp (via createPromiseImages)
//
//    3) A thread is then spawned to upload all the images to the GPU (via uploadAllToGPU).
//
//    4) While the images are being uploaded in one thread each tile thread reinflates the SkData
//       into its own copy of the SkPicture replacing all the indices w/ the
//       promise images generated in step 2 (via reinflateSKP).
//
//    5) This class is then reset - dropping all of its refs on the PromiseImageCallbackContexts
//
//    Each done callback unrefs its PromiseImageCallbackContext so, once all the promise images
//       are done, the PromiseImageCallbackContext is freed and its GrBackendTexture removed
//       from VRAM
//
// Note: if DDLs are going to be replayed multiple times, the reset call can be delayed until
// all the replaying is complete. This will pin the GrBackendTextures in VRAM.
class DDLPromiseImageHelper {
public:
    using DelayReleaseCallback = SkDeferredDisplayListRecorder::DelayReleaseCallback;
    DDLPromiseImageHelper(DelayReleaseCallback delayReleaseCallback = DelayReleaseCallback::kNo)
            : fDelayReleaseCallback(delayReleaseCallback) {}
    ~DDLPromiseImageHelper();

    // Convert the SkPicture into SkData replacing all the SkImages with an index.
    sk_sp<SkData> deflateSKP(const SkPicture* inputPicture);

    void createPromiseImages(Foo*);

    void uploadAllToGPU(GrContext* context);

    // Change the backing store texture for half the images. (Must ensure all fulfilled images are
    // released before calling this.).
    void replaceEveryOtherPromiseTexture(GrContext*);

    // reinflate a deflated SKP, replacing all the indices with promise images.
    sk_sp<SkPicture> reinflateSKP(SkData* compressedPicture) const;

    // Remove this class' refs on the PromiseImageCallbackContexts
    void reset() { fImageInfo.reset(); }

private:
    // This class acts as a proxy for a GrBackendTexture that backs a promise image.
    // When an image is upload to the GPU one or more of these objects are created and the
    // matching promiseImage is given refs to them.
    // Once a promise image receives its done callback this object is deleted - removing
    // the GrBackendTexture from VRAM.
    // Note that, while the DDLs are being created in the threads, the PromiseImageHelper holds
    // a ref on all the PromiseImageCallbackContexts. However, once all the threads are done,
    // it drops all of its refs (via "reset").
    class PromiseImageCallbackContext : public SkRefCnt {
    public:
        PromiseImageCallbackContext(GrContext* context, bool foo) : fContext(context) {}

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

    // This is the per image information extracted into this class from the parsing of the skp file.
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

#if 1
        void setCallbackContext1(int index, sk_sp<PromiseImageCallbackContext> callbackContext) {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            fCallbackContexts[index] = callbackContext;
        }
        PromiseImageCallbackContext* callbackContext1(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index].get();
        }
        sk_sp<PromiseImageCallbackContext> refCallbackContext1(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index];
        }

        const SkPromiseImageTexture* promiseTexture1(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVASizeInfo::kMaxCount : 1));
            return fCallbackContexts[index]->promiseImageTexture();
        }
#endif

        void setNormalBitmap(const SkBitmap& bm) {
            fBitmap = bm;
            fChannelsPerPlane[0] = fBitmap.colorType();
        }

        void setYUVData(sk_sp<SkCachedData> yuvData,
                        SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                        SkYUVColorSpace cs) {
            fYUVData = yuvData;
            memcpy(fYUVAIndices, yuvaIndices, sizeof(fYUVAIndices));
            fYUVColorSpace = cs;
        }
        void addYUVPlane(int index, const SkImageInfo& ii,
                         const void* plane, size_t widthBytes, int numChannels) {
            SkASSERT(this->isYUV());
            SkASSERT(index >= 0 && index < SkYUVASizeInfo::kMaxCount);
            fYUVPlanes[index].reset(ii, plane, widthBytes);
            fChannelsPerPlane[index] = numChannels;
        }

        sk_sp<SkImage> promiseImage() const { return fPromiseImage1; }
        void createPromiseImage(Foo*, DelayReleaseCallback);

        int numChannelsPerPlane(int index) { return fChannelsPerPlane[index]; }

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

        int                                fNumPlanes = 0;
        int                                fChannelsPerPlane[SkYUVASizeInfo::kMaxCount];

        // Up to SkYUVASizeInfo::kMaxCount for a YUVA image. Only one for a normal image.
        sk_sp<PromiseImageCallbackContext> fCallbackContexts[SkYUVASizeInfo::kMaxCount];

        sk_sp<SkImage>                     fPromiseImage1;
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

    static sk_sp<SkImage> FindOrCreatePromiseImage(const void* rawData, size_t length, void* ctxIn);

    bool isValidID(int id) const { return id >= 0 && id < fImageInfo.count(); }
    const PromiseImageInfo& getInfo(int id) const { return fImageInfo[id]; }
    void uploadImage(GrContext*, PromiseImageInfo*);

    // returns -1 if not found
    int findImage(SkImage* image) const;

    // returns -1 on failure
    int addImage(SkImage* image);

    // returns -1 on failure
    int findOrDefineImage(SkImage* image);

    DelayReleaseCallback fDelayReleaseCallback;
    SkTArray<PromiseImageInfo> fImageInfo;
};

#endif
