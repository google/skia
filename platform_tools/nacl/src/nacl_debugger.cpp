
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

#include "SkBase64.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkDebugger.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkString.h"

class SkiaInstance;

// Used by SkDebugf
SkiaInstance* gPluginInstance;

void FlushCallback(void* data, int32_t result);

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
        if (var_message.is_string()) {
            SkString msg(var_message.AsString().c_str());
            if (msg.startsWith("init")) {
            } else if (msg.startsWith("LoadSKP")) {
                size_t startIndex = strlen("LoadSKP");
                size_t dataSize = msg.size()/sizeof(char) - startIndex;
                SkBase64 decodedData;
                decodedData.decode(msg.c_str() + startIndex, dataSize);
                size_t decodedSize = 3 * (dataSize / 4);
                SkDebugf("Got size: %d\n", decodedSize);
                if (!decodedData.getData()) {
                    SkDebugf("Failed to decode SKP\n");
                    return;
                }
                SkMemoryStream pictureStream(decodedData.getData(), decodedSize);
                SkPicture* picture = SkPicture::CreateFromStream(&pictureStream);
                if (NULL == picture) {
                    SkDebugf("Failed to create SKP.\n");
                    return;
                }
                fDebugger.loadPicture(picture);
                picture->unref();

                // Set up the command list.
                PostMessage("ClearCommands");
                for (int i = 0; i < fDebugger.getSize(); ++i) {
                    SkString addCommand("AddCommand:");
                    addCommand.append(fDebugger.getDrawCommandAt(i)->toString());
                    PostMessage(addCommand.c_str());
                }
                PostMessage("UpdateCommands");

                // Set the overview text.
                SkString overviewText;
                fDebugger.getOverviewText(NULL, 0.0, &overviewText, 1);
                overviewText.prepend("SetOverview:");
                PostMessage(overviewText.c_str());

                // Draw the SKP.
                if (!fFlushLoopRunning) {
                    Paint();
                }
            } else if (msg.startsWith("CommandSelected:")) {
                size_t startIndex = strlen("CommandSelected:");
                int index = atoi(msg.c_str() + startIndex);
                fDebugger.setIndex(index);
                if (!fFlushLoopRunning) {
                    Paint();
                }
            } else if (msg.startsWith("Rewind")) {
                fCanvas->clear(SK_ColorWHITE);
                fDebugger.setIndex(0);
                if (!fFlushLoopRunning) {
                    Paint();
                }
            } else if (msg.startsWith("StepBack")) {
                fCanvas->clear(SK_ColorWHITE);
                int currentIndex = fDebugger.index();
                if (currentIndex > 1) {
                    fDebugger.setIndex(currentIndex - 1);
                    if (!fFlushLoopRunning) {
                        Paint();
                    }
                }
            } else if (msg.startsWith("Pause")) {
                // TODO(borenet)
            } else if (msg.startsWith("StepForward")) {
                int currentIndex = fDebugger.index();
                if (currentIndex < fDebugger.getSize() -1) {
                    fDebugger.setIndex(currentIndex + 1);
                    if (!fFlushLoopRunning) {
                        Paint();
                    }
                }
            } else if (msg.startsWith("Play")) {
                fDebugger.setIndex(fDebugger.getSize() - 1);
                if (!fFlushLoopRunning) {
                    Paint();
                }
            }
        }
    }

    void Paint() {
        if (!fImage.is_null()) {
            fDebugger.draw(fCanvas);
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
        const SkImageInfo info = SkImageInfo::MakeN32Premul(fWidth, fHeight);
        fBitmap.installPixels(info, fImage.data(), info.minRowBytes());
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
    SkDebugger fDebugger;

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
