
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKRASTERWIDGET_H_
#define SKRASTERWIDGET_H_

#include "SkSurface.h"
class SkDebugger;

#include <QWidget>

class  SkRasterWidget : public QWidget {
    Q_OBJECT

public:
    SkRasterWidget(SkDebugger* debugger);

    void updateImage();

Q_SIGNALS:
    void drawComplete();

protected:
    void paintEvent(QPaintEvent* event);

    void resizeEvent(QResizeEvent* event);

private:
    SkDebugger* fDebugger;
    sk_sp<SkSurface> fSurface;
    bool fNeedImageUpdate;
};

#endif /* SKRASTERWIDGET_H_ */
