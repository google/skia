
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
#include "SkDebugger.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"

#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"

class SkGLWidget : public QGLWidget {
Q_OBJECT

public:
    SkGLWidget(SkDebugger* debugger);

    ~SkGLWidget();

    void draw() {
        this->updateGL();
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
    SkDebugger* fDebugger;
    GrPlatformRenderTargetDesc getDesc(int w, int h);
};

#endif /* SKGLWIDGET_H_ */
