/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window.h"

#include "SkSurface.h"
#include "SkCanvas.h"
#include "WindowContext.h"

namespace sk_app {

static void default_backend_created_func(void* userData) {}

static bool default_char_func(SkUnichar c, uint32_t modifiers, void* userData) {
    return false;
}

static bool default_key_func(Window::Key key, Window::InputState state, uint32_t modifiers, 
                             void* userData) {
    return false;
}

static bool default_mouse_func(int x, int y, Window::InputState state, uint32_t modifiers, 
                               void* userData) {
    return false;
}

static bool default_mouse_wheel_func(float delta, uint32_t modifiers, void* userData) {
    return false;
}

static bool default_touch_func(intptr_t owner, Window::InputState state, float x, float y,
                               void* userData) {
    return false;
}

static void default_ui_state_changed_func(
        const SkString& stateName, const SkString& stateValue, void* userData) {}

static void default_paint_func(SkCanvas*, void* userData) {}

Window::Window() : fBackendCreatedFunc(default_backend_created_func)
                 , fCharFunc(default_char_func)
                 , fKeyFunc(default_key_func)
                 , fMouseFunc(default_mouse_func)
                 , fMouseWheelFunc(default_mouse_wheel_func)
                 , fTouchFunc(default_touch_func)
                 , fUIStateChangedFunc(default_ui_state_changed_func)
                 , fPaintFunc(default_paint_func) {
}

void Window::detach() {
    delete fWindowContext;
    fWindowContext = nullptr;
}

void Window::onBackendCreated() {
    fBackendCreatedFunc(fBackendCreatedUserData);
}

bool Window::onChar(SkUnichar c, uint32_t modifiers) {
    return fCharFunc(c, modifiers, fCharUserData);
}

bool Window::onKey(Key key, InputState state, uint32_t modifiers) {
    return fKeyFunc(key, state, modifiers, fKeyUserData);
}

bool Window::onMouse(int x, int y, InputState state, uint32_t modifiers) {
    return fMouseFunc(x, y, state, modifiers, fMouseUserData);
}

bool Window::onMouseWheel(float delta, uint32_t modifiers) {
    return fMouseWheelFunc(delta, modifiers, fMouseWheelUserData);
}

bool Window::onTouch(intptr_t owner, InputState state, float x, float y) {
    return fTouchFunc(owner, state, x, y, fTouchUserData);
}

void Window::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    return fUIStateChangedFunc(stateName, stateValue, fUIStateChangedUserData);
}

void Window::onPaint() {
    if (!fWindowContext) {
        return;
    }
    markInvalProcessed();
    sk_sp<SkSurface> backbuffer = fWindowContext->getBackbufferSurface();
    if (backbuffer) {
        // draw into the canvas of this surface
        SkCanvas* canvas = backbuffer->getCanvas();

        fPaintFunc(canvas, fPaintUserData);

        canvas->flush();

        fWindowContext->swapBuffers();
    } else {
        printf("no backbuffer!?\n");
        // try recreating testcontext
    }
}

void Window::onResize(int w, int h) {
    if (!fWindowContext) {
        return;
    }
    fWindowContext->resize(w, h);
}

int Window::width() {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->width();
}

int Window::height() {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->height();
}

void Window::setRequestedDisplayParams(const DisplayParams& params) {
    fRequestedDisplayParams = params;
    if (fWindowContext) {
        fWindowContext->setDisplayParams(fRequestedDisplayParams);
    }
}

int Window::sampleCount() const {
    if (!fWindowContext) {
        return -1;
    }
    return fWindowContext->sampleCount();
}

int Window::stencilBits() const {
    if (!fWindowContext) {
        return -1;
    }
    return fWindowContext->stencilBits();
}

const GrContext* Window::getGrContext() const {
    if (!fWindowContext) {
        return nullptr;
    }
    return fWindowContext->getGrContext();
}

void Window::inval() {
    if (!fWindowContext) {
        return;
    }
    if (!fIsContentInvalidated) {
        fIsContentInvalidated = true;
        onInval();
    }
}

void Window::markInvalProcessed() {
    fIsContentInvalidated = false;
}

}   // namespace sk_app
