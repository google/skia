/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMSrcSink.h"
#include "DMSrcSinkAndroid.h"

#include "AnimationContext.h"
#include "DisplayListRenderer.h"
#include "IContextFactory.h"
#include "RenderNode.h"
#include "SkAndroidSDKCanvas.h"
#include "SkCanvas.h"
#include "SkiaCanvasProxy.h"
#include "SkTLazy.h"
#include "SkMaskFilter.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "android/rect.h"
#include "android/native_window.h"
#include "gui/BufferQueue.h"
#include "gui/CpuConsumer.h"
#include "gui/IGraphicBufferConsumer.h"
#include "gui/IGraphicBufferProducer.h"
#include "gui/Surface.h"
#include "renderthread/RenderProxy.h"
#include "renderthread/TimeLord.h"

/* These functions are only compiled in the Android Framework. */

namespace {

/**
 * Helper class for setting up android::uirenderer::renderthread::RenderProxy.
 */
class ContextFactory : public android::uirenderer::IContextFactory {
public:
    android::uirenderer::AnimationContext* createAnimationContext
        (android::uirenderer::renderthread::TimeLord& clock) SK_OVERRIDE {
        return new android::uirenderer::AnimationContext(clock);
    }
};

}  // namespace

namespace DM {

Error HWUISink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString*) const {
    // Do all setup in this function because we don't know the size
    // for the RenderNode and RenderProxy during the constructor.
    // In practice this doesn't seem too expensive.
    const SkISize size = src.size();

    // Based on android::SurfaceTexture_init()
    android::sp<android::IGraphicBufferProducer> producer;
    android::sp<android::IGraphicBufferConsumer> consumer;
    android::BufferQueue::createBufferQueue(&producer, &consumer);

    // Consumer setup

    android::sp<android::CpuConsumer> cpuConsumer =
        new android::CpuConsumer(consumer, 1);
    cpuConsumer->setName(android::String8("SkiaTestClient"));
    cpuConsumer->setDefaultBufferSize(size.width(), size.height());

    // Producer setup

    android::sp<android::Surface> surface = new android::Surface(producer);
    native_window_set_buffers_dimensions(surface.get(), size.width(), size.height());
    native_window_set_buffers_format(surface.get(), android::PIXEL_FORMAT_RGBA_8888);
    native_window_set_usage(surface.get(), GRALLOC_USAGE_SW_READ_OFTEN |
                                           GRALLOC_USAGE_SW_WRITE_NEVER |
                                           GRALLOC_USAGE_HW_RENDER);

    // RenderNode setup based on hwui/tests/main.cpp:TreeContentAnimation
    SkAutoTDelete<android::uirenderer::RenderNode> rootNode
        (new android::uirenderer::RenderNode());
    rootNode->incStrong(nullptr);

    // Values set here won't be applied until the framework has called
    // RenderNode::pushStagingPropertiesChanges() during RenderProxy::syncAndDrawFrame().
    rootNode->mutateStagingProperties().setLeftTopRightBottom(0, 0, size.width(), size.height());
    rootNode->setPropertyFieldsDirty(android::uirenderer::RenderNode::X |
                                     android::uirenderer::RenderNode::Y);
    rootNode->mutateStagingProperties().setClipToBounds(false);
    rootNode->setPropertyFieldsDirty(android::uirenderer::RenderNode::GENERIC);

    // RenderProxy setup based on hwui/tests/main.cpp:TreeContentAnimation
    ContextFactory factory;
    SkAutoTDelete<android::uirenderer::renderthread::RenderProxy> proxy
        (new android::uirenderer::renderthread::RenderProxy(false, rootNode, &factory));
    proxy->loadSystemProperties();

    proxy->initialize(surface.get());

    float lightX = size.width() / 2.0f;
    android::uirenderer::Vector3 lightVector { lightX, -200.0f, 800.0f };
    proxy->setup(size.width(), size.height(), lightVector, 800.0f, 255 * 0.075f, 255 * 0.15f);

    // Do the draw

    SkAutoTDelete<android::uirenderer::DisplayListRenderer> renderer
        (new android::uirenderer::DisplayListRenderer());
    renderer->setViewport(size.width(), size.height());
    renderer->prepare();
    renderer->clipRect(0, 0, size.width(), size.height(), SkRegion::Op::kReplace_Op);

    Error err = src.draw(renderer->asSkCanvas());
    if (!err.isEmpty()) {
        return err;
    }

    renderer->finish();
    rootNode->setStagingDisplayList(renderer->finishRecording());

    proxy->syncAndDrawFrame();
    proxy->fence();

    // Capture pixels

    SkImageInfo destinationConfig =
        SkImageInfo::Make(size.width(), size.height(),
                          kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    dst->allocPixels(destinationConfig);
    sk_memset32((uint32_t*) dst->getPixels(), SK_ColorRED, size.width() * size.height());

    android::CpuConsumer::LockedBuffer nativeBuffer;
    android::status_t retval = cpuConsumer->lockNextBuffer(&nativeBuffer);
    if (retval == android::BAD_VALUE) {
        SkDebugf("HWUISink::draw() got no buffer; returning transparent");
        // No buffer ready to read - commonly triggered by dm sending us
        // a no-op source, or calling code that doesn't do anything on this
        // backend.
        dst->eraseColor(SK_ColorTRANSPARENT);
        return "";
    } else if (retval) {
        return SkStringPrintf("Failed to lock buffer to read pixels: %d.", retval);
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
        return "Failed to wrap HWUI buffer in a SkBitmap";
    }

    SK_ALWAYSBREAK(dst->colorType() == kRGBA_8888_SkColorType &&
                   "Destination buffer not RGBA!");
    success =
        nativeWrapper.readPixels(destinationConfig, dst->getPixels(), dst->rowBytes(), 0, 0);
    if (!success) {
        return "Failed to extract pixels from HWUI buffer";
    }

    cpuConsumer->unlockBuffer(nativeBuffer);
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaAndroidSDK::ViaAndroidSDK(Sink* sink) : fSink(sink) { }

Error ViaAndroidSDK::draw(const Src& src,
                          SkBitmap* bitmap,
                          SkWStream* stream,
                          SkString* log) const {
    struct ProxySrc : public Src {
        const Src& fSrc;
        ProxySrc(const Src& src)
            : fSrc(src) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            // Pass through HWUI's upper layers to get operational transforms
            SkAutoTDelete<android::Canvas> ac (android::Canvas::create_canvas(canvas));
            SkAutoTUnref<android::uirenderer::SkiaCanvasProxy> scProxy
                (new android::uirenderer::SkiaCanvasProxy(ac));

            // Pass through another proxy to get paint transforms
            SkAndroidSDKCanvas fc;
            fc.reset(scProxy);

            fSrc.draw(&fc);

            return "";
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }
    } proxy(src);

    return fSink->draw(proxy, bitmap, stream, log);
}

}  // namespace DM
