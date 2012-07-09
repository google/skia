
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKCANVASWIDGET_H
#define SKCANVASWIDGET_H

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDebugCanvas.h"
#include "SkDevice.h"
#include "SkPicture.h"
#include <QApplication>
#include <QtGui>
#include <QWidget>
#include <QWheelEvent>

/** \class SkCanvasWidget

      The QtWidget encompasses all skia screen drawing elements. It initializes
      an SkBitmap in memory that our SkCanvas draws to directly in memory.
      Then using QImage and QPainter we draw those pixels on the screen in
      this widget.
 */
class SkCanvasWidget : public QWidget {
    Q_OBJECT

public:
    /**
         Constructs a widget with the specified parent for layout purposes.
        @param parent  The parent container of this widget
     */
    SkCanvasWidget(QWidget *parent);

    ~SkCanvasWidget();

    /**
        Executes all saved draw commands up to the specified index.
         @param index  The position of the command we draw up to.
     */
    void drawTo(int index);

    /**
        Returns the height of the bitmap.
     */
    int getBitmapHeight() { return fBitmap.height(); }


    /*
        Returns the width of the bitmap.
     */
    int getBitmapWidth() { return fBitmap.width(); }

    /**
        Returns an array of values of the current matrix.
     */
    const SkMatrix& getCurrentMatrix() {
        return fCanvas->getTotalMatrix();
    }

    /**
        Returns an array of values of the current bounding clip.
     */
    const SkIRect& getCurrentClip() {
        return fCanvas->getTotalClip().getBounds();
    }

    /**
        TODO(chudy): Refactor into a struct of char**
        Returns parameter information about the ith draw command.
        @param: i  The index of the draw command we are accessing
     */
    std::vector<std::string>* getCurrentCommandInfo(int i) {
        return fDebugCanvas->getCommandInfoAt(i);
    }

    /**
          Returns a vector of strings with all the current canvas draw
          commands.
     */
    std::vector<std::string>* getDrawCommands() {
        return fDebugCanvas->getDrawCommandsAsStrings();
    }

    /**
        Loads a skia picture located at filename.
        @param filename  The name of the file we are loading.
     */
    void loadPicture(QString filename);

    /**
        Toggles the visibility / execution of the draw command at index i.
     */
    void toggleCommand(int index) {
        fDebugCanvas->toggleCommand(index);
    }

    /**
        Toggles the visibility / execution of the draw command at index i with
        the value of toggle.
     */
    void toggleCommand(int index, bool toggle) {
        fDebugCanvas->toggleCommand(index, toggle);
    }

    /**
        Toggles drawing filter on all drawing commands previous to current.
     */
    void toggleCurrentCommandFilter(bool toggle) {
        fDebugCanvas->toggleFilter(toggle);
    }

    /**
        Captures mouse clicks
        @param event  The event intercepted by Qt
     */
    void mouseMoveEvent(QMouseEvent* event);

    void mousePressEvent(QMouseEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);

    void resizeEvent(QResizeEvent* event);

    void wheelEvent(QWheelEvent* event);

signals:
    void scaleFactorChanged(float newScaleFactor);
    void commandChanged(int newCommand);

protected:
    /**
        Draws the current state of the widget.
        @param event  The event intercepted by Qt
     */
    void paintEvent(QPaintEvent *event);

private:
    SkBitmap fBitmap;
    SkCanvas* fCanvas;
    SkDebugCanvas* fDebugCanvas;
    SkDevice* fDevice;

    SkIPoint fPreviousPoint;
    SkIPoint fTransform;

    int fIndex;
    float fScaleFactor;
};

#endif
