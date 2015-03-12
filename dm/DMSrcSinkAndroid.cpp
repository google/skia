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

/** Discard SkShaders not exposed by the Android Java API. */

void CheckShader(SkPaint* paint) {
    SkShader* shader = paint->getShader();
    if (!shader) {
        return;
    }

    if (shader->asABitmap(NULL, NULL, NULL) == SkShader::kDefault_BitmapType) {
        return;
    }
    if (shader->asACompose(NULL)) {
        return;
    }
    SkShader::GradientType gtype = shader->asAGradient(NULL);
    if (gtype == SkShader::kLinear_GradientType ||
        gtype == SkShader::kRadial_GradientType ||
        gtype == SkShader::kSweep_GradientType) {
        return;
    }
    paint->setShader(NULL);
}

/** Simplify a paint.  */

void Filter(SkPaint* paint) {

    uint32_t flags = paint->getFlags();
    flags &= ~SkPaint::kLCDRenderText_Flag;
    paint->setFlags(flags);

    // Android doesn't support Xfermodes above kLighten_Mode
    SkXfermode::Mode mode;
    SkXfermode::AsMode(paint->getXfermode(), &mode);
    if (mode > SkXfermode::kLighten_Mode) {
        paint->setXfermode(NULL);
    }

    // Force bilinear scaling or none
    if (paint->getFilterQuality() != kNone_SkFilterQuality) {
        paint->setFilterQuality(kLow_SkFilterQuality);
    }

    CheckShader(paint);

    // Android SDK only supports mode & matrix color filters
    // (and, again, no modes above kLighten_Mode).
    SkColorFilter* cf = paint->getColorFilter();
    if (cf) {
        SkColor color;
        SkXfermode::Mode mode;
        SkScalar srcColorMatrix[20];
        bool isMode = cf->asColorMode(&color, &mode);
        if (isMode && mode > SkXfermode::kLighten_Mode) {
            paint->setColorFilter(
                SkColorFilter::CreateModeFilter(color, SkXfermode::kSrcOver_Mode));
        } else if (!isMode && !cf->asColorMatrix(srcColorMatrix)) {
            paint->setColorFilter(NULL);
        }
    }

    SkPathEffect* pe = paint->getPathEffect();
    if (pe && !pe->exposedInAndroidJavaAPI()) {
        paint->setPathEffect(NULL);
    }

    // TODO: Android doesn't support all the flags that can be passed to
    // blur filters; we need plumbing to get them out.

    paint->setImageFilter(NULL);
    paint->setLooper(NULL);
};

/** SkDrawFilter is likely to be deprecated; this is a proxy
    canvas that does the same thing: alter SkPaint fields.

    onDraw*() functions may have their SkPaint modified, and are then
    passed on to the same function on proxyTarget.

    This still suffers one of the same architectural flaws as SkDrawFilter:
    TextBlob paints are incomplete when filter is called.
*/

#define FILTER(p)             \
    SkPaint filteredPaint(p); \
    Filter(&filteredPaint);

#define FILTER_PTR(p)                          \
    SkTLazy<SkPaint> lazyPaint;                \
    SkPaint* filteredPaint = (SkPaint*) p;     \
    if (p) {                                   \
        filteredPaint = lazyPaint.set(*p);     \
        Filter(filteredPaint);                 \
    }


class FilteringCanvas : public SkCanvas {
public:
    FilteringCanvas(SkCanvas* proxyTarget) : fProxyTarget(proxyTarget) { }

protected:
    void onDrawPaint(const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPaint(filteredPaint);
    }
    void onDrawPoints(PointMode pMode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPoints(pMode, count, pts, filteredPaint);
    }
    void onDrawOval(const SkRect& r, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawOval(r, filteredPaint);
    }
    void onDrawRect(const SkRect& r, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawRect(r, filteredPaint);
    }
    void onDrawRRect(const SkRRect& r, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawRRect(r, filteredPaint);
    }
    void onDrawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPath(path, filteredPaint);
    }
    void onDrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                      const SkPaint* paint) SK_OVERRIDE {
        FILTER_PTR(paint);
        fProxyTarget->drawBitmap(bitmap, left, top, filteredPaint);
    }
    void onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, DrawBitmapRectFlags flags) SK_OVERRIDE {
        FILTER_PTR(paint);
        fProxyTarget->drawBitmapRectToRect(bitmap, src, dst, filteredPaint, flags);
    }
    void onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                          const SkRect& dst, const SkPaint* paint) SK_OVERRIDE {
        FILTER_PTR(paint);
        fProxyTarget->drawBitmapNine(bitmap, center, dst, filteredPaint);
    }
    void onDrawSprite(const SkBitmap& bitmap, int left, int top,
                      const SkPaint* paint) SK_OVERRIDE {
        FILTER_PTR(paint);
        fProxyTarget->drawSprite(bitmap, left, top, filteredPaint);
    }
    void onDrawVertices(VertexMode vMode, int vertexCount, const SkPoint vertices[],
                        const SkPoint texs[], const SkColor colors[], SkXfermode* xMode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawVertices(vMode, vertexCount, vertices, texs, colors,
                                   xMode, indices, indexCount, filteredPaint);
    }

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                      const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawDRRect(outer, inner, filteredPaint);
    }

    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawText(text, byteLength, x, y, filteredPaint);
    }
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPosText(text, byteLength, pos, filteredPaint);
    }
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPosTextH(text, byteLength, xpos, constY, filteredPaint);
    }
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawTextOnPath(text, byteLength, path, matrix, filteredPaint);
    }
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawTextBlob(blob, x, y, filteredPaint);
    }

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) SK_OVERRIDE {
        FILTER(paint);
        fProxyTarget->drawPatch(cubics, colors, texCoords, xmode, filteredPaint);
    }

protected:
    SkCanvas* fProxyTarget;
};

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
            FilteringCanvas fc(scProxy);

            fSrc.draw(&fc);

            return "";
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }
    } proxy(src);

    return fSink->draw(proxy, bitmap, stream, log);
}

}  // namespace DM
