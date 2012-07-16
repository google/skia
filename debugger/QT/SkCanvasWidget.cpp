
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPicture.h"
#include "SkStream.h"
#include "SkCanvasWidget.h"
#include "SkColor.h"

SkCanvasWidget::SkCanvasWidget(QWidget *parent) :
    QWidget(parent) {

    /* TODO(chudy): The 800x800 is a default number. Change it to be
     * set dynamically. Also need to pass size into debugCanvas for current
     * command filter. */
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 800, 800);
    fBitmap.allocPixels();
    fBitmap.eraseColor(0);

    /* TODO(chudy): Add fCanvas, fDevice to the stack. The bitmap being
     * cleared does get rid of fDevices link to it. See if there's someway around
     * it so that we don't have to delete both canvas and device to clear out
     * the bitmap. */
    fDevice = new SkDevice(fBitmap);
    fCanvas = new SkCanvas(fDevice);
    fDebugCanvas = new SkDebugCanvas();

    fScaleFactor = 1.0;
    fIndex = 0;
    fPreviousPoint.set(0,0);
    fTransform.set(0,0);

    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");
}

SkCanvasWidget::~SkCanvasWidget() {
    delete fCanvas;
    delete fDevice;
    delete fDebugCanvas;
}

void SkCanvasWidget::resizeEvent(QResizeEvent* event) {
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, event->size().width(), event->size().height());
    fBitmap.allocPixels();
    fBitmap.eraseColor(0);

    delete fCanvas;
    delete fDevice;

    fDevice = new SkDevice(fBitmap);
    fCanvas = new SkCanvas(fDevice);
    fDebugCanvas->setBounds(event->size().width(), event->size().height());
    fDebugCanvas->drawTo(fCanvas, fIndex+1, &fBitmap);
    this->update();
}

void SkCanvasWidget::drawTo(int fIndex) {
    delete fCanvas;
    fCanvas = new SkCanvas(fDevice);
    fBitmap.eraseColor(0);
    fCanvas->translate(fTransform.fX, fTransform.fY);
    if(fScaleFactor < 0) {
        fCanvas->scale((1.0 / -fScaleFactor),(1.0 / -fScaleFactor));
    } else if (fScaleFactor > 0) {
        fCanvas->scale(fScaleFactor, fScaleFactor);
    }

    emit commandChanged(fIndex);
    fDebugCanvas->drawTo(fCanvas, fIndex+1, &fBitmap);
    this->update();
    this->fIndex = fIndex;
}

void SkCanvasWidget::loadPicture(QString filename) {
    SkStream *stream = new SkFILEStream(filename.toAscii());
    SkPicture *picture = new SkPicture(stream);

    delete fDebugCanvas;
    fDebugCanvas = new SkDebugCanvas();
    fDebugCanvas->setBounds(this->width(), this->height());

    picture->draw(fDebugCanvas);
    fDebugCanvas->draw(fCanvas);

    fIndex = fDebugCanvas->getSize();

    SkColor color = fBitmap.getColor(fBitmap.width()-1,fBitmap.height()-1);

    int r = SkColorGetR(color);
    int g = SkColorGetG(color);
    int b = SkColorGetB(color);

    /* NOTE(chudy): This was a test to determine if the canvas size is accurately
     * saved in the bounds of the recorded picture. It is not. Everyone of the
     * sample GM images is 1000x1000. Even the one that claims it is
     * 2048x2048.
    std::cout << "Width: " << picture->width();
    std::cout << " Height: " <<  picture->height() << std::endl; */

    /* Updated style sheet without a background specified. If not removed
     * QPainter paints the specified background color on top of our canvas. */

    QString style("QWidget {border: 1px solid #cccccc; background-color: #");
    style.append(QString::number(r, 16));
    style.append(QString::number(g, 16));
    style.append(QString::number(b, 16));
    style.append(";}");
    this->setStyleSheet(style);
    this->update();
}

void SkCanvasWidget::mouseMoveEvent(QMouseEvent* event) {

    SkIPoint eventPoint = SkIPoint::Make(event->globalX(), event->globalY());
    fTransform += eventPoint - fPreviousPoint;
    fPreviousPoint = eventPoint;

    // TODO(chudy): Fix and remove +1 from drawTo calls.
    drawTo(fIndex);
    this->update();
}

void SkCanvasWidget::mousePressEvent(QMouseEvent* event) {
    fPreviousPoint.set(event->globalX(), event->globalY());
    fDebugCanvas->getBoxClass()->setHitPoint(event->x(), event->y());
    fDebugCanvas->isCalculatingHits(true);
    drawTo(fIndex);
    emit hitChanged(fDebugCanvas->getHitBoxPoint());
    fDebugCanvas->isCalculatingHits(false);
}

void SkCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    fTransform.set(0,0);
    fScaleFactor = 1.0;
    emit scaleFactorChanged(fScaleFactor);
    drawTo(fIndex);
    this->update();
}

void SkCanvasWidget::paintEvent(QPaintEvent *event) {
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

void SkCanvasWidget::wheelEvent(QWheelEvent* event) {
    fScaleFactor += event->delta()/120;

    /* The range of the fScaleFactor crosses over the range -1,0,1 frequently.
     * Based on the code below, -1 and 1 both scale the image to it's original
     * size we do the following to never have a registered wheel scroll
     * not effect the fScaleFactor. */
    if (fScaleFactor == 0) {
        fScaleFactor += (event->delta()/120) * 2;
    }

    emit scaleFactorChanged(fScaleFactor);

    // TODO(chudy): Fix and remove +1 from drawTo calls.
    drawTo(fIndex);
    this->update();
}
