
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKLISTWIDGET_H_
#define SKLISTWIDGET_H_

#include <QAbstractItemDelegate>
#include <QPainter>

/** \class SkListWidget

    This widget contains the draw commands.
 */
class SkListWidget : public QAbstractItemDelegate {
public:
    /**
        Constructs the list widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkListWidget(QObject* parent = nullptr) {}

    virtual ~SkListWidget() {}

    /**
        Draws the current state of the widget. Overriden from QWidget.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index ) const;

    /**
        Returns the default size of the widget. Overriden from QWidget.
     */
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;
};

#endif
