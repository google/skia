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

class GrPrepareCallback : SkNoncopyable {
public:
    virtual ~GrPrepareCallback() {}
    virtual void operator()(GrOpFlushState*) = 0;
};

template <typename T>
class GrMaskUploaderPrepareCallback : public GrPrepareCallback {
public:
    template <typename... Args>
    GrMaskUploaderPrepareCallback(sk_sp<GrTextureProxy> proxy, Args&&... args)
            : fProxy(std::move(proxy))
            , fWaited(false)
            , fData(std::forward<Args>(args)...) {}

    ~GrMaskUploaderPrepareCallback() override {
        if (!fWaited) {
            // This can happen if our owning op list fails to instantiate (so it never prepares)
            fPixelsReady.wait();
        }
    }

    void operator()(GrOpFlushState* flushState) override {
        auto uploadMask = [this](GrDrawOp::WritePixelsFn& writePixelsFn) {
            this->fPixelsReady.wait();
            this->fWaited = true;
            // If the worker thread was unable to allocate pixels, this check will fail, and we'll
            // end up drawing with an uninitialized mask texture, but at least we won't crash.
            if (this->fPixels.addr()) {
                writePixelsFn(this->fProxy.get(), 0, 0,
                              this->fPixels.width(), this->fPixels.height(),
                              kAlpha_8_GrPixelConfig,
                              this->fPixels.addr(), this->fPixels.rowBytes());
                // Free this memory immediately, so it can be recycled. This avoids memory pressure
                // when there is a large amount of threaded work still running during flush.
                this->fPixels.reset();
            }
        };
        flushState->addASAPUpload(std::move(uploadMask));
    }

    SkAutoPixmapStorage* getPixels() { return &fPixels; }
    SkSemaphore* getSemaphore() { return &fPixelsReady; }
    T& data() { return fData; }

private:
    sk_sp<GrTextureProxy> fProxy;
    SkAutoPixmapStorage fPixels;
    SkSemaphore fPixelsReady;
    bool fWaited;

    T fData;
};

#endif
