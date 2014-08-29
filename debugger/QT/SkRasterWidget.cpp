
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterWidget.h"

SkRasterWidget::SkRasterWidget(SkDebugger *debugger) : QWidget() {
    fBitmap.allocN32Pixels(800, 800);
    fBitmap.eraseColor(SK_ColorTRANSPARENT);
    fDevice = new SkBitmapDevice(fBitmap);
    fDebugger = debugger;
    fCanvas = new SkCanvas(fDevice);
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
}

SkRasterWidget::~SkRasterWidget() {
    SkSafeUnref(fCanvas);
    SkSafeUnref(fDevice);
}

void SkRasterWidget::resizeEvent(QResizeEvent* event) {
    fBitmap.allocN32Pixels(event->size().width(), event->size().height());
    fBitmap.eraseColor(SK_ColorTRANSPARENT);
    SkSafeUnref(fCanvas);
    SkSafeUnref(fDevice);
    fDevice = new SkBitmapDevice(fBitmap);
    fCanvas = new SkCanvas(fDevice);
    fDebugger->setWindowSize(event->size().width(), event->size().height());
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

#if SK_R32_SHIFT == 0
        painter.drawImage(origin, image.rgbSwapped());
#else
        painter.drawImage(origin, image);
#endif
        painter.end();
        emit drawComplete();
    }
}
