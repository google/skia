/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPrepareCallback_DEFINED
#define GrPrepareCallback_DEFINED

#include "SkAutoPixmapStorage.h"
#include "SkRefCnt.h"
#include "SkSemaphore.h"

#include "GrOpFlushState.h"

class GrTextureProxy;

/**
 * GrDeferredProxyUploader assists with threaded generation of textures. Currently used by both
 * software clip masks, and the software path renderer. The calling code typically needs to store
 * some additional data (T) for use on the worker thread. GrTDeferredProxyUploader allows storing
 * such data. A worker thread uses that data to populate fPixels. This object's operator() is
 * invoked at flush time, and handles scheduling the texture upload.
 */
class GrDeferredProxyUploader : public SkNoncopyable {
public:
    GrDeferredProxyUploader() : fScheduledUpload(false), fWaited(false) {}

    virtual ~GrDeferredProxyUploader() {
        if (!fWaited) {
            // This can happen if our owning op list fails to instantiate (so it never prepares)
            fPixelsReady.wait();
        }
    }

    void operator()(GrOpFlushState* flushState, GrTextureProxy* proxy) {
        if (fScheduledUpload) {
            // Multiple references to the owning proxy may have caused us to already execute
            return;
        }

        auto uploadMask = [this, proxy](GrDrawOp::WritePixelsFn& writePixelsFn) {
            this->fPixelsReady.wait();
            this->fWaited = true;
            // If the worker thread was unable to allocate pixels, this check will fail, and we'll
            // end up drawing with an uninitialized mask texture, but at least we won't crash.
            if (this->fPixels.addr()) {
                writePixelsFn(proxy, 0, 0, this->fPixels.width(), this->fPixels.height(),
                              proxy->config(), this->fPixels.addr(), this->fPixels.rowBytes());
                // Free this memory immediately, so it can be recycled. This avoids memory pressure
                // when there is a large amount of threaded work still running during flush.
                this->fPixels.reset();
            }
            // Upload has finished, so tell the proxy to release this GrDeferredProxyUploader
            proxy->resetDeferredUploader();
        };
        flushState->addASAPUpload(std::move(uploadMask));
        fScheduledUpload = true;
    }

    SkAutoPixmapStorage* getPixels() { return &fPixels; }
    SkSemaphore* getSemaphore() { return &fPixelsReady; }

private:
    SkAutoPixmapStorage fPixels;
    SkSemaphore fPixelsReady;
    bool fScheduledUpload;
    bool fWaited;
};

template <typename T>
class GrTDeferredProxyUploader : public GrDeferredProxyUploader {
public:
    template <typename... Args>
    GrTDeferredProxyUploader(Args&&... args)
        : fData(skstd::make_unique<T>(std::forward<Args>(args)...)) {
    }

    T& data() { return *fData; }
    void freeData() {
        fData.reset();
    }

private:
    std::unique_ptr<T> fData;
};

#endif
