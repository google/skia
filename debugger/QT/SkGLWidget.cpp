
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGLWidget.h"

#if SK_SUPPORT_GPU

SkGLWidget::SkGLWidget(SkDebugger* debugger) : QGLWidget() {
    fDebugger = debugger;
    fCurIntf = NULL;
    fCurContext = NULL;
    fGpuDevice = NULL;
    fCanvas = NULL;
}

SkGLWidget::~SkGLWidget() {
    SkSafeUnref(fCurIntf);
    SkSafeUnref(fCurContext);
    SkSafeUnref(fGpuDevice);
    SkSafeUnref(fCanvas);
}

void SkGLWidget::setSampleCount(int sampleCount)
{
    QGLFormat currentFormat = format();
    currentFormat.setSampleBuffers(sampleCount > 0);
    currentFormat.setSamples(sampleCount);
    setFormat(currentFormat);
}

void SkGLWidget::initializeGL() {
    fCurIntf = GrGLCreateNativeInterface();
    if (!fCurIntf) {
        return;
    }
    glStencilMask(0xffffffff);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    fCurContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext) fCurIntf);
    GrBackendRenderTargetDesc desc = this->getDesc(this->width(), this->height());
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    GrRenderTarget* curRenderTarget = fCurContext->wrapBackendRenderTarget(desc);
    fGpuDevice = new SkGpuDevice(fCurContext, curRenderTarget);
    fCanvas = new SkCanvas(fGpuDevice);
    curRenderTarget->unref();
}

void SkGLWidget::resizeGL(int w, int h) {
    if (fCurContext) {
        glDisable(GL_SCISSOR_TEST);
        glStencilMask(0xffffffff);
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        fCurContext->resetContext();

        GrBackendRenderTargetDesc desc = this->getDesc(w, h);
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
        GrRenderTarget* curRenderTarget = fCurContext->wrapBackendRenderTarget(desc);
        SkSafeUnref(fGpuDevice);
        SkSafeUnref(fCanvas);
        fGpuDevice = new SkGpuDevice(fCurContext, curRenderTarget);
        fCanvas = new SkCanvas(fGpuDevice);
    }
    fDebugger->resize(w, h);
    draw();
}

void SkGLWidget::paintGL() {
    if (!this->isHidden() && fCanvas) {
        fDebugger->draw(fCanvas);
        // TODO(chudy): Implement an optional flush button in Gui.
        fCanvas->flush();
        emit drawComplete();
    }
}

GrBackendRenderTargetDesc SkGLWidget::getDesc(int w, int h) {
    GrBackendRenderTargetDesc desc;
    desc.fWidth = SkScalarRoundToInt(this->width());
    desc.fHeight = SkScalarRoundToInt(this->height());
    desc.fConfig = kSkia8888_GrPixelConfig;
    GR_GL_GetIntegerv(fCurIntf, GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(fCurIntf, GR_GL_STENCIL_BITS, &desc.fStencilBits);
    GrGLint buffer;
    GR_GL_GetIntegerv(fCurIntf, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    return desc;
}

#endif
