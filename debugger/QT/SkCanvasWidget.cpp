
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvasWidget.h"

SkCanvasWidget::SkCanvasWidget() : QWidget()
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
    fTransform.set(0,0);
    fScaleFactor = 1.0;

    setWidgetVisibility(kGPU_WidgetType, true);
    this->setDisabled(true);
}

SkCanvasWidget::~SkCanvasWidget() {
    if (fDebugCanvas) {
        delete fDebugCanvas;
    }
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
    fIndex = fDebugCanvas->getSize();
    fRasterWidget.setDebugCanvas(fDebugCanvas);
    fGLWidget.setDebugCanvas(fDebugCanvas);

    // TODO(chudy): Remove bounds from debug canvas storage.
    fDebugCanvas->setBounds(this->width(), this->height());
}

void SkCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    SkIPoint eventPoint = SkIPoint::Make(event->globalX(), event->globalY());
    fTransform += eventPoint - fPreviousPoint;
    fPreviousPoint = eventPoint;
    updateWidgetTransform(kTranslate);
    drawTo(fIndex);
}

void SkCanvasWidget::mousePressEvent(QMouseEvent* event) {
    fPreviousPoint.set(event->globalX(), event->globalY());
    if (fDebugCanvas) {
        fDebugCanvas->getBoxClass()->setHitPoint(event->x(), event->y());
        fDebugCanvas->isCalculatingHits(true);
        drawTo(fIndex);
        emit hitChanged(fDebugCanvas->getHitBoxPoint());
        fDebugCanvas->isCalculatingHits(false);
    }
}

void SkCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    fTransform.set(0,0);
    fScaleFactor = 1.0;
    emit scaleFactorChanged(fScaleFactor);
    // TODO(chudy): Change to signal / slot mechanism.
    resetWidgetTransform();
    drawTo(fIndex);
}

void SkCanvasWidget::resetWidgetTransform() {
    fTransform.set(0,0);
    fScaleFactor = 1.0;
    updateWidgetTransform(kTranslate);
    updateWidgetTransform(kScale);
}

void SkCanvasWidget::setWidgetVisibility(WidgetType type, bool isHidden) {
    if (type == kRaster_8888_WidgetType) {
        fRasterWidget.setHidden(isHidden);
    } else if (type == kGPU_WidgetType) {
        fGLWidget.setHidden(isHidden);
    }
}

void SkCanvasWidget::updateWidgetTransform(TransformType type) {
    if (type == kTranslate) {
        fRasterWidget.setTranslate(fTransform);
        fGLWidget.setTranslate(fTransform);
    } else if (type == kScale) {
        fRasterWidget.setScale(fScaleFactor);
        fGLWidget.setScale(fScaleFactor);
    }
}

void SkCanvasWidget::zoom(float zoomIncrement) {
    fScaleFactor += zoomIncrement;

    /* The range of the fScaleFactor crosses over the range -1,0,1 frequently.
    * Based on the code below, -1 and 1 both scale the image to it's original
    * size we do the following to never have a registered wheel scroll
    * not effect the fScaleFactor. */
    if (fScaleFactor == 0) {
        fScaleFactor = 2 * zoomIncrement;
    }
    emit scaleFactorChanged(fScaleFactor);
    updateWidgetTransform(kScale);
    drawTo(fIndex);
}
