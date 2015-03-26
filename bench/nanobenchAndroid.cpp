/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "nanobenchAndroid.h"

#include "AnimationContext.h"
#include "IContextFactory.h"
#include "SkiaCanvasProxy.h"
#include "android/rect.h"
#include "android/native_window.h"
#include "renderthread/TimeLord.h"

namespace {

/**
 * Helper class for setting up android::uirenderer::renderthread::RenderProxy.
 */
class ContextFactory : public android::uirenderer::IContextFactory {
public:
    android::uirenderer::AnimationContext* createAnimationContext
        (android::uirenderer::renderthread::TimeLord& clock) override {
        return new android::uirenderer::AnimationContext(clock);
    }
};

}

HWUITarget::HWUITarget(const Config& c, Benchmark* bench) : Target(c) { }

void HWUITarget::setup() {
    this->proxy->fence();
}

SkCanvas* HWUITarget::beginTiming(SkCanvas* canvas) {
    this->renderer->prepare();
    this->renderer->clipRect(0, 0, this->size.width(), this->size.height(),
                               SkRegion::Op::kReplace_Op);
    SkCanvas* targetCanvas = this->renderer->asSkCanvas();
    if (targetCanvas) {
        this->fc.reset(targetCanvas);
        canvas = &this->fc;
        // This might minimally distort timing, but canvas isn't valid outside the timer.
        canvas->clear(SK_ColorWHITE);
        }
    return canvas;
}

void HWUITarget::endTiming() {
    this->renderer->finish();
    this->rootNode->setStagingDisplayList(this->renderer->finishRecording());
    this->proxy->syncAndDrawFrame();
    // Surprisingly, calling this->proxy->fence() here appears to make no difference to
    // the timings we record.
}

void HWUITarget::fence() {
    this->proxy->fence();
}

bool HWUITarget::needsFrameTiming() const {
    return true;
}

bool HWUITarget::init(SkImageInfo info, Benchmark* bench) {
    // extracted from DMSrcSinkAndroid.cpp's HWUISink::draw()
    size.set(bench->getSize().x(), bench->getSize().y());
    android::BufferQueue::createBufferQueue(&this->producer, &this->consumer);
    this->cpuConsumer = new android::CpuConsumer(this->consumer, 1);
    this->cpuConsumer->setName(android::String8("SkiaBenchmarkClient"));
    this->cpuConsumer->setDefaultBufferSize(size.width(), size.height());
    this->androidSurface = new android::Surface(this->producer);
    native_window_set_buffers_dimensions(this->androidSurface.get(),
                                         size.width(), size.height());
    native_window_set_buffers_format(this->androidSurface.get(),
                                     android::PIXEL_FORMAT_RGBA_8888);
    native_window_set_usage(this->androidSurface.get(), GRALLOC_USAGE_SW_READ_OFTEN |
                                           GRALLOC_USAGE_SW_WRITE_NEVER |
                                           GRALLOC_USAGE_HW_RENDER);
    this->rootNode.reset(new android::uirenderer::RenderNode());
    this->rootNode->incStrong(nullptr);
    this->rootNode->mutateStagingProperties().setLeftTopRightBottom
        (0, 0, size.width(), size.height());
    this->rootNode->mutateStagingProperties().setClipToBounds(false);
    this->rootNode->setPropertyFieldsDirty(android::uirenderer::RenderNode::GENERIC);
    ContextFactory factory;
    this->proxy.reset
        (new android::uirenderer::renderthread::RenderProxy(false, this->rootNode, &factory));
    this->proxy->loadSystemProperties();
    this->proxy->initialize(this->androidSurface.get());
    float lightX = size.width() / 2.0f;
    android::uirenderer::Vector3 lightVector { lightX, -200.0f, 800.0f };
    this->proxy->setup(size.width(), size.height(), lightVector, 800.0f,
                         255 * 0.075f, 255 * 0.15f);
    this->renderer.reset(new android::uirenderer::DisplayListRenderer());
    this->renderer->setViewport(size.width(), size.height());

    // Since we have no SkSurface for HWUI, other parts of the code base have to
    // explicitly work around the fact that it may be invalid / have no SkCanvas.

    return true;
}

bool HWUITarget::capturePixels(SkBitmap* bmp) {
    SkImageInfo destinationConfig =
        SkImageInfo::Make(this->size.width(), this->size.height(),
                          kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bmp->allocPixels(destinationConfig);
    sk_memset32((uint32_t*) bmp->getPixels(), SK_ColorRED,
                this->size.width() * this->size.height());

    android::CpuConsumer::LockedBuffer nativeBuffer;
    android::status_t retval = this->cpuConsumer->lockNextBuffer(&nativeBuffer);
    if (retval == android::BAD_VALUE) {
        SkDebugf("write_canvas_png() got no buffer; returning transparent");
        // No buffer ready to read - commonly triggered by dm sending us
        // a no-op source, or calling code that doesn't do anything on this
        // backend.
        bmp->eraseColor(SK_ColorTRANSPARENT);
        return false;
    } else if (retval) {
        SkDebugf("Failed to lock buffer to read pixels: %d.", retval);
        return false;
    }

    // Move the pixels into the destination SkBitmap

    SK_ALWAYSBREAK(nativeBuffer.format == android::PIXEL_FORMAT_RGBA_8888 &&
                   "Native buffer not RGBA!");
    SkImageInfo nativeConfig =
        SkImageInfo::Make(nativeBuffer.width, nativeBuffer.height,
                          kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Android stride is in pixels, Skia stride is in bytes
    SkBitmap nativeWrapper;
    bool success =
        nativeWrapper.installPixels(nativeConfig, nativeBuffer.data, nativeBuffer.stride * 4);
    if (!success) {
        SkDebugf("Failed to wrap HWUI buffer in a SkBitmap");
        return false;
    }

    SK_ALWAYSBREAK(bmp->colorType() == kRGBA_8888_SkColorType &&
                   "Destination buffer not RGBA!");
    success =
        nativeWrapper.readPixels(destinationConfig, bmp->getPixels(), bmp->rowBytes(), 0, 0);
    if (!success) {
        SkDebugf("Failed to extract pixels from HWUI buffer");
        return false;
    }

    this->cpuConsumer->unlockBuffer(nativeBuffer);

    return true;
}


