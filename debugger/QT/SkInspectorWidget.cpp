
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkInspectorWidget.h"
#include <iostream>

SkInspectorWidget::SkInspectorWidget(QWidget *parent) : QWidget(parent) {
    // NOTE(chudy): Keeps the inspector widget fully expanded.
    fHorizontalLayout = new QHBoxLayout(this);
    fHorizontalLayout->setSpacing(6);
    fHorizontalLayout->setContentsMargins(11, 11, 11, 11);

    fTabWidget = new QTabWidget();

    fOverviewTab = new QWidget();
    fOverviewLayout = new QHBoxLayout(fOverviewTab);
    fOverviewLayout->setSpacing(6);
    fOverviewLayout->setContentsMargins(11, 11, 11, 11);
    fOverviewText = new QTextEdit();
    fOverviewText->setReadOnly(true);
    fOverviewLayout->addWidget(fOverviewText);

    fDetailTab = new QWidget();
    fDetailLayout = new QHBoxLayout(fDetailTab);
    fDetailLayout->setSpacing(6);
    fDetailLayout->setContentsMargins(11,11,11,11);
    fDetailText = new QTextEdit();
    fDetailText->setReadOnly(true);
    fDetailLayout->addWidget(fDetailText);

    fTabWidget->addTab(fOverviewTab, QString("Overview"));
    fTabWidget->addTab(fDetailTab, QString("Details"));

    fHorizontalLayout->setAlignment(Qt::AlignTop);
    fHorizontalLayout->addWidget(fTabWidget);

    /* NOTE(chudy): We add all the line edits to (this). Then we lay them out
     * by adding them to horizontal layouts.
     *
     * We will have 1 big vertical layout, 3 horizontal layouts and then 3
     * line edits in each horizontal layout.
     */

    fMatrixAndClipWidget = new QWidget(this);
    fMatrixAndClipWidget->setFixedSize(200,300);
    fMatrixAndClipWidget->setDisabled(true);

    fVerticalLayout = new QVBoxLayout(fMatrixAndClipWidget);
    fVerticalLayout->setAlignment(Qt::AlignVCenter);
    fVerticalLayout->addLayout(currentMatrix());
    fVerticalLayout->addLayout(currentClip());
    fHorizontalLayout->addWidget(fMatrixAndClipWidget);


}

SkInspectorWidget::~SkInspectorWidget() {}

QString SkInspectorWidget::getDetailText() {
    return fDetailText->toHtml();
}

QString SkInspectorWidget::getOverviewText() {
    return fOverviewText->toHtml();
}

void SkInspectorWidget::setDetailText(QString text) {
    fDetailText->setHtml(text);
}

void SkInspectorWidget::setOverviewText(QString text) {
    fOverviewText->setHtml(text);
}

QVBoxLayout* SkInspectorWidget::currentMatrix() {
    fMatrixLabel = new QLabel(this);
    fMatrixLabel->setText("Current Matrix");
    fMatrixLabel->setAlignment(Qt::AlignHCenter);
    fMatrixLayout = new QVBoxLayout();
    fMatrixLayout->setSpacing(6);
    fMatrixLayout->setContentsMargins(11,11,11,0);
    fMatrixLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    fMatrixLayout->addWidget(fMatrixLabel);

    for(int i =0; i<9; i++) {
        fMatrixEntry[i] = new QLineEdit();
        fMatrixEntry[i]->setMinimumSize(QSize(50,25));
        fMatrixEntry[i]->setMaximumSize(QSize(50,16777215));
        fMatrixEntry[i]->setReadOnly(true);

        if(!(i%3)) fMatrixRow[i/3] = new QHBoxLayout();
        fMatrixRow[i/3]->addWidget(fMatrixEntry[i]);
        if(i%3 == 2) fMatrixLayout->addLayout(fMatrixRow[i/3]);
    }

    return fMatrixLayout;
}

QVBoxLayout* SkInspectorWidget::currentClip() {
    fClipLabel = new QLabel(this);
    fClipLabel->setText("Current Clip");
    fClipLabel->setAlignment(Qt::AlignHCenter);
    fClipLayout = new QVBoxLayout();
    fClipLayout->setSpacing(6);
    fClipLayout->setContentsMargins(11,11,11,0);
    fClipLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    fClipLayout->addWidget(fClipLabel);

    for(int i =0; i<4; i++) {
        fClipEntry[i] = new QLineEdit();
        fClipEntry[i]->setMinimumSize(QSize(50,25));
        fClipEntry[i]->setMaximumSize(QSize(50,16777215));
        fClipEntry[i]->setReadOnly(true);

        if(!(i%2)) fClipRow[i/2] = new QHBoxLayout();
        fClipRow[i/2]->addWidget(fClipEntry[i]);
        if(i%2 == 1) fClipLayout->addLayout(fClipRow[i/2]);
    }

    return fClipLayout;
}
