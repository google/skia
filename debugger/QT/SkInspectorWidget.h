
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKINSPECTORWIDGET_H_
#define SKINSPECTORWIDGET_H_

#include "SkMatrix.h"

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

/** \class SkInspectorWidget

    The InspectorWidget contains the overview and details tab. These contain
    information about the whole picture and details about each draw command.
 */
class SkInspectorWidget : public QWidget {
    Q_OBJECT

public:
    /**
        Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkInspectorWidget(QWidget *parent = NULL);

    ~SkInspectorWidget();

    void setDisabled(bool isDisabled) {
        fMatrixAndClipWidget.setDisabled(isDisabled);
    }
    /**
        Sets the text in the detail tab.
        @param text
     */
    void setDetailText(QString text);

    /**
        Sets the text in the overview tab.
        @param text
     */
    void setOverviewText(QString text);

    /**
        Sets the text in the current matrix.
        @param matrixValues
     */
    void setMatrix(const SkMatrix& matrix);

    /**
        Sets the text in the current clip.
        @param clipValues
     */
    void setClip(const SkIRect& clip);

private:
    QHBoxLayout fHorizontalLayout;
    QTabWidget fTabWidget;

    QWidget fOverviewTab;
    QHBoxLayout fOverviewLayout;
    QTextEdit fOverviewText;

    QWidget fDetailTab;
    QHBoxLayout fDetailLayout;
    QTextEdit fDetailText;

    QWidget fMatrixAndClipWidget;
    QVBoxLayout fVerticalLayout;

    QLabel fMatrixLabel;
    QVBoxLayout fMatrixLayout;
    QHBoxLayout fMatrixRow[3];
    QLineEdit fMatrixEntry[9];

    QLabel fClipLabel;
    QVBoxLayout fClipLayout;
    QHBoxLayout fClipRow[2];
    QLineEdit fClipEntry[4];

    QVBoxLayout* setupMatrix();
    QVBoxLayout* setupClip();
};

#endif
