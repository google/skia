
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
#include <QGroupBox>
#include <QGridLayout>

/** \class SkInspectorWidget

    The InspectorWidget contains the overview and details tab. These contain
    information about the whole picture and details about each draw command.
 */
class SkInspectorWidget : public QWidget {
    Q_OBJECT

public:
    enum TabType {
        kOverview_TabType,
        kDetail_TabType,
        kClipStack_TabType,
        kTotalTabCount,
    };

    /**
        Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkInspectorWidget();

    void setDisabled(bool isDisabled) {
        fMatrixAndClipWidget.setDisabled(isDisabled);
    }

    /**
        Sets the text in tab at the specified index.
        @param text
     */
    void setText(QString text, TabType type);

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

    class Tab : public QWidget {
        QWidget fTab;
        QHBoxLayout fTabLayout;
        QTextEdit fTabText;
        QString fName;

        Tab(const char* name) {
            fTabText.setReadOnly(true);
            fTabLayout.addWidget(&fTabText);
            fTab.setLayout(&fTabLayout);
            fName = QString(name);
        }
    };

private:
    QHBoxLayout fHorizontalLayout;
    QTabWidget fTabWidget;

    QWidget fTabs[kTotalTabCount];
    QHBoxLayout fTabLayouts[kTotalTabCount];
    QTextEdit fTabTexts[kTotalTabCount];

    QFrame fMatrixAndClipWidget;
    QVBoxLayout fVerticalLayout;

    QGroupBox fMatrixGroup;
    QGridLayout fMatrixLayout;
    QLineEdit fMatrixEntry[9];

    QGroupBox fClipGroup;
    QGridLayout fClipLayout;
    QLineEdit fClipEntry[4];

    void setupMatrix();
    void setupClip();
};

#endif
