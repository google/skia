
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkRasterWidget.h"

SkRasterWidget::SkRasterWidget(SkDebugger *debugger) : QWidget() {
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 800, 800);
    fBitmap.allocPixels();
    fBitmap.eraseColor(0);
    fDevice = new SkDevice(fBitmap);
    fDebugger = debugger;
    fCanvas = new SkCanvas(fDevice);
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
}

SkRasterWidget::~SkRasterWidget() {
    SkSafeUnref(fCanvas);
    SkSafeUnref(fDevice);
}

void SkRasterWidget::resizeEvent(QResizeEvent* event) {
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, event->size().width(), event->size().height());
    fBitmap.allocPixels();
    SkSafeUnref(fCanvas);
    SkSafeUnref(fDevice);
    fDevice = new SkDevice(fBitmap);
    fCanvas = new SkCanvas(fDevice);
    fDebugger->resize(event->size().width(), event->size().height());
    this->update();
}

void SkRasterWidget::paintEvent(QPaintEvent* event) {
    if (!this->isHidden()) {
        fDebugger->draw(fCanvas);
        QPainter painter(this);
        QStyleOption opt;
        opt.init(this);

        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

        QPoint origin(0,0);
        QImage image((uchar *)fBitmap.getPixels(), fBitmap.width(),
                fBitmap.height(), QImage::Format_ARGB32_Premultiplied);

        painter.drawImage(origin, image);
        painter.end();
        emit drawComplete();
    }
}
