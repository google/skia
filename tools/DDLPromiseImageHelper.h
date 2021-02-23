/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PromiseImageHelper_DEFINED
#define PromiseImageHelper_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkPromiseImageTexture.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/SkTArray.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkTLazy.h"

class GrDirectContext;
class SkImage;
class SkMipmap;
class SkPicture;
class SkTaskGroup;

// This class acts as a proxy for a GrBackendTexture that backs an image.
// Whenever a promise image is created for the image, the promise image receives a ref to
// potentially several of these objects. Once all the promise images receive their done
// callbacks this object is deleted - removing the GrBackendTexture from VRAM.
// Note that while the DDLs are being created in the threads, the PromiseImageHelper holds
// a ref on all the PromiseImageCallbackContexts. However, once all the threads are done
// it drops all of its refs (via "reset").
class PromiseImageCallbackContext : public SkRefCnt {
public:
    PromiseImageCallbackContext(GrDirectContext* direct, GrBackendFormat backendFormat)
            : fContext(direct)
            , fBackendFormat(backendFormat) {}

    ~PromiseImageCallbackContext() override;

    const GrBackendFormat& backendFormat() const { return fBackendFormat; }

    void setBackendTexture(const GrBackendTexture& backendTexture);

    void destroyBackendTexture();

    sk_sp<SkPromiseImageTexture> fulfill() {
        ++fTotalFulfills;
        return fPromiseImageTexture;
    }

    void release() {
        ++fDoneCnt;
        SkASSERT(fDoneCnt <= fNumImages);
    }

    void wasAddedToImage() { fNumImages++; }

    const SkPromiseImageTexture* promiseImageTexture() const {
        return fPromiseImageTexture.get();
    }

    static sk_sp<SkPromiseImageTexture> PromiseImageFulfillProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        return callbackContext->fulfill();
    }

    static void PromiseImageReleaseProc(void* textureContext) {
        auto callbackContext = static_cast<PromiseImageCallbackContext*>(textureContext);
        callbackContext->release();
        callbackContext->unref();
    }

private:
    GrDirectContext*             fContext;
    GrBackendFormat              fBackendFormat;
    sk_sp<SkPromiseImageTexture> fPromiseImageTexture;
    int                          fNumImages = 0;
    int                          fTotalFulfills = 0;
    int                          fDoneCnt = 0;

    using INHERITED = SkRefCnt;
};

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
    DDLPromiseImageHelper(const SkYUVAPixmapInfo::SupportedDataTypes& supportedYUVADataTypes)
            : fSupportedYUVADataTypes(supportedYUVADataTypes) {}
    ~DDLPromiseImageHelper() = default;

    // Convert the SkPicture into SkData replacing all the SkImages with an index.
    sk_sp<SkData> deflateSKP(const SkPicture* inputPicture);

    void createCallbackContexts(GrDirectContext*);

    void uploadAllToGPU(SkTaskGroup*, GrDirectContext*);
    void deleteAllFromGPU(SkTaskGroup*, GrDirectContext*);

    // reinflate a deflated SKP, replacing all the indices with promise images.
    sk_sp<SkPicture> reinflateSKP(sk_sp<GrContextThreadSafeProxy>,
                                  SkData* compressedPicture,
                                  SkTArray<sk_sp<SkImage>>* promiseImages) const;

    // Remove this class' refs on the PromiseImageCallbackContexts
    void reset() { fImageInfo.reset(); }

