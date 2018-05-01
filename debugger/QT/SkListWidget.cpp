
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkListWidget.h"

void SkListWidget::paint (QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    /* We adjust the initial position of the list item so that
     * we don't have overlapping top and bottom borders of concurrent
     * widget items. */
    QRect r = option.rect;
    r.adjust(-1,-1,1,0);

    QPen linePen(QColor::fromRgb(211,211,211), 1, Qt::SolidLine);
    QPen fontPen(QColor::fromRgb(51,51,51), 1, Qt::SolidLine);
    QPen fontMarkedPen(Qt::white, 1, Qt::SolidLine);

    // If selected
    if(option.state & QStyle::State_Selected){
        QLinearGradient gradientSelected(r.left(),r.top(),r.left(),r.height()+r.top());
        gradientSelected.setColorAt(0.0, QColor::fromRgb(119,213,247));
        gradientSelected.setColorAt(0.9, QColor::fromRgb(27,134,183));
        gradientSelected.setColorAt(1.0, QColor::fromRgb(0,120,174));
        painter->setBrush(gradientSelected);
        painter->drawRect(r);

        painter->setPen(linePen);
        painter->drawLine(r.topLeft(),r.topRight());
        painter->drawLine(r.topRight(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.bottomRight());
        painter->drawLine(r.topLeft(),r.bottomLeft());

        painter->setPen(fontMarkedPen);

    } else {
        // Alternating background
        painter->setBrush((index.row() % 2) ? Qt::white : QColor(252,252,252));
        painter->drawRect(r);

        painter->setPen(linePen);
        painter->drawLine(r.topLeft(),r.topRight());
        painter->drawLine(r.topRight(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.bottomRight());
        painter->drawLine(r.topLeft(),r.bottomLeft());

        painter->setPen(fontPen);
    }

    QIcon breakpointIcon =
            QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QIcon deleteIcon =
            QIcon(qvariant_cast<QPixmap>(index.data(Qt::UserRole + 2)));
    int indent = index.data(Qt::UserRole + 3).toInt();

    QString drawCommandText = index.data(Qt::DisplayRole).toString();
    QString drawCommandNumber;
    drawCommandNumber = index.data(Qt::UserRole + 1).toString();
    float time = index.data(Qt::UserRole + 4).toFloat();
    QString drawTime;
    drawTime.setNum(time, 'f', 2);
    drawTime += "%";

    /* option.rect is a struct that Qt uses as a target to draw into. Following
     * the format (x1,y1,x2,y2) x1 and y1 represent where the painter can start
     * drawing. x2 and y2 represent where the drawing area has to terminate
     * counting from the bottom right corner of each list item styled with this
     * widget. A (x1,y1,0,0) rect would mean that the item being drawn would
     * be pushed down into that bottom corner. Negative values in the x2,y2
     * spot act as a margin for the bottom and right sides. Positive values in
     * x1,y1 act as a margin for the top and left. The target area will not
     * affect size of text but will scale icons. */
    static const int kImageSpace = 35;
    static const int kCommandNumberSpace = 33;
    static const int kTimeSpace = 30;

    // Breakpoint Icon
    r = option.rect.adjusted(5, 10, -10, -10);
    breakpointIcon.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);

    // Delete Icon
    r = option.rect.adjusted(19, 10, -10, -10);
    deleteIcon.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);

    // Draw Command
    if (time >= 0.0) {
        r = option.rect.adjusted(kImageSpace+kCommandNumberSpace+kTimeSpace+indent, 0, -10, -7);
    } else {
        // don't need time offset
        r = option.rect.adjusted(kImageSpace+kCommandNumberSpace+indent, 0, -10, -7);
    }
    painter->drawText(r.left(), r.top(), r.width(), r.height(),
            Qt::AlignBottom|Qt::AlignLeft, drawCommandText, &r);

    // Draw Command Number
    r = option.rect.adjusted(kImageSpace, 0, -10, -7);
    painter->drawText(r.left(), r.top(), r.width(), r.height(),
                      Qt::AlignBottom|Qt::AlignLeft, drawCommandNumber, &r);

    if (time >= 0.0) {
        // Draw time
        r = option.rect.adjusted(kImageSpace+kCommandNumberSpace, 0, -10, -7);
        painter->drawText(r.left(), r.top(), r.width(), r.height(),
                          Qt::AlignBottom|Qt::AlignLeft, drawTime, &r);
    }
}

QSize SkListWidget::sizeHint (const QStyleOptionViewItem& option,
                              const QModelIndex& index) const{
    return QSize(200, 30);
}
