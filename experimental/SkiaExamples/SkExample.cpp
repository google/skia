/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "SkExample.h"

#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "SkApplication.h"
#include "SkCommandLineFlags.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"

DEFINE_string2(match, m, NULL, "[~][^]substring[$] [...] of test name to run.\n" \
                               "Multiple matches may be separated by spaces.\n" \
                               "~ causes a matching test to always be skipped\n" \
                               "^ requires the start of the test to match\n" \
                               "$ requires the end of the test to match\n" \
                               "^ and $ requires an exact match\n" \
                               "If a test does not match any list entry,\n" \
                               "it is skipped unless some list entry starts with ~");

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

SkExampleWindow::SkExampleWindow(void* hwnd)
    : INHERITED(hwnd) {
    fRegistry = SkExample::Registry::Head();
    fCurrExample = fRegistry->factory()(this);

    if (FLAGS_match.count()) {
        // Start with the a matching sample if possible.
        bool found = this->findNextMatch();
        if (!found) {
            SkDebugf("No matching SkExample found.\n");
        }
    }
}

void SkExampleWindow::tearDownBackend() {
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

bool SkExampleWindow::setupBackend(DeviceType type) {
    fType = type;

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

    setupRenderTarget();

    return true;
}

void SkExampleWindow::setupRenderTarget() {
    GrBackendRenderTargetDesc desc;
    desc.fWidth = SkScalarRoundToInt(width());
    desc.fHeight = SkScalarRoundToInt(height());
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

SkCanvas* SkExampleWindow::createCanvas() {
    if (fType == kGPU_DeviceType) {
        if (NULL != fContext && NULL != fRenderTarget) {
            SkAutoTUnref<SkBaseDevice> device(new SkGpuDevice(fContext, fRenderTarget));
            return new SkCanvas(device);
        }
        tearDownBackend();
        setupBackend(kRaster_DeviceType);
    }
    return INHERITED::createCanvas();
}

void SkExampleWindow::draw(SkCanvas* canvas) {
    if (NULL != fCurrExample) {
        fCurrExample->draw(canvas);
    }
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

void SkExampleWindow::onSizeChange() {
    setupRenderTarget();
}

#ifdef SK_BUILD_FOR_WIN
void SkExampleWindow::onHandleInval(const SkIRect& rect) {
    RECT winRect;
    winRect.top = rect.top();
    winRect.bottom = rect.bottom();
    winRect.right = rect.right();
    winRect.left = rect.left();
    InvalidateRect((HWND)this->getHWND(), &winRect, false);
}
#endif

bool SkExampleWindow::findNextMatch() {
    bool found = false;
    // Avoid infinite loop by knowing where we started.
    const SkExample::Registry* begin = fRegistry;
    while (!found) {
        fRegistry = fRegistry->next();
        if (NULL == fRegistry) {  // Reached the end of the registered samples. GOTO head.
            fRegistry = SkExample::Registry::Head();
        }
        SkExample* next = fRegistry->factory()(this);
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, next->getName().c_str())) {
            fCurrExample = next;
            found = true;
        }
        if (begin == fRegistry) {  // We looped through every sample without finding anything.
            break;
        }
    }
    return found;
}

bool SkExampleWindow::onHandleChar(SkUnichar unichar) {
    if ('n' == unichar) {
        bool found = findNextMatch();
        if (!found) {
            SkDebugf("No SkExample that matches your query\n");
        }
    }
    return true;
}

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return new SkExampleWindow(hwnd);
}
