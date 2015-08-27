
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <QtGui>

#include "SkDebugger.h"
#include "SkDrawCommandGeometryWidget.h"

SkDrawCommandGeometryWidget::SkDrawCommandGeometryWidget(SkDebugger *debugger)
    : QFrame()
    , fDebugger(debugger)
    , fCommandIndex(-1) {
    this->setStyleSheet("QFrame {background-color: black; border: 1px solid #cccccc;}");
}

void SkDrawCommandGeometryWidget::resizeEvent(QResizeEvent* event) {
    this->QFrame::resizeEvent(event);
    QRect r = this->contentsRect();
    int dim = std::min(r.width(), r.height());
    if (dim == 0) {
        fSurface.reset(nullptr);
    } else {
        SkImageInfo info = SkImageInfo::MakeN32Premul(dim, dim);
        fSurface.reset(SkSurface::NewRaster(info));
        this->updateImage();
    }
}

void SkDrawCommandGeometryWidget::paintEvent(QPaintEvent* event) {
    this->QFrame::paintEvent(event);

    if (!fSurface) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    SkImageInfo info;
    size_t rowBytes;
    if (const void* pixels = fSurface->peekPixels(&info, &rowBytes)) {
        SkASSERT(info.width() > 0);
        SkASSERT(info.height() > 0);

        QRectF resultRect;
        if (this->width() < this->height()) {
            float ratio = this->width() / info.width();
            resultRect = QRectF(0, 0, this->width(), ratio * info.height());
        } else {
            float ratio = this->height() / info.height();
            resultRect = QRectF(0, 0, ratio * info.width(), this->height());
        }

        resultRect.moveCenter(this->contentsRect().center());

        QImage image(reinterpret_cast<const uchar*>(pixels),
                     info.width(),
                     info.height(),
                     rowBytes,
                     QImage::Format_ARGB32_Premultiplied);
        painter.drawImage(resultRect, image);
    }
}

void SkDrawCommandGeometryWidget::setDrawCommandIndex(int commandIndex) {
    fCommandIndex = commandIndex;
    this->updateImage();
}

void SkDrawCommandGeometryWidget::updateImage() {
    if (!fSurface) {
        return;
    }

    bool didRender = false;
    const SkTDArray<SkDrawCommand*>& commands = fDebugger->getDrawCommands();
    if (0 != commands.count() && fCommandIndex >= 0) {
        SkASSERT(commands.count() > fCommandIndex);
        SkDrawCommand* command = commands[fCommandIndex];
        didRender = command->render(fSurface->getCanvas());
    }

    if (!didRender) {
        fSurface->getCanvas()->clear(SK_ColorTRANSPARENT);
    }

    fSurface->getCanvas()->flush();
    this->update();
}
