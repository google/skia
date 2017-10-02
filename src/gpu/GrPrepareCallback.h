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
 * An instance of any class derived from GrPrepareCallback can be passed to
 * GrOpList::addPrepareCallback. At flush time, all callbacks (on op lists being flushed) will be
 * invoked (via operator()). Note that the callback receives the GrOpFlushState, so it can trigger
 * ASAP uploads (similar to an Op's onPrepare).
 *
 * All callbacks are invoked at the beginning of flush, before prepare is called.
 */
class GrPrepareCallback : SkNoncopyable {
public:
    virtual ~GrPrepareCallback() {}
    virtual void operator()(GrOpFlushState*) = 0;
};

/**
 * GrMaskUploaderPrepareCallback assists with threaded generation of mask textures. Currently used
 * by both software clip masks, and the software path renderer. The calling code typically needs
 * to store some additional data (T) for use on the worker thread. That payload is accessed by the
 * worker thread to populate the mask in fPixels (using GrSWMaskHelper). This callback's operator()
 * handles scheduling the texture upload at flush time.
 */
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
