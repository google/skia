
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkInspectorWidget.h"
#include <iostream>

static const int kSignificantNumbersInFields = 5;

SkInspectorWidget::SkInspectorWidget() : QWidget()
    , fHorizontalLayout(this)
    , fMatrixAndClipWidget(this)
    , fVerticalLayout(&fMatrixAndClipWidget) {
    QString tabNames[kTotalTabCount];
    tabNames[kOverview_TabType] = "Overview";
    tabNames[kDetail_TabType] = "Details";
    tabNames[kClipStack_TabType] = "Clip Stack";

    for (int i = 0; i < kTotalTabCount; i++) {
        fTabTexts[i].setReadOnly(true);
        fTabLayouts[i].addWidget(&fTabTexts[i]);
        fTabs[i].setLayout(&fTabLayouts[i]);
        fTabWidget.addTab(&fTabs[i], tabNames[i]);
    }

    fHorizontalLayout.setAlignment(Qt::AlignTop);
    fHorizontalLayout.addWidget(&fTabWidget);

    fMatrixAndClipWidget.setFrameStyle(QFrame::Panel);
    fMatrixAndClipWidget.setDisabled(true);
    fVerticalLayout.setAlignment(Qt::AlignVCenter);
    this->setupMatrix();
    this->setupClip();
    fVerticalLayout.addWidget(&fMatrixGroup);
    fVerticalLayout.addWidget(&fClipGroup);
    fHorizontalLayout.addWidget(&fMatrixAndClipWidget);
}

void SkInspectorWidget::setText(QString text, TabType type) {
    fTabTexts[type].setHtml(text);
}

void SkInspectorWidget::setMatrix(const SkMatrix& matrix) {
    for(int i=0; i<9; i++) {
        fMatrixEntry[i].setText(QString::number(matrix.get(i), 'g', kSignificantNumbersInFields));
    }
}

void SkInspectorWidget::setClip(const SkIRect& clip) {
    fClipEntry[0].setText(QString::number(clip.left(), 'g', kSignificantNumbersInFields));
    fClipEntry[1].setText(QString::number(clip.top(), 'g', kSignificantNumbersInFields));
    fClipEntry[2].setText(QString::number(clip.right(), 'g', kSignificantNumbersInFields));
    fClipEntry[3].setText(QString::number(clip.bottom(), 'g', kSignificantNumbersInFields));
}

void SkInspectorWidget::setupMatrix() {
    fMatrixGroup.setTitle("Current Matrix");
    fMatrixGroup.setLayout(&fMatrixLayout);
    for (int r = 0; r < 3; ++r) {
        for(int c = 0; c < 3; c++) {
            QLineEdit* entry = &fMatrixEntry[r * 3 + c];
            fMatrixLayout.addWidget(entry, r, c, Qt::AlignTop | Qt::AlignHCenter);
            entry->setReadOnly(true);
            entry->setFixedWidth(70);
        }
    }
}

void SkInspectorWidget::setupClip() {
    fClipGroup.setTitle("Current Clip");
    fClipGroup.setLayout(&fClipLayout);
    for(int r = 0; r < 2; r++) {
        for(int c = 0; c < 2; c++) {
            QLineEdit* entry = &fClipEntry[r * 2 + c];
            fClipLayout.addWidget(entry, r, c, Qt::AlignTop | Qt::AlignHCenter);
            entry->setReadOnly(true);
            entry->setFixedWidth(70);
        }
    }
}
