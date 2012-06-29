
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPicture.h"
#include "SkStream.h"
#include "SkCanvasWidget.h"
#include <iostream>

SkCanvasWidget::SkCanvasWidget(QWidget *parent) :
    QWidget(parent) {

    fBitmap = new SkBitmap();
    fBitmap->setConfig(SkBitmap::kARGB_8888_Config, 800, 800);
    fBitmap->allocPixels();
    fBitmap->eraseColor(0);
    fDevice = new SkDevice(*fBitmap);
    fCanvas = new SkCanvas(fDevice);
    fDebugCanvas = new SkDebugCanvas();
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
}

SkCanvasWidget::~SkCanvasWidget() {}

void SkCanvasWidget::drawTo(int index) {
    delete fCanvas;
    fCanvas = new SkCanvas(fDevice);
    fDebugCanvas->drawTo(fCanvas, index+1);
    this->update();
}

void SkCanvasWidget::loadPicture(QString filename) {
    SkStream *stream = new SkFILEStream(filename.toAscii());
    SkPicture *picture = new SkPicture(stream);

    delete fDebugCanvas;
    fDebugCanvas = new SkDebugCanvas();

    picture->draw(fDebugCanvas);
    fDebugCanvas->draw(fCanvas);

    /* NOTE(chudy): This was a test to determine if the canvas size is accurately
     * saved in the bounds of the recorded picture. It is not. Everyone of the
     * sample GM images is 1000x1000. Even the one that claims it is
     * 2048x2048.
    std::cout << "Width: " << picture->width();
    std::cout << " Height: " <<  picture->height() << std::endl; */

    /* NOTE(chudy): Updated style sheet without a background specified to
     * draw over our SkCanvas. */
    this->setStyleSheet("QWidget {border: 1px solid #cccccc;}");
    this->update();
}

void SkCanvasWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);

    if (fBitmap) {
        const QPoint origin(0,0);
        QImage image((uchar *)fBitmap->getPixels(), fBitmap->width(), fBitmap->height(), QImage::Format_ARGB32_Premultiplied);
        painter.drawImage(origin,image);
    }

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    painter.end();
}
