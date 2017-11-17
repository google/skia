/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "HelloWorld.h"

#include "GrContext.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkTime.h"

using namespace sk_app;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new HelloWorld(argc, argv, platformData);
}

static void on_backend_created_func(void* userData) {
    HelloWorld* hw = reinterpret_cast<HelloWorld*>(userData);
    return hw->onBackendCreated();
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    HelloWorld* hw = reinterpret_cast<HelloWorld*>(userData);
    return hw->onPaint(canvas);
}

static bool on_key_handler(Window::Key key, Window::InputState state, uint32_t modifiers,
                           void* userData) {
    HelloWorld* hw = reinterpret_cast<HelloWorld*>(userData);
    return hw->onKey(key, state, modifiers);
}

static bool on_char_handler(SkUnichar c, uint32_t modifiers, void* userData) {
    HelloWorld* hw = reinterpret_cast<HelloWorld*>(userData);
    return hw->onChar(c, modifiers);
}

HelloWorld::HelloWorld(int argc, char** argv, void* platformData) {
    SkGraphics::Init();

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->setRequestedDisplayParams(DisplayParams());

    // register callbacks
    fCommands.attach(fWindow);
    fWindow->registerBackendCreatedFunc(on_backend_created_func, this);
    fWindow->registerPaintFunc(on_paint_handler, this);
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerCharFunc(on_char_handler, this);

    // add key-bindings ???

    fWindow->attach(Window::kNativeGL_BackendType);
}

HelloWorld::~HelloWorld() {
    fWindow->detach();
    delete fWindow;
}

void HelloWorld::updateTitle() {
    if (!fWindow || fWindow->sampleCount() < 0) {
        return;
    }

    fWindow->setTitle("Hello, World");
}

void HelloWorld::onBackendCreated() {
    this->updateTitle();
    fWindow->show();
    fWindow->inval();
}

void HelloWorld::onPaint(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    // TODO
    SkPaint p;
    p.setColor(SK_ColorCYAN);
    canvas->drawCircle(100, 50, 40, p);

    fCommands.drawHelp(canvas);
}

void HelloWorld::onIdle() {
    // Just re-paint continously
    fWindow->inval();
}

bool HelloWorld::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    return fCommands.onKey(key, state, modifiers);
}

bool HelloWorld::onChar(SkUnichar c, uint32_t modifiers) {
    return fCommands.onChar(c, modifiers);
}
