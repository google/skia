
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKINSPECTORWIDGET_H_
#define SKINSPECTORWIDGET_H_

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

    /**
        Returns a QString representation of the text currently in the detail tab.
     */
    QString getDetailText();

    /**
        Returns a QString representation of the text currently in the overview tab.
     */
    QString getOverviewText();

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

private:
    QWidget* fDetailTab;
    QTextEdit* fDetailText;
    QHBoxLayout* fDetailLayout;
    QHBoxLayout* fHorizontalLayout;
    QWidget* fOverviewTab;
    QTextEdit* fOverviewText;
    QHBoxLayout* fOverviewLayout;
    QTabWidget* fTabWidget;

    QWidget* fMatrixAndClipWidget;
    QVBoxLayout* fVerticalLayout;

    QVBoxLayout* fMatrixLayout;
    QLabel* fMatrixLabel;
    QHBoxLayout* fMatrixRow[3];
    QLineEdit* fMatrixEntry[9];

    QVBoxLayout* fClipLayout;
    QLabel* fClipLabel;
    QHBoxLayout* fClipRow[2];
    QLineEdit* fClipEntry[4];

    QVBoxLayout* currentMatrix();
    QVBoxLayout* currentClip();
};

#endif
