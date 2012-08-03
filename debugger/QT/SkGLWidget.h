
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKGLWIDGET_H_
#define SKGLWIDGET_H_

#include <QtOpenGL/QGLWidget>
#include "SkDebugCanvas.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"

#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"

class SkGLWidget : public QGLWidget {
Q_OBJECT

public:
    SkGLWidget();

    ~SkGLWidget();

    void setDebugCanvas(SkDebugCanvas* debugCanvas) {
        fDebugCanvas = debugCanvas;
        fIndex = debugCanvas->getSize() - 1;
        this->updateGL();
    }

    void drawTo(int index) {
        fIndex = index;
        this->updateGL();
    }

    void setTranslate(SkIPoint translate) {
        fTransform = translate;
    }

    void setScale(float scale) {
        fScaleFactor = scale;
    }

signals:
    void drawComplete();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();


private:
    const GrGLInterface* fCurIntf;
    GrContext* fCurContext;
    SkGpuDevice* fGpuDevice;
    SkCanvas* fCanvas;
    SkDebugCanvas* fDebugCanvas;
    int fIndex;
    SkIPoint fTransform;
    float fScaleFactor;
    GrPlatformRenderTargetDesc getDesc(int w, int h);
};

#endif /* SKGLWIDGET_H_ */
