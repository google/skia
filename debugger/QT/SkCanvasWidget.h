
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKCANVASWIDGET_H_
#define SKCANVASWIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include "SkStream.h"
#include "SkRasterWidget.h"
#include "SkGLWidget.h"
#include "SkDebugger.h"

class SkCanvasWidget : public QWidget {
    Q_OBJECT

public:
    SkCanvasWidget(QWidget* parent, SkDebugger* debugger);

    ~SkCanvasWidget();

    enum WidgetType {
        kRaster_8888_WidgetType = 1 << 0,
#if SK_SUPPORT_GPU
        kGPU_WidgetType         = 1 << 1,
#endif
    };

    void drawTo(int index);

    void setWidgetVisibility(WidgetType type, bool isHidden);

#if SK_SUPPORT_GPU
    void setGLSampleCount(int sampleCount);
#endif

    /** Zooms the canvas by scale with the transformation centered at the widget point (px, py). */
    void zoom(float scale, int px, int py);

    void resetWidgetTransform();

    enum ZoomCommandTypes {
        kIn_ZoomCommand,
        kOut_ZoomCommand,
    };
public Q_SLOTS:
    /**
     *  Zooms in or out (see ZoomCommandTypes) by the standard zoom factor
     *  with the transformation centered in the middle of the widget.
     */
    void zoom(int zoomCommand);

Q_SIGNALS:
    void scaleFactorChanged(float newScaleFactor);
    void commandChanged(int newCommand);
    void hitChanged(int hit);

private:
    QHBoxLayout fHorizontalLayout;
    SkRasterWidget fRasterWidget;
#if SK_SUPPORT_GPU
    SkGLWidget fGLWidget;
#endif
    SkDebugger* fDebugger;
    SkIPoint fPreviousPoint;
    SkMatrix fUserMatrix;

    void mouseMoveEvent(QMouseEvent* event);

    void mousePressEvent(QMouseEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent* event);

    void snapWidgetTransform();
};


#endif /* SKCANVASWIDGET_H_ */
