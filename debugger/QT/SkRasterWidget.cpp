
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkRasterWidget.h"

SkRasterWidget::SkRasterWidget(QWidget* parent) : QWidget(parent) {
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 800, 800);
    fBitmap.allocPixels();
    fBitmap.eraseColor(0);
    fTransform.set(0,0);
    fScaleFactor = 1.0;
    fIndex = 0;
    fDevice = NULL;
    fDebugCanvas = NULL;
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
}

SkRasterWidget::~SkRasterWidget() {
    delete fDevice;
}

void SkRasterWidget::resizeEvent(QResizeEvent* event) {
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, event->size().width(), event->size().height());
    fBitmap.allocPixels();
    delete fDevice;
    fDevice = new SkDevice(fBitmap);
    this->update();
}

void SkRasterWidget::paintEvent(QPaintEvent* event) {
    if (fDebugCanvas) {
        fBitmap.eraseColor(0);
        SkCanvas canvas(fDevice);
        canvas.translate(fTransform.fX, fTransform.fY);
        if (fScaleFactor < 0) {
            canvas.scale((1.0 / -fScaleFactor), (1.0 / -fScaleFactor));
        } else if (fScaleFactor > 0) {
            canvas.scale(fScaleFactor, fScaleFactor);
        }

        fMatrix = canvas.getTotalMatrix();
        fClip = canvas.getTotalClip().getBounds();
        fDebugCanvas->drawTo(&canvas, fIndex);

        QPainter painter(this);
        QStyleOption opt;
        opt.init(this);

        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

        QPoint origin(0,0);
        QImage image((uchar *)fBitmap.getPixels(), fBitmap.width(),
                fBitmap.height(), QImage::Format_ARGB32_Premultiplied);

        painter.drawImage(origin, image);
        painter.end();
    }
}
