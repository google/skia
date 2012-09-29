
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugger.h"

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
    picture->draw(fDebugCanvas);
    fIndex = fDebugCanvas->getSize() - 1;
    SkRefCnt_SafeAssign(fPicture, picture);
}

SkPicture* SkDebugger::makePicture() {
    SkSafeUnref(fPicture);
    fPicture = new SkPicture();
    SkCanvas* canvas = fPicture->beginRecording(fPictureWidth, fPictureHeight);
    fDebugCanvas->draw(canvas);
    fPicture->endRecording();
    return fPicture;
}
