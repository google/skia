
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <QtGui>

#include "SkDebugger.h"
#include "SkImageWidget.h"

SkImageWidget::SkImageWidget(SkDebugger *debugger)
    : QWidget()
    , fDebugger(debugger) {
    this->setStyleSheet("QWidget {background-color: white; border: 1px solid #cccccc;}");

    SkImageInfo info;
    info.fWidth = kImageWidgetWidth;
    info.fHeight = kImageWidgetHeight;
    info.fColorType = kN32_SkColorType;
    info.fAlphaType = kPremul_SkAlphaType;

    fSurface = SkSurface::NewRasterDirect(info, fPixels, 4 * kImageWidgetWidth);
}

void SkImageWidget::paintEvent(QPaintEvent* event) {
    if (this->isHidden()) {
        return;
    }

    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const SkTDArray<SkDrawCommand*>& commands = fDebugger->getDrawCommands();
    if (0 != commands.count()) {
        SkDrawCommand* command = commands[fDebugger->index()];

        if (command->render(fSurface->getCanvas())) {
            QPoint origin(0,0);
            QImage image((uchar*) fPixels,
                         kImageWidgetWidth,
                         kImageWidgetHeight,
                         QImage::Format_ARGB32_Premultiplied);

            painter.drawImage(origin, image);
        } else {
            painter.drawRect(0, 0, kImageWidgetWidth, kImageWidgetHeight);
        }
    }

    painter.end();
    emit drawComplete();
}
