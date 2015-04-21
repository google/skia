/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHwuiRenderer_DEFINED
#define SkHwuiRenderer_DEFINED

#include "DisplayListCanvas.h"
#include "RenderNode.h"
#include "SkTypes.h"
#include "gui/CpuConsumer.h"
#include "gui/IGraphicBufferConsumer.h"
#include "gui/IGraphicBufferProducer.h"
#include "gui/Surface.h"
#include "renderthread/RenderProxy.h"

class SkBitmap;

struct SkHwuiRenderer {
    SkAutoTDelete<android::uirenderer::RenderNode> rootNode;
    SkAutoTDelete<android::uirenderer::renderthread::RenderProxy> proxy;
    SkAutoTDelete<android::uirenderer::DisplayListCanvas> canvas;
    android::sp<android::IGraphicBufferProducer> producer;
    android::sp<android::IGraphicBufferConsumer> consumer;
    android::sp<android::CpuConsumer> cpuConsumer;
    android::sp<android::Surface> androidSurface;
    SkISize size;

    void initialize(SkISize size);

    /// Returns a canvas to draw into.
    SkCanvas* prepareToDraw();

    void finishDrawing();

    bool capturePixels(SkBitmap* bmp);
};

#endif  // SkHwuiRenderer_DEFINED
