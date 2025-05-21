/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkSurface_Base.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkColorSpace.h" // IWYU pragma: keep
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "src/image/SkRescaleAndReadPixels.h"

#include <atomic>
#include <cstdint>
#include <memory>

class GrRecordingContext;
class SkPaint;
class SkSurfaceProps;
namespace skgpu { namespace graphite { class Recorder; } }

SkSurface_Base::SkSurface_Base(int width, int height, const SkSurfaceProps* props)
        : SkSurface(width, height, props) {}

SkSurface_Base::SkSurface_Base(const SkImageInfo& info, const SkSurfaceProps* props)
        : SkSurface(info, props) {}

SkSurface_Base::~SkSurface_Base() {
    // in case the canvas outsurvives us, we null the callback
    if (fCachedCanvas) {
        fCachedCanvas->setSurfaceBase(nullptr);
    }
}

GrRecordingContext* SkSurface_Base::onGetRecordingContext() const { return nullptr; }

skgpu::graphite::Recorder* SkSurface_Base::onGetRecorder() const { return nullptr; }
SkRecorder* SkSurface_Base::onGetBaseRecorder() const { return nullptr; }

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                            const SkSamplingOptions& sampling, const SkPaint* paint) {
    auto image = this->makeImageSnapshot();
    if (image) {
        canvas->drawImage(image.get(), x, y, sampling, paint);
    }
}

void SkSurface_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                 SkIRect origSrcRect,
                                                 SkSurface::RescaleGamma rescaleGamma,
                                                 RescaleMode rescaleMode,
                                                 SkSurface::ReadPixelsCallback callback,
                                                 SkSurface::ReadPixelsContext context) {
    SkBitmap src;
    SkPixmap peek;
    SkIRect srcRect;
    if (this->peekPixels(&peek)) {
        src.installPixels(peek);
        srcRect = origSrcRect;
    } else {
        src.setInfo(this->imageInfo().makeDimensions(origSrcRect.size()));
        src.allocPixels();
        if (!this->readPixels(src, origSrcRect.x(), origSrcRect.y())) {
            callback(context, nullptr);
            return;
        }
        srcRect = SkIRect::MakeSize(src.dimensions());
    }
    return SkRescaleAndReadPixels(src, info, srcRect, rescaleGamma, rescaleMode, callback,
                                  context);
}

void SkSurface_Base::onAsyncRescaleAndReadPixelsYUV420(
        SkYUVColorSpace yuvColorSpace, bool readAlpha, sk_sp<SkColorSpace> dstColorSpace,
        SkIRect srcRect, SkISize dstSize, RescaleGamma rescaleGamma, RescaleMode,
        ReadPixelsCallback callback, ReadPixelsContext context) {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

bool SkSurface_Base::outstandingImageSnapshot() const {
    return fCachedImage && !fCachedImage->unique();
}

bool SkSurface_Base::aboutToDraw(ContentChangeMode mode) {
    this->dirtyGenerationID();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);

    if (fCachedImage) {
        // the surface may need to fork its backend, if its sharing it with
        // the cached image. Note: we only call if there is an outstanding owner
        // on the image (besides us).
        bool unique = fCachedImage->unique();
        if (!unique) {
            if (!this->onCopyOnWrite(mode)) {
                return false;
            }
        }

        // regardless of copy-on-write, we must drop our cached image now, so
        // that the next request will get our new contents.
        fCachedImage.reset();

        if (unique) {
            // Our content isn't held by any image now, so we can consider that content mutable.
            // Raster surfaces need to be told it's safe to consider its pixels mutable again.
            // We make this call after the ->unref() so the subclass can assert there are no images.
            this->onRestoreBackingMutability();
        }
    } else if (kDiscard_ContentChangeMode == mode) {
        this->onDiscard();
    }
    return true;
}

uint32_t SkSurface_Base::newGenerationID() {
    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    static std::atomic<uint32_t> nextID{1};
    return nextID.fetch_add(1, std::memory_order_relaxed);
}

sk_sp<const SkCapabilities> SkSurface_Base::onCapabilities() {
    return SkCapabilities::RasterBackend();
}