private:
    // This is the information extracted into this class from the parsing of the skp file.
    // Once it has all been uploaded to the GPU and distributed to the promise images, it
    // is all dropped via "reset".
    class PromiseImageInfo {
    public:
        PromiseImageInfo(int index, uint32_t originalUniqueID, const SkImageInfo& ii);
        PromiseImageInfo(PromiseImageInfo&& other);
        ~PromiseImageInfo();

        int index() const { return fIndex; }
        uint32_t originalUniqueID() const { return fOriginalUniqueID; }
        bool isYUV() const { return fYUVAPixmaps.isValid(); }

        SkISize overallDimensions() const { return fImageInfo.dimensions(); }
        SkColorType overallColorType() const { return fImageInfo.colorType(); }
        SkAlphaType overallAlphaType() const { return fImageInfo.alphaType(); }
        sk_sp<SkColorSpace> refOverallColorSpace() const { return fImageInfo.refColorSpace(); }

        const SkYUVAInfo& yuvaInfo() const { return fYUVAPixmaps.yuvaInfo(); }

        const SkPixmap& yuvPixmap(int index) const {
            SkASSERT(this->isYUV());
            return fYUVAPixmaps.planes()[index];
        }

        const SkBitmap& baseLevel() const {
            SkASSERT(!this->isYUV());
            return fBaseLevel;
        }
        // This returns an array of all the available mipLevels - suitable for passing into
        // createBackendTexture.
        std::unique_ptr<SkPixmap[]> normalMipLevels() const;
        int numMipLevels() const;

        void setCallbackContext(int index, sk_sp<PromiseImageCallbackContext> callbackContext) {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVAInfo::kMaxPlanes : 1));
            fCallbackContexts[index] = callbackContext;
        }
        PromiseImageCallbackContext* callbackContext(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVAInfo::kMaxPlanes : 1));
            return fCallbackContexts[index].get();
        }
        sk_sp<PromiseImageCallbackContext> refCallbackContext(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVAInfo::kMaxPlanes : 1));
            return fCallbackContexts[index];
        }

        GrMipmapped mipMapped(int index) const {
            if (this->isYUV()) {
                return GrMipmapped::kNo;
            }
            return fMipLevels ? GrMipmapped::kYes : GrMipmapped::kNo;
        }
        const GrBackendFormat& backendFormat(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVAInfo::kMaxPlanes : 1));
            return fCallbackContexts[index]->backendFormat();
        }
        const SkPromiseImageTexture* promiseTexture(int index) const {
            SkASSERT(index >= 0 && index < (this->isYUV() ? SkYUVAInfo::kMaxPlanes : 1));
            return fCallbackContexts[index]->promiseImageTexture();
        }

        void setMipLevels(const SkBitmap& baseLevel, std::unique_ptr<SkMipmap> mipLevels);

        /** Takes ownership of the plane data. */
        void setYUVPlanes(SkYUVAPixmaps yuvaPixmaps) { fYUVAPixmaps = std::move(yuvaPixmaps); }

    private:
        const int                          fIndex;                // index in the 'fImageInfo' array
        const uint32_t                     fOriginalUniqueID;     // original ID for deduping

        const SkImageInfo                  fImageInfo;            // info for the overarching image

        // CPU-side cache of a normal SkImage's mipmap levels
        SkBitmap                           fBaseLevel;
        std::unique_ptr<SkMipmap>          fMipLevels;

        // CPU-side cache of a YUV SkImage's contents
        SkYUVAPixmaps                      fYUVAPixmaps;

        // Up to SkYUVASizeInfo::kMaxCount for a YUVA image. Only one for a normal image.
        sk_sp<PromiseImageCallbackContext> fCallbackContexts[SkYUVAInfo::kMaxPlanes];
    };

    struct DeserialImageProcContext {
        sk_sp<GrContextThreadSafeProxy> fThreadSafeProxy;
        const DDLPromiseImageHelper*    fHelper;
        SkTArray<sk_sp<SkImage>>*       fPromiseImages;
    };

    static void CreateBETexturesForPromiseImage(GrDirectContext*, PromiseImageInfo*);
    static void DeleteBETexturesForPromiseImage(PromiseImageInfo*);

    static sk_sp<SkImage> CreatePromiseImages(const void* rawData, size_t length, void* ctxIn);

    bool isValidID(int id) const { return id >= 0 && id < fImageInfo.count(); }
    const PromiseImageInfo& getInfo(int id) const { return fImageInfo[id]; }
    void uploadImage(GrDirectContext*, PromiseImageInfo*);

    // returns -1 if not found
    int findImage(SkImage* image) const;

    // returns -1 on failure
    int addImage(SkImage* image);

    // returns -1 on failure
    int findOrDefineImage(SkImage* image);

    SkYUVAPixmapInfo::SupportedDataTypes fSupportedYUVADataTypes;
    SkTArray<PromiseImageInfo> fImageInfo;
};

#endif
