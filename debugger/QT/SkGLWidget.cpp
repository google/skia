
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
}

SkGLWidget::~SkGLWidget() {
}

void SkGLWidget::setSampleCount(int sampleCount) {
    QGLFormat currentFormat = format();
    currentFormat.setSampleBuffers(sampleCount > 0);
    currentFormat.setSamples(sampleCount);
    setFormat(currentFormat);
}

void SkGLWidget::initializeGL() {
    if (!fCurIntf) {
        fCurIntf.reset(GrGLCreateNativeInterface());
    }
    if (!fCurIntf) {
        return;
    }
    // The call may come multiple times, for example after setSampleCount().  The QGLContext will be
    // different, but we do not have a mechanism to catch the destroying of QGLContext, so that
    // proper resource cleanup could be made.
    if (fCurContext) {
        fCurContext->abandonContext();
    }

    fGpuSurface = nullptr;
    fCanvas = nullptr;

    fCurContext.reset(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext) fCurIntf.get()));
}

void SkGLWidget::createRenderTarget() {
    if (!fCurContext) {
        return;
    }

    glDisable(GL_SCISSOR_TEST);
    glStencilMask(0xffffffff);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    fCurContext->resetContext();

    GrBackendRenderTargetDesc desc = this->getDesc(this->width(), this->height());
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;

    fGpuSurface = SkSurface::MakeFromBackendRenderTarget(fCurContext.get(), desc, nullptr);
    fCanvas = fGpuSurface->getCanvas();
}

void SkGLWidget::resizeGL(int w, int h) {
    SkASSERT(w == this->width() && h == this->height());
    this->createRenderTarget();
}

void SkGLWidget::paintGL() {
    if (!this->isHidden() && fCanvas) {
        fCurContext->resetContext();
        fDebugger->draw(fCanvas);
        // TODO(chudy): Implement an optional flush button in Gui.
        fCanvas->flush();
        Q_EMIT drawComplete();
    }
}

GrBackendRenderTargetDesc SkGLWidget::getDesc(int w, int h) {
    GrBackendRenderTargetDesc desc;
    desc.fWidth = SkScalarRoundToInt(this->width());
    desc.fHeight = SkScalarRoundToInt(this->height());
    desc.fConfig = kSkia8888_GrPixelConfig;
    GR_GL_GetIntegerv(fCurIntf.get(), GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(fCurIntf.get(), GR_GL_STENCIL_BITS, &desc.fStencilBits);
    GrGLint buffer;
    GR_GL_GetIntegerv(fCurIntf.get(), GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    return desc;
}

#endif
