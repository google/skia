/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKDRAWCOMMANDGEOMETRYWIDGET_H_
#define SKDRAWCOMMANDGEOMETRYWIDGET_H_

#include <QFrame>

#include "SkSurface.h"
class SkDebugger;

class  SkDrawCommandGeometryWidget : public QFrame {
    Q_OBJECT

public:
    SkDrawCommandGeometryWidget(SkDebugger* debugger);
    void setDrawCommandIndex(int index);

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    void updateImage();

    SkDebugger* fDebugger;
    sk_sp<SkSurface> fSurface;
    int fCommandIndex;
};

#endif /* SKDRAWCOMMANDGEOMETRYWIDGET_H_ */
