/*
 * Copyright 2015 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "HelloWorld.h"

#include "gl/GrGLInterface.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkGr.h"

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

HelloWorldWindow::HelloWorldWindow(void* hwnd)
    : INHERITED(hwnd) {
    fType = kGPU_DeviceType;
    fRenderTarget = NULL;
    fRotationAngle = 0;
    this->setTitle();
    this->setUpBackend();
}

HelloWorldWindow::~HelloWorldWindow() {
    tearDownBackend();
}

void HelloWorldWindow::tearDownBackend() {
    SkSafeUnref(fContext);
    fContext = NULL;

    SkSafeUnref(fInterface);
    fInterface = NULL;

    SkSafeUnref(fRenderTarget);
    fRenderTarget = NULL;

    INHERITED::detach();
}

void HelloWorldWindow::setTitle() {
    SkString title("Hello World ");
    title.appendf(fType == kRaster_DeviceType ? "raster" : "opengl");
    INHERITED::setTitle(title.c_str());
}

bool HelloWorldWindow::setUpBackend() {
    this->setColorType(kRGBA_8888_SkColorType);
    this->setVisibleP(true);
    this->setClipToBounds(false);

    bool result = attach(kNativeGL_BackEndType, 0 /*msaa*/, &fAttachmentInfo);
    if (false == result) {
        SkDebugf("Not possible to create backend.\n");
        detach();
        return false;
    }

    fInterface = GrGLCreateNativeInterface();

    SkASSERT(NULL != fInterface);

    fContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fInterface);
    SkASSERT(NULL != fContext);

    this->setUpRenderTarget();
    return true;
}

void HelloWorldWindow::setUpRenderTarget() {
    SkSafeUnref(fRenderTarget);
    fRenderTarget = this->renderTarget(fAttachmentInfo, fInterface, fContext);
}

void HelloWorldWindow::drawContents(SkCanvas* canvas) {
    // Clear background
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    // Draw a rectangle with red paint
    SkRect rect = {
            10, 10,
            128, 128
    };
    canvas->drawRect(rect, paint);

    // Set up a linear gradient and draw a circle
    {
        SkPoint linearPoints[] = {
                {0, 0},
                {300, 300}
        };
        SkColor linearColors[] = {SK_ColorGREEN, SK_ColorBLACK};

        SkShader* shader = SkGradientShader::CreateLinear(
                linearPoints, linearColors, NULL, 2,
                SkShader::kMirror_TileMode);
        SkAutoUnref shader_deleter(shader);

        paint.setShader(shader);
        paint.setFlags(SkPaint::kAntiAlias_Flag);

        canvas->drawCircle(200, 200, 64, paint);

        // Detach shader
        paint.setShader(NULL);
    }

    // Draw a message with a nice black paint.
    paint.setFlags(
            SkPaint::kAntiAlias_Flag |
            SkPaint::kSubpixelText_Flag |  // ... avoid waggly text when rotating.
            SkPaint::kUnderlineText_Flag);
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(20);

    canvas->save();

    static const char message[] = "Hello World";

    // Translate and rotate
    canvas->translate(300, 300);
    fRotationAngle += 0.2f;
    if (fRotationAngle > 360) {
        fRotationAngle -= 360;
    }
    canvas->rotate(fRotationAngle);

    // Draw the text:
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->restore();
}

void HelloWorldWindow::draw(SkCanvas* canvas) {
    drawContents(canvas);
    // in case we have queued drawing calls
    fContext->flush();
    // Invalidate the window to force a redraw. Poor man's animation mechanism.
    this->inval(NULL);

    if (kRaster_DeviceType == fType) {
        // need to send the raster bits to the (gpu) window
        SkImage* snap = fSurface->newImageSnapshot();
        size_t rowBytes;
        SkImageInfo info;
        const void* pixels = snap->peekPixels(&info, &rowBytes);
        fRenderTarget->writePixels(0, 0, snap->width(), snap->height(),
                                        SkImageInfo2GrPixelConfig(info.colorType(),
                                                                info.alphaType(),
                                                                info.profileType()),
                                        pixels,
                                        rowBytes,
                                        GrContext::kFlushWrites_PixelOp);
        SkSafeUnref(snap);
    }
    INHERITED::present();
}

void HelloWorldWindow::onSizeChange() {
    setUpRenderTarget();
}

bool HelloWorldWindow::onHandleChar(SkUnichar unichar) {
    if (' ' == unichar) {
        fType = fType == kRaster_DeviceType ? kGPU_DeviceType: kRaster_DeviceType;
        tearDownBackend();
        setUpBackend();
        this->setTitle();
        this->inval(NULL);
    }
    return true;
}

SkOSWindow* create_sk_window(void* hwnd, int , char** ) {
    return new HelloWorldWindow(hwnd);
}
