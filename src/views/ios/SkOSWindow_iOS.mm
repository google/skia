/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <UIKit/UIKit.h>
#include "SkCanvas.h"
#include "SkGraphics.h"
#import "SkEventNotifier.h"
#include "SkOSMenu.h"
#include "SkTime.h"
#include "SkTypes.h"
#import "SkUIView.h"
#include "SkWindow.h"

#define kINVAL_UIVIEW_EventType "inval-uiview"

SkOSWindow::SkOSWindow(void* hWnd) : fHWND(hWnd) {
    fInvalEventIsPending = false;
    fNotifier = [[SkEventNotifier alloc] init];
}
SkOSWindow::~SkOSWindow() {
    [(SkEventNotifier*)fNotifier release];
}

void SkOSWindow::onHandleInval(const SkIRect& r) {
    if (!fInvalEventIsPending) {
        fInvalEventIsPending = true;
        (new SkEvent(kINVAL_UIVIEW_EventType, this->getSinkID()))->post();
    }
}

bool SkOSWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(kINVAL_UIVIEW_EventType)) {
        fInvalEventIsPending = false;
        const SkIRect& r = this->getDirtyBounds();
        [(SkUIView*)fHWND postInvalWithRect:&r];
        return true;
    }
    if ([(SkUIView*)fHWND onHandleEvent:evt]) {
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void SkOSWindow::onSetTitle(const char title[]) {
    [(SkUIView*)fHWND setSkTitle:title];
}

void SkOSWindow::onAddMenu(const SkOSMenu* menu) {
    [(SkUIView*)fHWND onAddMenu:menu];
}

void SkOSWindow::onUpdateMenu(const SkOSMenu* menu) {
    [(SkUIView*)fHWND onUpdateMenu:menu];
}

bool SkOSWindow::attach(SkBackEndTypes /* attachType */,
                        int /* msaaSampleCount */,
                        bool /* deepColor */,
                        AttachmentInfo* info) {
    [(SkUIView*)fHWND getAttachmentInfo:info];
    bool success = true;
    return success;
}

void SkOSWindow::release() {}

void SkOSWindow::present() {
}
