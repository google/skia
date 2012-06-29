
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKSETTINGSWIDGET_H_
#define SKSETTINGSWIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QFrame>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

/** \class SkSettingsWidget

    The SettingsWidget contains multiple checkboxes and toggles for altering
    the visibility.
 */
class SkSettingsWidget : public QWidget {
    Q_OBJECT

public:
    /**
        Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkSettingsWidget(QWidget *parent = NULL);
    ~SkSettingsWidget();

private:
    QHBoxLayout* fHorizontalLayout;

    QVBoxLayout* mainFrameLayout;

    QVBoxLayout* fVerticalLayout;
    QVBoxLayout* fVerticalLayout_2;
    QTextEdit* fText;
    QFrame* fFrame;
    QFrame* mainFrame;

    QLabel* fVisibility;
    QRadioButton* fVisibleOn;
    QRadioButton* fVisibleOff;

    QLabel* fCommandToggle;
    QFrame* fCommandFrame;
    QVBoxLayout* fCommandLayout;

    QCheckBox* fCommandCheckBox;
    QCheckBox* fCommandSingleDraw;
};

#endif /* SKSETTINGSWIDGET_H_ */
