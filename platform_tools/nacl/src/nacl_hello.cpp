
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/var.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkString.h"

class SkiaInstance;

// Used by SkDebugf
SkiaInstance* gPluginInstance;

void FlushCallback(void* data, int32_t result);

static void doDraw(SkCanvas* canvas, const SkPaint& paint, const char text[]) {
    canvas->drawColor(SK_ColorWHITE);
    SkPaint red;
    red.setColor(SK_ColorRED);
    canvas->drawCircle(150.0, 150.0, 100.0, red);
    SkRect bounds;
    canvas->getClipBounds(&bounds);
    canvas->drawText(text, strlen(text),
                     bounds.centerX(), bounds.centerY(),
                     paint);
}

// Skia's subclass of pp::Instance, our interface with the browser.
class SkiaInstance : public pp::Instance {
public:
    explicit SkiaInstance(PP_Instance instance)
        : pp::Instance(instance)
        , fCanvas(NULL)
        , fFlushLoopRunning(false)
        , fFlushPending(false)
    {
        gPluginInstance = this;
        SkGraphics::Init();
    }

    virtual ~SkiaInstance() {
        SkGraphics::Term();
        gPluginInstance = NULL;
    }

    virtual void HandleMessage(const pp::Var& var_message) {
        // Receive a message from javascript.
    }

    void Paint() {
        if (!fImage.is_null()) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setTextSize(SkIntToScalar(30));
            paint.setTextAlign(SkPaint::kCenter_Align);
            doDraw(fCanvas, paint, "Hello");

            fDeviceContext.PaintImageData(fImage, pp::Point(0, 0));
            if (!fFlushPending) {
                fFlushPending = true;
                fDeviceContext.Flush(pp::CompletionCallback(&FlushCallback, this));
            } else {
                SkDebugf("A flush is pending... Skipping flush.\n");
            }
        } else {
            SkDebugf("No pixels to write to!\n");
        }
    }

    virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
        if (position.size().width() == fWidth &&
            position.size().height() == fHeight) {
            return;  // We don't care about the position, only the size.
        }
        fWidth = position.size().width();
        fHeight = position.size().height();
        fDeviceContext = pp::Graphics2D(this, pp::Size(fWidth, fHeight), false);
        if (!BindGraphics(fDeviceContext)) {
            SkDebugf("Couldn't bind the device context\n");
            return;
        }
        fImage = pp::ImageData(this,
                               PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                               pp::Size(fWidth, fHeight), false);
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, fWidth, fHeight);
        fBitmap.setPixels(fImage.data());
        if (fCanvas) {
            delete fCanvas;
        }
        fCanvas = new SkCanvas(fBitmap);
        fCanvas->clear(SK_ColorWHITE);
        if (!fFlushLoopRunning) {
            Paint();
        }
    }

    void OnFlush() {
        fFlushLoopRunning = true;
        fFlushPending = false;
        Paint();
    }

private:
    pp::Graphics2D fDeviceContext;
    pp::ImageData fImage;
    int fWidth;
    int fHeight;

    SkBitmap fBitmap;
    SkCanvas* fCanvas;

    bool fFlushLoopRunning;
    bool fFlushPending;
};

void FlushCallback(void* data, int32_t result) {
    static_cast<SkiaInstance*>(data)->OnFlush();
}

class SkiaModule : public pp::Module {
public:
    SkiaModule() : pp::Module() {}
    virtual ~SkiaModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new SkiaInstance(instance);
    }
};

namespace pp {
Module* CreateModule() {
    return new SkiaModule();
}
}  // namespace pp
