
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkListWidget.h"

SkListWidget::SkListWidget(QObject *parent) {}

SkListWidget::~SkListWidget() {}

void SkListWidget::paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    /*
     * NOTE(chudy): We adjust the initial position of the list item so that
     * we don't have overlapping top and bottom borders of concurrent
     * widget items.
     */
    QRect r = option.rect;
    r.adjust(-1,-1,1,0);

    QPen linePen(QColor::fromRgb(211,211,211), 1, Qt::SolidLine);
    QPen fontPen(QColor::fromRgb(51,51,51), 1, Qt::SolidLine);
    QPen fontMarkedPen(Qt::white, 1, Qt::SolidLine);

    // NOTE(chudy): If selected.
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
        // NOTE(chudy): Alternating background.
        painter->setBrush( (index.row() % 2) ? Qt::white : QColor(252,252,252) );
        painter->drawRect(r);

        painter->setPen(linePen);
        painter->drawLine(r.topLeft(),r.topRight());
        painter->drawLine(r.topRight(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.bottomRight());
        painter->drawLine(r.topLeft(),r.bottomLeft());

        painter->setPen(fontPen);
    }

    QIcon ic = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 1).toString();
    QString hidden = index.data(Qt::UserRole + 2).toString();

    int imageSpace = 35;
    r = option.rect.adjusted(5, 10, -10, -10);
    ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);

    // NOTE(chudy): Draw command.
    r = option.rect.adjusted(imageSpace, 0, -10, -7);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignRight, title, &r);

    // NOTE(chudy): Number of draw command.
    r = option.rect.adjusted(imageSpace, 0, -10, -7);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, description, &r);
}

QSize SkListWidget::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const{
    return QSize(200, 30);
}
