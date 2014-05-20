
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugger.h"
#include "SkPictureRecorder.h"
#include "SkString.h"


SkDebugger::SkDebugger() {
    // Create this some other dynamic way?
    fDebugCanvas = new SkDebugCanvas(100, 100);
    fPicture = NULL;
    fPictureWidth = 0;
    fPictureHeight = 0;
    fIndex = 0;
}

SkDebugger::~SkDebugger() {
    // Need to inherit from SkRef object in order for following to work
    SkSafeUnref(fDebugCanvas);
    SkSafeUnref(fPicture);
}

void SkDebugger::loadPicture(SkPicture* picture) {
    fPictureWidth = picture->width();
    fPictureHeight = picture->height();
    delete fDebugCanvas;
    fDebugCanvas = new SkDebugCanvas(fPictureWidth, fPictureHeight);
    fDebugCanvas->setBounds(fPictureWidth, fPictureHeight);
    fDebugCanvas->setPicture(picture);
    picture->draw(fDebugCanvas);
    fDebugCanvas->setPicture(NULL);
    fIndex = fDebugCanvas->getSize() - 1;
    SkRefCnt_SafeAssign(fPicture, picture);
}

SkPicture* SkDebugger::copyPicture() {
    // We can't just call clone here since we want to removed the "deleted"
    // commands. Playing back will strip those out.
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(fPictureWidth, fPictureHeight, NULL, 0);

    bool vizMode = fDebugCanvas->getMegaVizMode();
    fDebugCanvas->setMegaVizMode(false);
    bool overDraw = fDebugCanvas->getOverdrawViz();
    fDebugCanvas->setOverdrawViz(false);
    bool pathOps = fDebugCanvas->getAllowSimplifyClip();
    fDebugCanvas->setAllowSimplifyClip(false);
    int saveCount = fDebugCanvas->getOutstandingSaveCount();
    fDebugCanvas->setOutstandingSaveCount(0);

    fDebugCanvas->draw(canvas);

    int temp = fDebugCanvas->getOutstandingSaveCount();
    for (int i = 0; i < temp; ++i) {
        canvas->restore();
    }

    fDebugCanvas->setMegaVizMode(vizMode);
    fDebugCanvas->setOverdrawViz(overDraw);
    fDebugCanvas->setOutstandingSaveCount(saveCount);
    fDebugCanvas->setAllowSimplifyClip(pathOps);

    return recorder.endRecording();
}

void SkDebugger::getOverviewText(const SkTDArray<double>* typeTimes,
                                 double totTime,
                                 SkString* overview,
                                 int numRuns) {
    const SkTDArray<SkDrawCommand*>& commands = this->getDrawCommands();

    SkTDArray<int> counts;
    counts.setCount(LAST_DRAWTYPE_ENUM+1);
    for (int i = 0; i < LAST_DRAWTYPE_ENUM+1; ++i) {
        counts[i] = 0;
    }

    for (int i = 0; i < commands.count(); i++) {
        counts[commands[i]->getType()]++;
    }

    overview->reset();
    int total = 0;
#ifdef SK_DEBUG
    double totPercent = 0, tempSum = 0;
#endif
    for (int i = 0; i < LAST_DRAWTYPE_ENUM+1; ++i) {
        if (0 == counts[i]) {
            // if there were no commands of this type then they should've consumed no time
            SkASSERT(NULL == typeTimes || 0.0 == (*typeTimes)[i]);
            continue;
        }

        overview->append(SkDrawCommand::GetCommandString((DrawType) i));
        overview->append(": ");
        overview->appendS32(counts[i]);
        if (NULL != typeTimes && totTime >= 0.0) {
            overview->append(" - ");
            overview->appendf("%.2f", (*typeTimes)[i]/(float)numRuns);
            overview->append("ms");
            overview->append(" - ");
            double percent = 100.0*(*typeTimes)[i]/totTime;
            overview->appendf("%.2f", percent);
            overview->append("%");
#ifdef SK_DEBUG
            totPercent += percent;
            tempSum += (*typeTimes)[i];
#endif
        }
        overview->append("<br/>");
        total += counts[i];
    }
#ifdef SK_DEBUG
    if (NULL != typeTimes) {
        SkASSERT(SkScalarNearlyEqual(SkDoubleToScalar(totPercent),
                                     SkDoubleToScalar(100.0)));
        SkASSERT(SkScalarNearlyEqual(SkDoubleToScalar(tempSum),
                                     SkDoubleToScalar(totTime)));
    }
#endif

    if (totTime > 0.0) {
        overview->append("Total Time: ");
        overview->appendf("%.2f", totTime/(float)numRuns);
        overview->append("ms");
#ifdef SK_DEBUG
        overview->append(" ");
        overview->appendScalar(SkDoubleToScalar(totPercent));
        overview->append("% ");
#endif
        overview->append("<br/>");
    }

    SkString totalStr;
    totalStr.append("Total Draw Commands: ");
    totalStr.appendScalar(SkDoubleToScalar(total));
    totalStr.append("<br/>");
    overview->insert(0, totalStr);

    overview->append("<br/>");
    overview->append("SkPicture Width: ");
    overview->appendS32(pictureWidth());
    overview->append("px<br/>");
    overview->append("SkPicture Height: ");
    overview->appendS32(pictureHeight());
    overview->append("px");
}

void SkDebugger::getClipStackText(SkString* clipStack) {
    clipStack->set(fDebugCanvas->clipStackData());
}
