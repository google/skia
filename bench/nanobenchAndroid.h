/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef nanobenchAndroid_DEFINED
#define nanobenchAndroid_DEFINED

#include "DisplayListRenderer.h"
#include "RenderNode.h"
#include "SkAndroidSDKCanvas.h"
#include "gui/BufferQueue.h"
#include "gui/CpuConsumer.h"
#include "gui/IGraphicBufferConsumer.h"
#include "gui/IGraphicBufferProducer.h"
#include "gui/Surface.h"
#include "renderthread/RenderProxy.h"

#include "nanobench.h"

struct HWUITarget : public Target {
    explicit HWUITarget(const Config& c, Benchmark* bench);

    SkAutoTDelete<android::uirenderer::RenderNode> rootNode;
    SkAutoTDelete<android::uirenderer::renderthread::RenderProxy> proxy;
    SkAutoTDelete<android::uirenderer::DisplayListRenderer> renderer;
    android::sp<android::IGraphicBufferProducer> producer;
    android::sp<android::IGraphicBufferConsumer> consumer;
    android::sp<android::CpuConsumer> cpuConsumer;
    android::sp<android::Surface> androidSurface;
    SkISize size;
    SkAndroidSDKCanvas fc;

    void setup() override;
    SkCanvas* beginTiming(SkCanvas* canvas) override;
    void endTiming() override;
    void fence() override;
    bool needsFrameTiming() const override;

    /// Returns false if initialization fails
    bool init(SkImageInfo info, Benchmark* bench) override;
    bool capturePixels(SkBitmap* bmp) override;
};



#endif  // nanobenchAndroid_DEFINED
