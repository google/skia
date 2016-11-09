/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import  <Cocoa/Cocoa.h>
#include "SkOSWindow_Mac.h"
#include "SkOSMenu.h"
#include "SkTypes.h"
#include "SkWindow.h"
#import  "SkNSView.h"
#import  "SkEventNotifier.h"
#define  kINVAL_NSVIEW_EventType "inval-nsview"

static_assert(SK_SUPPORT_GPU, "not_implemented_for_non_gpu_build");

SkOSWindow::SkOSWindow(void* hWnd) : fHWND(hWnd) {
    fInvalEventIsPending = false;
    fGLContext = NULL;
    fNotifier = [[SkEventNotifier alloc] init];
}
SkOSWindow::~SkOSWindow() {
    [(SkEventNotifier*)fNotifier release];
}

void SkOSWindow::onHandleInval(const SkIRect& r) {
    if (!fInvalEventIsPending) {
        fInvalEventIsPending = true;
        (new SkEvent(kINVAL_NSVIEW_EventType, this->getSinkID()))->post();
    }
}

bool SkOSWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(kINVAL_NSVIEW_EventType)) {
        fInvalEventIsPending = false;
        const SkIRect& r = this->getDirtyBounds();
        [(SkNSView*)fHWND postInvalWithRect:&r];
        [(NSOpenGLContext*)fGLContext update];
        return true;
    }
    if ([(SkNSView*)fHWND onHandleEvent:evt]) {
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

bool SkOSWindow::onDispatchClick(int x, int y, Click::State state, void* owner,
                                 unsigned modi) {
    return this->INHERITED::onDispatchClick(x, y, state, owner, modi);
}

void SkOSWindow::onSetTitle(const char title[]) {
    [(SkNSView*)fHWND setSkTitle:title];
}

void SkOSWindow::onAddMenu(const SkOSMenu* menu) {
    [(SkNSView*)fHWND onAddMenu:menu];
}

void SkOSWindow::onUpdateMenu(const SkOSMenu* menu) {
    [(SkNSView*)fHWND onUpdateMenu:menu];
}

bool SkOSWindow::attach(SkBackEndTypes attachType, int sampleCount, bool /*deepColor*/,
                        AttachmentInfo* info) {
    return [(SkNSView*)fHWND attach:attachType withMSAASampleCount:sampleCount andGetInfo:info];
}

void SkOSWindow::release() {
    [(SkNSView*)fHWND detach];
}

void SkOSWindow::present() {
    [(SkNSView*)fHWND present];
}

void SkOSWindow::closeWindow() {
    [[(SkNSView*)fHWND window] close];
}

void SkOSWindow::setVsync(bool enable) {
    [(SkNSView*)fHWND setVSync:enable];
}

bool SkOSWindow::makeFullscreen() {
    NSScreen* _Nullable screen = [NSScreen mainScreen];
    if (screen) {
        [(SkNSView*)fHWND enterFullScreenMode:(NSScreen* _Nonnull)screen withOptions:nil];
    }
    return true;
}

