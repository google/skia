
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGLWidget.h"

SkGLWidget::SkGLWidget(QWidget* parent) : QGLWidget(parent) {
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
    fTransform.set(0,0);
    fScaleFactor = 1.0;
    fIndex = 0;
    fDebugCanvas = NULL;
}

SkGLWidget::~SkGLWidget() {
    SkSafeUnref(fCurIntf);
    SkSafeUnref(fCurContext);
    SkSafeUnref(fGpuDevice);
}

void SkGLWidget::initializeGL() {
    fCurIntf = GrGLCreateNativeInterface();
    fCurContext = GrContext::Create(kOpenGL_Shaders_GrEngine, (GrPlatform3DContext) fCurIntf);
    GrRenderTarget* curRenderTarget = fCurContext->createPlatformRenderTarget(getDesc(this->width(), this->height()));
    fGpuDevice = new SkGpuDevice(fCurContext, curRenderTarget);
    curRenderTarget->unref();

    glClearColor(1, 1, 1, 0);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void SkGLWidget::resizeGL(int w, int h) {
    GrRenderTarget* curRenderTarget = fCurContext->createPlatformRenderTarget(getDesc(w,h));
    SkSafeUnref(fGpuDevice);
    fGpuDevice = new SkGpuDevice(fCurContext, curRenderTarget);
    drawTo(fIndex);
}

void SkGLWidget::paintGL() {
    glClearColor(1, 1, 1, 0);
    SkCanvas canvas(fGpuDevice);
    canvas.translate(fTransform.fX, fTransform.fY);
    if(fScaleFactor < 0) {
        canvas.scale((1.0 / -fScaleFactor),(1.0 / -fScaleFactor));
    } else if (fScaleFactor > 0) {
        canvas.scale(fScaleFactor, fScaleFactor);
    }
    // TODO(chudy): Remove bitmap arguement.
    fDebugCanvas->drawTo(&canvas, fIndex+1, NULL);
    canvas.flush();
}

GrPlatformRenderTargetDesc SkGLWidget::getDesc(int w, int h) {
    GrPlatformRenderTargetDesc desc;
    desc.fWidth = SkScalarRound(this->width());
    desc.fHeight = SkScalarRound(this->height());
    desc.fConfig = kSkia8888_PM_GrPixelConfig;
    GR_GL_GetIntegerv(fCurIntf, GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(fCurIntf, GR_GL_STENCIL_BITS, &desc.fStencilBits);
    GrGLint buffer;
    GR_GL_GetIntegerv(fCurIntf, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    return desc;
}
