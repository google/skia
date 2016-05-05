/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window.h"

#include "SkSurface.h"
#include "SkCanvas.h"
#include "VulkanWindowContext.h"

namespace sk_app {

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

static void default_paint_func(SkCanvas*, void* userData) {}

Window::Window() : fCharFunc(default_char_func)
                 , fKeyFunc(default_key_func)
                 , fMouseFunc(default_mouse_func)
                 , fPaintFunc(default_paint_func) {
}

void Window::detach() {
    delete fWindowContext;
    fWindowContext = nullptr;
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

void Window::onPaint() {
    SkSurface* backbuffer = fWindowContext->getBackbufferSurface();
    if (backbuffer) {
        // draw into the canvas of this surface
        SkCanvas* canvas = backbuffer->getCanvas();

        fPaintFunc(canvas, fPaintUserData);

        canvas->flush();

        fWindowContext->swapBuffers();
    } else {
        // try recreating testcontext
    }

}

void Window::onResize(uint32_t w, uint32_t h) {
    fWidth = w;
    fHeight = h;
    fWindowContext->resize(w, h);
}

}   // namespace sk_app
