
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
#include "SkTArray.h"

class SkString;

class SkDebugger {
public:
    SkDebugger();

    ~SkDebugger();

    void setIndex(int index) {
        fIndex = index;
    }
    void draw(SkCanvas* canvas) {
        if (fIndex >= 0) {
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

    SkDrawCommand* getDrawCommandAt(int index) {
        return fDebugCanvas->getDrawCommandAt(index);
    }

    const SkTDArray<SkDrawCommand*>& getDrawCommands() const {
        return fDebugCanvas->getDrawCommands();
    }

    void highlightCurrentCommand(bool on) {
        fDebugCanvas->toggleFilter(on);
    }

    void loadPicture(SkPicture* picture);

    SkPicture* copyPicture();

    int getSize() const {
        return fDebugCanvas->getSize();
    }

    void setUserMatrix(SkMatrix userMatrix) {
        // Should this live in debugger instead?
        fDebugCanvas->setUserMatrix(userMatrix);
    }

    int getCommandAtPoint(int x, int y, int index) {
        return fDebugCanvas->getCommandAtPoint(x, y, index);
    }

    const SkTDArray<SkString*>* getCommandInfo(int index) const {
        return fDebugCanvas->getCommandInfo(index);
    }

    const SkMatrix& getCurrentMatrix() {
        return fDebugCanvas->getCurrentMatrix();
    }

    const SkIRect& getCurrentClip() {
        return fDebugCanvas->getCurrentClip();
    }

    SkRect pictureCull() const   { 
        return nullptr == fPicture ? SkRect::MakeEmpty() : fPicture->cullRect();
    }

    int index() {
        return fIndex;
    }

    void setOverdrawViz(bool overDrawViz) {
        if (fDebugCanvas) {
            fDebugCanvas->setOverdrawViz(overDrawViz);
        }
    }

    void setPathOps(bool pathOps) {
        if (fDebugCanvas) {
            fDebugCanvas->setAllowSimplifyClip(pathOps);
        }
    }

    void setMegaViz(bool megaViz) {
        if (fDebugCanvas) {
            fDebugCanvas->setMegaVizMode(megaViz);
        }
    }

    void setTexFilterOverride(bool texFilterOverride, SkFilterQuality quality) {
        if (fDebugCanvas) {
            fDebugCanvas->overrideTexFiltering(texFilterOverride, quality);
        }
    }

    void getOverviewText(const SkTDArray<double>* typeTimes, double totTime,
                         SkString* overview, int numRuns);

    void getClipStackText(SkString* clipStack);

private:
    SkDebugCanvas* fDebugCanvas;
    SkPicture* fPicture;

    int fIndex;
};


#endif /* SKDEBUGGER_H_ */
