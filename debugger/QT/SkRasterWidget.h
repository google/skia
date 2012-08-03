/*
 * SkRasterWidget.h
 *
 *  Created on: Jul 28, 2012
 *      Author: chudy
 */


#ifndef SKRASTERWIDGET_H_
#define SKRASTERWIDGET_H_

#include "SkGpuDevice.h"
#include "SkDevice.h"
#include "SkDebugCanvas.h"

#include <QApplication>
#include <QtGui>
#include <QWidget>

class  SkRasterWidget : public QWidget {
    Q_OBJECT

public:
    SkRasterWidget();

    ~SkRasterWidget();

    void drawTo(int index) {
        fIndex = index;
        this->update();
    }

    void setDebugCanvas(SkDebugCanvas* debugCanvas) {
        fDebugCanvas = debugCanvas;
        fIndex = debugCanvas->getSize() - 1;
        this->update();
    }

    int getBitmapHeight() {
        return fBitmap.height();
    }

    int getBitmapWidth() {
        return fBitmap.width();
    }

    void setTranslate(SkIPoint transform) {
        fTransform = transform;
    }

    void setScale(float scale) {
        fScaleFactor = scale;
    }

signals:
    void drawComplete();

protected:
    void paintEvent(QPaintEvent* event);

    void resizeEvent(QResizeEvent* event);

private:
    SkBitmap fBitmap;
    SkDebugCanvas* fDebugCanvas;
    SkCanvas* fCanvas;
    SkDevice* fDevice;

    int fIndex;
    SkIPoint fTransform;
    float fScaleFactor;
};

#endif /* SKRASTERWIDGET_H_ */
