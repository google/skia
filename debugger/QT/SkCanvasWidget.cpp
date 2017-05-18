
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvasWidget.h"
#include <QtGui>

SkCanvasWidget::SkCanvasWidget(QWidget* parent,
        SkDebugger* debugger) : QWidget(parent)
    , fHorizontalLayout(this)
    , fRasterWidget(debugger)
#if SK_SUPPORT_GPU
    , fGLWidget(debugger)
#endif
{

    fDebugger = debugger;

    fHorizontalLayout.setSpacing(6);
    fHorizontalLayout.setContentsMargins(0,0,0,0);
    fRasterWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
#if SK_SUPPORT_GPU
    fGLWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
#endif

    fHorizontalLayout.addWidget(&fRasterWidget);
#if SK_SUPPORT_GPU
    fHorizontalLayout.addWidget(&fGLWidget);
#endif

    fPreviousPoint.set(0,0);
    fUserMatrix.reset();

#if SK_SUPPORT_GPU
    setWidgetVisibility(kGPU_WidgetType, true);
#endif
    connect(&fRasterWidget, SIGNAL(drawComplete()), this->parentWidget(), SLOT(drawComplete()));
#if SK_SUPPORT_GPU
    connect(&fGLWidget, SIGNAL(drawComplete()), this->parentWidget(), SLOT(drawComplete()));
#endif
}

SkCanvasWidget::~SkCanvasWidget() {}

void SkCanvasWidget::drawTo(int index) {
    fDebugger->setIndex(index);
    fRasterWidget.updateImage();
#if SK_SUPPORT_GPU
    fGLWidget.updateImage();
#endif
    Q_EMIT commandChanged(fDebugger->index());
}

void SkCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    SkIPoint eventPoint = SkIPoint::Make(event->globalX(), event->globalY());
    SkIPoint eventOffset = eventPoint - fPreviousPoint;
    fPreviousPoint = eventPoint;
    fUserMatrix.postTranslate(eventOffset.fX, eventOffset.fY);
    fDebugger->setUserMatrix(fUserMatrix);
    drawTo(fDebugger->index());
}

void SkCanvasWidget::mousePressEvent(QMouseEvent* event) {
    fPreviousPoint.set(event->globalX(), event->globalY());
    Q_EMIT hitChanged(fDebugger->getCommandAtPoint(event->x(), event->y(),
            fDebugger->index()));
}

void SkCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (modifiers.testFlag(Qt::ControlModifier)) {
        snapWidgetTransform();
    } else {
        resetWidgetTransform();
    }
}

#define ZOOM_FACTOR (1.25f)

void SkCanvasWidget::wheelEvent(QWheelEvent* event) {
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (modifiers.testFlag(Qt::ControlModifier)) {
        zoom(event->delta() > 0 ? ZOOM_FACTOR : (1.0f / ZOOM_FACTOR), event->x(), event->y());
    } else {
        if (Qt::Horizontal == event->orientation()) {
            fUserMatrix.postTranslate(event->delta(), 0.0f);
        } else {
            fUserMatrix.postTranslate(0.0f, event->delta());
        }
        fDebugger->setUserMatrix(fUserMatrix);
        drawTo(fDebugger->index());
    }
}

void SkCanvasWidget::zoom(int zoomCommand) {
    zoom(kIn_ZoomCommand == zoomCommand ? ZOOM_FACTOR : (1.0f / ZOOM_FACTOR),
         this->size().width() / 2, this->size().height() / 2);
}

void SkCanvasWidget::snapWidgetTransform() {
    double x, y;
    modf(fUserMatrix.getTranslateX(), &x);
    modf(fUserMatrix.getTranslateY(), &y);
    fUserMatrix[SkMatrix::kMTransX] = x;
    fUserMatrix[SkMatrix::kMTransY] = y;
    fDebugger->setUserMatrix(fUserMatrix);
    drawTo(fDebugger->index());
}

void SkCanvasWidget::resetWidgetTransform() {
    fUserMatrix.reset();
    fDebugger->setUserMatrix(fUserMatrix);
    Q_EMIT scaleFactorChanged(fUserMatrix.getScaleX());
    drawTo(fDebugger->index());
}

void SkCanvasWidget::setWidgetVisibility(WidgetType type, bool isHidden) {
    if (type == kRaster_8888_WidgetType) {
        fRasterWidget.setHidden(isHidden);
    }
#if SK_SUPPORT_GPU
    else if (type == kGPU_WidgetType) {
        fGLWidget.setHidden(isHidden);
    }
#endif
}

#if SK_SUPPORT_GPU
void SkCanvasWidget::setGLSampleCount(int sampleCount)
{
    fGLWidget.setSampleCount(sampleCount);
}
#endif

void SkCanvasWidget::zoom(float scale, int px, int py) {
    fUserMatrix.postScale(scale, scale, px, py);
    Q_EMIT scaleFactorChanged(fUserMatrix.getScaleX());
    fDebugger->setUserMatrix(fUserMatrix);
    drawTo(fDebugger->index());
}
