
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvasWidget.h"

SkCanvasWidget::SkCanvasWidget(QWidget* parent,
        SkDebugger* debugger) : QWidget(parent)
    , fHorizontalLayout(this)
    , fRasterWidget(debugger)
    , fGLWidget(debugger)
{

    fDebugger = debugger;

    fHorizontalLayout.setSpacing(6);
    fHorizontalLayout.setContentsMargins(0,0,0,0);
    fRasterWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fGLWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);

    fHorizontalLayout.addWidget(&fRasterWidget);
    fHorizontalLayout.addWidget(&fGLWidget);

    fPreviousPoint.set(0,0);
    fUserOffset.set(0,0);
    fUserScaleFactor = 1.0;

    setWidgetVisibility(kGPU_WidgetType, true);
    connect(&fRasterWidget, SIGNAL(drawComplete()),
            this->parentWidget(), SLOT(drawComplete()));
}

SkCanvasWidget::~SkCanvasWidget() {}

void SkCanvasWidget::drawTo(int index) {
    fDebugger->setIndex(index);
    fRasterWidget.draw();
    fGLWidget.draw();
    emit commandChanged(fDebugger->index());
}

void SkCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    SkIPoint eventPoint = SkIPoint::Make(event->globalX(), event->globalY());
    fUserOffset += eventPoint - fPreviousPoint;
    fPreviousPoint = eventPoint;
    fDebugger->setUserOffset(fUserOffset);
    drawTo(fDebugger->index());
}

void SkCanvasWidget::mousePressEvent(QMouseEvent* event) {
    fPreviousPoint.set(event->globalX(), event->globalY());
    emit hitChanged(fDebugger->getCommandAtPoint(event->x(), event->y(),
            fDebugger->index()));
}

void SkCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    resetWidgetTransform();
}

void SkCanvasWidget::resetWidgetTransform() {
    fUserOffset.set(0,0);
    fUserScaleFactor = 1.0;
    fDebugger->setUserOffset(fUserOffset);
    fDebugger->setUserScale(fUserScaleFactor);
    emit scaleFactorChanged(fUserScaleFactor);
    drawTo(fDebugger->index());
}

void SkCanvasWidget::setWidgetVisibility(WidgetType type, bool isHidden) {
    if (type == kRaster_8888_WidgetType) {
        fRasterWidget.setHidden(isHidden);
    } else if (type == kGPU_WidgetType) {
        fGLWidget.setHidden(isHidden);
    }
}

void SkCanvasWidget::zoom(float zoomIncrement) {
    fUserScaleFactor += zoomIncrement;

    /* The range of the fUserScaleFactor crosses over the range -1,0,1 frequently.
    * Based on the code below, -1 and 1 both scale the image to it's original
    * size we do the following to never have a registered wheel scroll
    * not effect the fUserScaleFactor. */
    if (fUserScaleFactor == 0) {
        fUserScaleFactor = 2 * zoomIncrement;
    }
    emit scaleFactorChanged(fUserScaleFactor);
    fDebugger->setUserScale(fUserScaleFactor);
    drawTo(fDebugger->index());
}
