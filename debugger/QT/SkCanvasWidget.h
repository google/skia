
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKCANVASWIDGET_H_
#define SKCANVASWIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include "SkStream.h"
#include "SkRasterWidget.h"
#include "SkGLWidget.h"

class SkCanvasWidget : public QWidget {
    Q_OBJECT

public:
    SkCanvasWidget();

    ~SkCanvasWidget();

    enum WidgetType {
        kRaster_8888_WidgetType = 1 << 0,
        kGPU_WidgetType         = 1 << 1,
    };

    /**
        Returns the visibility of the command at the specified index.
        @param index  The index of the draw command
     */
    bool commandIsVisibleAtIndex(int index) {
        return fDebugCanvas->getDrawCommandVisibilityAt(index);
    }

    /**
        Toggles the visibility / execution of the draw command at index i with
        the value of toggle.
     */
    void setCommandVisibliltyAtIndex(int index, bool toggle) {
        fDebugCanvas->toggleCommand(index, toggle);
    }

    /**
          Returns a vector of strings with all the current canvas draw
          commands.
     */
    std::vector<std::string>* getDrawCommands() {
        return fDebugCanvas->getDrawCommandsAsStrings();
    }

    SkDebugCanvas* getCurrentDebugCanvas() {
        return fDebugCanvas;
    }

    void drawTo(int index);

    void setWidgetVisibility(WidgetType type, bool isHidden);

    /**
        Toggles drawing filter on all drawing commands previous to current.
     */
    void toggleCurrentCommandFilter(bool toggle) {
        fDebugCanvas->toggleFilter(toggle);
    }

    /**
        TODO(chudy): Refactor into a struct of char**
        Returns parameter information about the ith draw command.
        @param: i  The index of the draw command we are accessing
     */
    std::vector<std::string>* getCurrentCommandInfo(int i) {
        return fDebugCanvas->getCommandInfoAt(i);
    }

    const SkMatrix& getCurrentMatrix() {
        return fRasterWidget.getCurrentMatrix();
    }

    const SkIRect& getCurrentClip() {
        return fRasterWidget.getCurrentClip();
    }

    void loadPicture(QString filename);

    // TODO(chudy): Not full proof since fRasterWidget isn't always drawn to.
    int getBitmapHeight() {
        return fRasterWidget.getBitmapHeight();
    }

    int getBitmapWidth() {
        return fRasterWidget.getBitmapWidth();
    }

    SkRasterWidget* getRasterWidget() {
        return &fRasterWidget;
    }

    void zoom(float zoomIncrement);

signals:
    void scaleFactorChanged(float newScaleFactor);
    void commandChanged(int newCommand);
    void hitChanged(int hit);

private slots:
    void keyZoom(int zoomIncrement) {
        zoom(zoomIncrement);
    }

private:
    QHBoxLayout fHorizontalLayout;
    SkRasterWidget fRasterWidget;
    SkGLWidget fGLWidget;
    SkDebugCanvas* fDebugCanvas;
    SkIPoint fPreviousPoint;
    SkIPoint fUserOffset;
    float fUserScaleFactor;
    int fIndex;

    void resetWidgetTransform();

    void mouseMoveEvent(QMouseEvent* event);

    void mousePressEvent(QMouseEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent* event) {
        zoom(event->delta()/120);
    }
};


#endif /* SKCANVASWIDGET_H_ */
