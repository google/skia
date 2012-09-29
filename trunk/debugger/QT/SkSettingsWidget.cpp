
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSettingsWidget.h"
#include <iostream>
#include <math.h>

// TODO(chudy): See if the layout can't be attached to the frame post construction.
SkSettingsWidget::SkSettingsWidget() : QWidget()
    , mainFrameLayout(this)
    , fVerticalLayout(&mainFrame)
    , fVisibleFrameLayout(&fVisibleFrame)
    , fVisibleOn(&fVisibleFrame)
    , fVisibleOff(&fVisibleFrame)
    , fCommandLayout(&fCommandFrame)
    , fCurrentCommandBox(&fCommandFrame)
    , fCommandHitBox(&fCommandFrame)
    , fCanvasLayout(&fCanvasFrame)
    , fZoomLayout(&fZoomFrame)
    , fZoomBox(&fZoomFrame)
{
    // Sets up the container and it's alignment around the settings widget.
    mainFrame.setFrameShape(QFrame::StyledPanel);
    mainFrame.setFrameShadow(QFrame::Raised);
    mainFrameLayout.setSpacing(6);
    mainFrameLayout.setContentsMargins(0,0,0,0);
    mainFrameLayout.addWidget(&mainFrame);

    // Vertical Layout is the alignment inside of the main frame.
    fVerticalLayout.setContentsMargins(11,11,11,11);
    fVerticalLayout.setAlignment(Qt::AlignTop);

    // Visible Toggle
    fVisibileText.setText("Visibility Filter");
    fVisibleFrame.setFrameShape(QFrame::StyledPanel);
    fVisibleFrame.setFrameShadow(QFrame::Raised);
    fVisibleOn.setText("On");
    fVisibleOff.setText("Off");
    fVisibleOff.setChecked(true);
    fVisibleFrameLayout.setSpacing(6);
    fVisibleFrameLayout.setContentsMargins(11,11,11,11);
    fVisibleFrameLayout.addWidget(&fVisibleOn);
    fVisibleFrameLayout.addWidget(&fVisibleOff);

    // Canvas
    fCanvasToggle.setText("Render Targets");
    fCanvasFrame.setFrameShape(QFrame::StyledPanel);
    fCanvasFrame.setFrameShadow(QFrame::Raised);

    fRasterLabel.setText("Raster: ");
    fRasterLabel.setMinimumWidth(178);
    fRasterLabel.setMaximumWidth(178);

    fRasterCheckBox.setChecked(true);

    fGLLabel.setText("OpenGL: ");
    fGLLabel.setMinimumWidth(178);
    fGLLabel.setMaximumWidth(178);

    fRasterLayout.addWidget(&fRasterLabel);
    fRasterLayout.addWidget(&fRasterCheckBox);

    fGLLayout.addWidget(&fGLLabel);
    fGLLayout.addWidget(&fGLCheckBox);

    fCanvasLayout.setSpacing(6);
    fCanvasLayout.setContentsMargins(11,11,11,11);
    fCanvasLayout.addLayout(&fRasterLayout);
    fCanvasLayout.addLayout(&fGLLayout);

    // Command Toggle
    fCommandToggle.setText("Command Scrolling Preferences");
    fCommandFrame.setFrameShape(QFrame::StyledPanel);
    fCommandFrame.setFrameShadow(QFrame::Raised);

    fCurrentCommandLabel.setText("Current Command: ");
    fCurrentCommandLabel.setMinimumWidth(178);
    fCurrentCommandLabel.setMaximumWidth(178);
    fCurrentCommandBox.setText("0");
    fCurrentCommandBox.setMinimumSize(QSize(50,25));
    fCurrentCommandBox.setMaximumSize(QSize(50,25));
    fCurrentCommandBox.setAlignment(Qt::AlignRight);

    fCurrentCommandLayout.setSpacing(0);
    fCurrentCommandLayout.setContentsMargins(0,0,0,0);
    fCurrentCommandLayout.setAlignment(Qt::AlignLeft);
    fCurrentCommandLayout.addWidget(&fCurrentCommandLabel);
    fCurrentCommandLayout.addWidget(&fCurrentCommandBox);

    fCommandHitLabel.setText("Command HitBox: ");
    fCommandHitLabel.setMinimumWidth(178);
    fCommandHitLabel.setMaximumWidth(178);
    fCommandHitBox.setText("0");
    fCommandHitBox.setMinimumSize(QSize(50,25));
    fCommandHitBox.setMaximumSize(QSize(50,25));
    fCommandHitBox.setAlignment(Qt::AlignRight);
    fCommandHitLayout.setSpacing(0);
    fCommandHitLayout.setContentsMargins(0,0,0,0);
    fCommandHitLayout.setAlignment(Qt::AlignLeft);
    fCommandHitLayout.addWidget(&fCommandHitLabel);
    fCommandHitLayout.addWidget(&fCommandHitBox);

    fCommandLayout.setSpacing(6);
    fCommandLayout.setContentsMargins(11,11,11,11);
    fCommandLayout.addLayout(&fCurrentCommandLayout);
    fCommandLayout.addLayout(&fCommandHitLayout);

    // Zoom Info
    fZoomSetting.setText("Zoom Level: ");
    fZoomSetting.setMinimumWidth(178);
    fZoomSetting.setMaximumWidth(178);
    fZoomFrame.setFrameShape(QFrame::StyledPanel);
    fZoomFrame.setFrameShadow(QFrame::Raised);
    fZoomBox.setText("100%");
    fZoomBox.setMinimumSize(QSize(50,25));
    fZoomBox.setMaximumSize(QSize(50,25));
    fZoomBox.setAlignment(Qt::AlignRight);
    fZoomLayout.setSpacing(6);
    fZoomLayout.setContentsMargins(11,11,11,11);
    fZoomLayout.addWidget(&fZoomSetting);
    fZoomLayout.addWidget(&fZoomBox);

    // Adds all widgets to settings container
    fVerticalLayout.addWidget(&fVisibileText);
    fVerticalLayout.addWidget(&fVisibleFrame);
    fVerticalLayout.addWidget(&fCommandToggle);
    fVerticalLayout.addWidget(&fCommandFrame);
    fVerticalLayout.addWidget(&fCanvasToggle);
    fVerticalLayout.addWidget(&fCanvasFrame);
    fVerticalLayout.addWidget(&fZoomFrame);

    this->setDisabled(true);
}


void SkSettingsWidget::updateCommand(int newCommand) {
    fCurrentCommandBox.setText(QString::number(newCommand));
}

void SkSettingsWidget::updateHit(int newHit) {
    fCommandHitBox.setText(QString::number(newHit));
}

QRadioButton* SkSettingsWidget::getVisibilityButton() {
    return &fVisibleOn;
}

void SkSettingsWidget::setZoomText(int scaleFactor) {
    if(scaleFactor == 1 || scaleFactor == -1) {
        fZoomBox.setText("100%");
    } else if (scaleFactor > 1) {
        fZoomBox.setText(QString::number(scaleFactor*100).append("%"));
    } else if (scaleFactor < -1) {
        fZoomBox.setText(QString::number(100 / pow(2.0f, (-scaleFactor - 1))).append("%"));
    }
}
