/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "BaseExample.h"

#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "SkApplication.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

BaseExample::BaseExample(void* hWnd, int argc, char** argv)
    : INHERITED(hWnd) {}

void BaseExample::tearDownBackend() {
  if (kGPU_DeviceType == fType) {
        SkSafeUnref(fContext);
        fContext = NULL;

        SkSafeUnref(fInterface);
        fInterface = NULL;

        SkSafeUnref(fRenderTarget);
        fRenderTarget = NULL;

        detach();
    }
}

bool BaseExample::setupBackend(DeviceType type) {
    fType = type;

    this->setConfig(SkBitmap::kARGB_8888_Config);
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

    setupRenderTarget();

    return true;
}

void BaseExample::setupRenderTarget() {
    GrBackendRenderTargetDesc desc;
    desc.fWidth = SkScalarRound(width());
    desc.fHeight = SkScalarRound(height());
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fSampleCnt = fAttachmentInfo.fSampleCount;
    desc.fStencilBits = fAttachmentInfo.fStencilBits;

    GrGLint buffer;
    GR_GL_GetIntegerv(fInterface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    fRenderTarget = fContext->wrapBackendRenderTarget(desc);

    fContext->setRenderTarget(fRenderTarget);
}

SkCanvas* BaseExample::createCanvas() {
    if (fType == kGPU_DeviceType) {
        if (NULL != fContext && NULL != fRenderTarget) {
            SkAutoTUnref<SkDevice> device(new SkGpuDevice(fContext, fRenderTarget));
            return new SkCanvas(device);
        }
        tearDownBackend();
        setupBackend(kRaster_DeviceType);
    }
    return INHERITED::createCanvas();
}

void BaseExample::draw(SkCanvas* canvas) {
    if (fType == kGPU_DeviceType) {

        SkASSERT(NULL != fContext);
        fContext->flush();
    }
    if (fType == kRaster_DeviceType) {
        // need to send the raster bits to the (gpu) window
        fContext->setRenderTarget(fRenderTarget);
        const SkBitmap& bm = getBitmap();
        fRenderTarget->writePixels(0, 0, bm.width(), bm.height(),
                                      kSkia8888_GrPixelConfig,
                                      bm.getPixels(),
                                      bm.rowBytes());
    }
    INHERITED::present();
}

void BaseExample::onSizeChange() {
    setupRenderTarget();
}

#ifdef SK_BUILD_FOR_WIN
void BaseExample::onHandleInval(const SkIRect& rect) {
    RECT winRect;
    winRect.top = rect.top();
    winRect.bottom = rect.bottom();
    winRect.right = rect.right();
    winRect.left = rect.left();
    InvalidateRect((HWND)this->getHWND(), &winRect, false);
}
#endif
