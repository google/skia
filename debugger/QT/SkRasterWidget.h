/*
 * SkRasterWidget.h
 *
 *  Created on: Jul 28, 2012
 *      Author: chudy
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

signals:
    void drawComplete();

protected:
    void paintEvent(QPaintEvent* event);

    void resizeEvent(QResizeEvent* event);

private:
    SkDebugger* fDebugger;
    SkAutoTUnref<SkSurface> fSurface;
    bool fNeedImageUpdate;
};

#endif /* SKRASTERWIDGET_H_ */
