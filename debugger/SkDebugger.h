
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKDEBUGGER_H_
#define SKDEBUGGER_H_

#include "SkDebugCanvas.h"
#include "SkPicture.h"

class SkDebugger {
public:
    SkDebugger();

    ~SkDebugger();

    void setIndex(int index) {
        fIndex = index;
    }
    void draw(SkCanvas* canvas) {
        if (fIndex > 0) {
            fDebugCanvas->drawTo(canvas, fIndex);
        }
    }

    void step();
    void stepBack();
    void play();
    void rewind();

    bool isCommandVisible(int index) {
        return fDebugCanvas->getDrawCommandVisibilityAt(index);
    }

    void setCommandVisible(int index, bool isVisible) {
        fDebugCanvas->toggleCommand(index, isVisible);
    }

    SkTDArray<SkString*>* getDrawCommands() {
        return fDebugCanvas->getDrawCommandsAsStrings();
    }

    void highlightCurrentCommand(bool on) {
        fDebugCanvas->toggleFilter(on);
    }

    void resize(int width, int height) {
        fDebugCanvas->setBounds(width, height);
    }

    void loadPicture(SkPicture* picture);

    SkPicture* makePicture();

    int getSize() {
        return fDebugCanvas->getSize();
    }

    void setUserOffset(SkIPoint userOffset) {
        // Should this live in debugger instead?
        fDebugCanvas->setUserOffset(userOffset);
    }

    void setUserScale(float userScale) {
        fDebugCanvas->setUserScale(userScale);
    }

    int getCommandAtPoint(int x, int y, int index) {
        return fDebugCanvas->getCommandAtPoint(x, y, index);
    }

    SkTDArray<SkString*>* getCommandInfo(int index) {
        return fDebugCanvas->getCommandInfo(index);
    }

    const SkMatrix& getCurrentMatrix() {
        return fDebugCanvas->getCurrentMatrix();
    }

    const SkIRect& getCurrentClip() {
        return fDebugCanvas->getCurrentClip();
    }

    int pictureHeight() {
        return fPictureHeight;
    }

    int pictureWidth() {
        return fPictureWidth;
    }

    int index() {
        return fIndex;
    }

private:
    SkDebugCanvas* fDebugCanvas;
    SkPicture* fPicture;

    int fPictureWidth;
    int fPictureHeight;
    int fIndex;
};


#endif /* SKDEBUGGER_H_ */
