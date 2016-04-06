/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window.h"

#include "SkSurface.h"
#include "SkCanvas.h"
#include "VulkanTestContext.h"

static bool default_key_func(int key, bool down, void* userData) {
    return false;
}

static bool default_mouse_func(int x, int y, bool down, void* userData) {
    return false;
}

static void default_paint_func(SkCanvas*, void* userData) {}

Window::Window() : fKeyFunc(default_key_func)
                 , fMouseFunc(default_mouse_func)
                 , fPaintFunc(default_paint_func) {
}

void Window::detach() {
    delete fTestContext;
    fTestContext = nullptr;
}

void Window::onPaint() {
    SkSurface* backbuffer = fTestContext->getBackbufferSurface();
    if (backbuffer) {
        // draw into the canvas of this surface
        SkCanvas* canvas = backbuffer->getCanvas();

        fPaintFunc(canvas, fPaintUserData);

        canvas->flush();

        fTestContext->swapBuffers();
    }

}


void Window::onSize() {
    fTestContext->resize();
}
