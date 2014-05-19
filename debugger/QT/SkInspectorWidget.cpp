
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkInspectorWidget.h"
#include <iostream>

SkInspectorWidget::SkInspectorWidget() : QWidget()
    , fHorizontalLayout(this)
    , fMatrixAndClipWidget(this)
    , fVerticalLayout(&fMatrixAndClipWidget)
    , fMatrixLabel(this)
    , fClipLabel(this) {

    fHorizontalLayout.setSpacing(6);
    fHorizontalLayout.setContentsMargins(11, 11, 11, 11);

    QString tabNames[kTotalTabCount];
    tabNames[kOverview_TabType] = "Overview";
    tabNames[kDetail_TabType] = "Details";
    tabNames[kClipStack_TabType] = "Clip Stack";

    for (int i = 0; i < kTotalTabCount; i++) {
        fTabTexts[i].setReadOnly(true);
        fTabLayouts[i].setSpacing(6);
        fTabLayouts[i].setContentsMargins(11, 11, 11, 11);
        fTabLayouts[i].addWidget(&fTabTexts[i]);
        fTabs[i].setLayout(&fTabLayouts[i]);
        fTabWidget.addTab(&fTabs[i], tabNames[i]);
    }

    fHorizontalLayout.setAlignment(Qt::AlignTop);
    fHorizontalLayout.addWidget(&fTabWidget);

    /* NOTE(chudy): We add all the line edits to (this). Then we lay them out
     * by adding them to horizontal layouts.
     *
     * We will have 1 big vertical layout, 3 horizontal layouts and then 3
     * line edits in each horizontal layout. */
    fMatrixAndClipWidget.setFixedSize(260,300);
    fMatrixAndClipWidget.setDisabled(true);

    fVerticalLayout.setAlignment(Qt::AlignVCenter);
    fVerticalLayout.addLayout(setupMatrix());
    fVerticalLayout.addLayout(setupClip());
    fHorizontalLayout.addWidget(&fMatrixAndClipWidget);
}

void SkInspectorWidget::setText(QString text, TabType type) {
    fTabTexts[type].setHtml(text);
}

void SkInspectorWidget::setMatrix(const SkMatrix& matrix) {
    for(int i=0; i<9; i++) {
        fMatrixEntry[i].setText(QString::number(matrix.get(i)));
    }
}

void SkInspectorWidget::setClip(const SkIRect& clip) {
    fClipEntry[0].setText(QString::number(clip.left()));
    fClipEntry[1].setText(QString::number(clip.top()));
    fClipEntry[2].setText(QString::number(clip.right()));
    fClipEntry[3].setText(QString::number(clip.bottom()));
}

QVBoxLayout* SkInspectorWidget::setupMatrix() {
    fMatrixLabel.setText("Current Matrix");
    fMatrixLabel.setAlignment(Qt::AlignHCenter);

    fMatrixLayout.setSpacing(6);
    fMatrixLayout.setContentsMargins(11,11,11,0);
    fMatrixLayout.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    fMatrixLayout.addWidget(&fMatrixLabel);

    for(int i =0; i<9; i++) {
        fMatrixEntry[i].setMinimumSize(QSize(70,25));
        fMatrixEntry[i].setMaximumSize(QSize(70,16777215));
        fMatrixEntry[i].setReadOnly(true);

        fMatrixRow[i/3].addWidget(&fMatrixEntry[i]);
        if(i%3 == 2) {
            fMatrixLayout.addLayout(&fMatrixRow[i/3]);
        }
    }

    return &fMatrixLayout;
}

QVBoxLayout* SkInspectorWidget::setupClip() {
    fClipLabel.setText("Current Clip");
    fClipLabel.setAlignment(Qt::AlignHCenter);

    fClipLayout.setSpacing(6);
    fClipLayout.setContentsMargins(11,11,11,0);
    fClipLayout.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    fClipLayout.addWidget(&fClipLabel);

    for(int i =0; i<4; i++) {
        fClipEntry[i].setMinimumSize(QSize(70,25));
        fClipEntry[i].setMaximumSize(QSize(70,16777215));
        fClipEntry[i].setReadOnly(true);

        fClipRow[i/2].addWidget(&fClipEntry[i]);
        if(i%2 == 1) {
            fClipLayout.addLayout(&fClipRow[i/2]);
        }
    }

    return &fClipLayout;
}
