/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SampleSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkStream.h"

SampleSlide::SampleSlide(const SkViewFactory* factory) : fViewFactory(factory) {
    SkView* view = (*factory)();
    SampleCode::RequestTitle(view, &fName);
    view->unref();
}

SampleSlide::~SampleSlide() {}

void SampleSlide::draw(SkCanvas* canvas) {
    fView->draw(canvas);
}

void SampleSlide::load(SkScalar winWidth, SkScalar winHeight) {
    fView = (*fViewFactory)();
    fView->setVisibleP(true);
    fView->setClipToBounds(false);
    fView->setSize(winWidth, winHeight);
}

void SampleSlide::unload() {
    fView->unref();
    fView = nullptr;
}
