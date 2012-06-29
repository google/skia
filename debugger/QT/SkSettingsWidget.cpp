
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSettingsWidget.h"

SkSettingsWidget::SkSettingsWidget(QWidget *parent) : QWidget(parent) {
    mainFrameLayout = new QVBoxLayout(this);
    mainFrameLayout->setSpacing(6);
    mainFrameLayout->setContentsMargins(0,0,0,0);


    mainFrame = new QFrame();
    mainFrame->setFrameShape(QFrame::StyledPanel);
    mainFrame->setFrameShadow(QFrame::Raised);

    mainFrameLayout->addWidget(mainFrame);

    fVerticalLayout = new QVBoxLayout(mainFrame);
    fVerticalLayout->setContentsMargins(11,11,11,11);
    fVerticalLayout->setAlignment(Qt::AlignTop);

    fVisibility = new QLabel();
    fVisibility->setText("Visibility Filter");

    fFrame = new QFrame();
    fFrame->setFrameShape(QFrame::StyledPanel);
    fFrame->setFrameShadow(QFrame::Raised);

    fVerticalLayout_2 = new QVBoxLayout(fFrame);
    fVerticalLayout_2->setSpacing(6);
    fVerticalLayout_2->setContentsMargins(11,11,11,11);

    fVerticalLayout->addWidget(fVisibility);
    fVerticalLayout->addWidget(fFrame);

    fVisibleOn = new QRadioButton(fFrame);
    fVisibleOn->setText("On");
    fVisibleOff = new QRadioButton(fFrame);
    fVisibleOff->setText("Off");

    fVisibleOff->setChecked(true);

    fVerticalLayout_2->addWidget(fVisibleOn);
    fVerticalLayout_2->addWidget(fVisibleOff);

    fCommandToggle = new QLabel();
    fCommandToggle->setText("Command Scrolling Preferences");

    fCommandFrame = new QFrame();
    fCommandFrame->setFrameShape(QFrame::StyledPanel);
    fCommandFrame->setFrameShadow(QFrame::Raised);

    fCommandLayout = new QVBoxLayout(fCommandFrame);
    fCommandLayout->setSpacing(6);
    fCommandLayout->setContentsMargins(11,11,11,11);

    fVerticalLayout->addWidget(fCommandToggle);
    fVerticalLayout->addWidget(fCommandFrame);

    fCommandCheckBox = new QCheckBox(fCommandFrame);
    fCommandCheckBox->setText("Toggle Sticky Activate");
    fCommandSingleDraw = new QCheckBox(fCommandFrame);
    fCommandSingleDraw->setText("Display Single Command");

    fCommandLayout->addWidget(fCommandCheckBox);
    fCommandLayout->addWidget(fCommandSingleDraw);

    this->setDisabled(true);
}

SkSettingsWidget::~SkSettingsWidget() {}
