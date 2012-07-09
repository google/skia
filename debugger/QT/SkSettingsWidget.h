
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
#include <QLineEdit>

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

    void setZoomText(int scaleFactor);

    QCheckBox* getCommandCheckBox();

    QRadioButton* getVisibilityButton();

private slots:
    void updateCommand(int newCommand);

signals:
    void scrollingPreferences(bool isStickyActivate);
    void showStyle(bool isSingleCommand);
    void visibilityFilter(bool isEnabled);

private:
    QVBoxLayout mainFrameLayout;
    QFrame mainFrame;
    QVBoxLayout fVerticalLayout;

    QLabel fVisibileText;
    QFrame fVisibleFrame;
    QVBoxLayout fVisibleFrameLayout;
    QRadioButton fVisibleOn;
    QRadioButton fVisibleOff;

    QLabel fCommandToggle;
    QFrame fCommandFrame;
    QVBoxLayout fCommandLayout;

    QLineEdit fCurrentCommandBox;
    QLabel fCurrentCommandLabel;
    QHBoxLayout fCurrentCommandLayout;

    QCheckBox fCommandCheckBox;

    QLabel fZoomSetting;
    QFrame fZoomFrame;
    QLineEdit fZoomBox;
    QHBoxLayout fZoomLayout;
};

#endif /* SKSETTINGSWIDGET_H_ */
