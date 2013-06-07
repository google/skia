/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKIMAGEWIDGET_H_
#define SKIMAGEWIDGET_H_

#include <QWidget>

#include "SkSurface.h"
class SkDebugger;

class  SkImageWidget : public QWidget {
    Q_OBJECT

public:
    SkImageWidget(SkDebugger* debugger);

    virtual ~SkImageWidget() {
        fSurface->unref();
    }

    void draw() {
        this->update();
    }

    static const int kImageWidgetWidth = 256;
    static const int kImageWidgetHeight = 256;

signals:
    void drawComplete();

protected:
    void paintEvent(QPaintEvent* event);

private:
    SkDebugger* fDebugger;
    char        fPixels[kImageWidgetHeight * 4 * kImageWidgetWidth];
    SkSurface*  fSurface;
};

#endif /* SKIMAGEWIDGET_H_ */
