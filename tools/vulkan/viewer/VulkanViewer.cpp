/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "VulkanViewer.h"

#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkCommonFlags.h"

DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new VulkanViewer(argc, argv, platformData);
}

static bool on_key_handler(int key, bool keyDown, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onKey(key, keyDown);
}

static bool on_mouse_handler(int x, int y, bool mouseDown, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onMouse(x, y, mouseDown);
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onPaint(canvas);
}

VulkanViewer::VulkanViewer(int argc, char** argv, void* platformData) :
    fGMs(skiagm::GMRegistry::Head()){

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(Window::kVulkan_BackendType, 0, nullptr);

    // register callbacks
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerMouseFunc(on_mouse_handler, this);
    fWindow->registerPaintFunc(on_paint_handler, this);

    fWindow->setTitle("VulkanViewer");
    fWindow->show();
}

VulkanViewer::~VulkanViewer() {
    fWindow->detach();
    delete fWindow;
}

bool VulkanViewer::onKey(int key, bool keyDown) {
    if (keyDown) {
        fInputHandler.onKeyDown(key);
    } else {
        fInputHandler.onKeyUp(key);
    }
    return true;
}

bool VulkanViewer::onMouse(int x, int y, bool mouseDown) {
    if (mouseDown) {
        fInputHandler.onMouseDown(x, y);
    } else {
        fInputHandler.onMouseUp();
    }
    return true;
}

void VulkanViewer::onPaint(SkCanvas* canvas) {
    SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(nullptr));

    canvas->save();

    gm->draw(canvas);

    canvas->restore();
}

void VulkanViewer::onIdle(float dt) {
    if (fInputHandler.isKeyPressed('l') || fInputHandler.isKeyPressed('L')) {
        fGMs = fGMs->next();
    }
    fWindow->onPaint();

    fInputHandler.Update();
}
