
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvasWidget.h"

SkCanvasWidget::SkCanvasWidget(QWidget* parent) : QWidget(parent)
    , fHorizontalLayout(this)
{
    fHorizontalLayout.setSpacing(6);
    fHorizontalLayout.setContentsMargins(0,0,0,0);
    fRasterWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fGLWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);

    fHorizontalLayout.addWidget(&fRasterWidget);
    fHorizontalLayout.addWidget(&fGLWidget);
    fDebugCanvas = NULL;

    fIndex = 0;
    fPreviousPoint.set(0,0);
    fUserOffset.set(0,0);
    fUserScaleFactor = 1.0;

    setWidgetVisibility(kGPU_WidgetType, true);
    this->setDisabled(true);
    connect(&fRasterWidget, SIGNAL(drawComplete()),
            this->parentWidget(), SLOT(drawComplete()));
}

SkCanvasWidget::~SkCanvasWidget() {
    delete fDebugCanvas;
}

void SkCanvasWidget::drawTo(int index) {
    fIndex = index;
    if (!fRasterWidget.isHidden()) {
        fRasterWidget.drawTo(index);
    }
    if (!fGLWidget.isHidden()) {
        fGLWidget.drawTo(index);
    }
    emit commandChanged(fIndex);
}

void SkCanvasWidget::loadPicture(QString filename) {
    this->setDisabled(false);
    SkStream* stream = new SkFILEStream(filename.toAscii());
    SkPicture* picture = new SkPicture(stream);

    /* TODO(chudy): Implement function that doesn't require new
     * instantiation of debug canvas. */
    delete fDebugCanvas;
    fDebugCanvas = new SkDebugCanvas(picture->width(), picture->height());

    picture->draw(fDebugCanvas);
    fIndex = fDebugCanvas->getSize() - 1;
    fRasterWidget.setDebugCanvas(fDebugCanvas);
    fGLWidget.setDebugCanvas(fDebugCanvas);
    fDebugCanvas->setBounds(this->width(), this->height());
}

void SkCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    SkIPoint eventPoint = SkIPoint::Make(event->globalX(), event->globalY());
    fUserOffset += eventPoint - fPreviousPoint;
    fPreviousPoint = eventPoint;
    fDebugCanvas->setUserOffset(fUserOffset);
    drawTo(fIndex);
}

void SkCanvasWidget::mousePressEvent(QMouseEvent* event) {
    fPreviousPoint.set(event->globalX(), event->globalY());
    emit hitChanged(fDebugCanvas->getCommandAtPoint(event->x(), event->y(),
            fIndex));
}

void SkCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    resetWidgetTransform();
}

void SkCanvasWidget::resetWidgetTransform() {
    fUserOffset.set(0,0);
    fUserScaleFactor = 1.0;
    fDebugCanvas->setUserOffset(fUserOffset);
    fDebugCanvas->setUserScale(fUserScaleFactor);
    emit scaleFactorChanged(fUserScaleFactor);
    drawTo(fIndex);
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
    fDebugCanvas->setUserScale(fUserScaleFactor);
    drawTo(fIndex);
}
