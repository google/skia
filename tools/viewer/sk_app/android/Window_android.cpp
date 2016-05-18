/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window_android.h"

#include "VulkanWindowContext_android.h"

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    Window_android* window = new Window_android();
    if (!window->init((SkiaAndroidApp*)platformData)) {
        delete window;
        return nullptr;
    }
    return window;
}

bool Window_android::init(SkiaAndroidApp* skiaAndroidApp) {
    SkASSERT(skiaAndroidApp);
    fSkiaAndroidApp = skiaAndroidApp;
    fSkiaAndroidApp->fWindow = this;
    return true;
}

const DisplayParams& Window_android::getDisplayParams() {
    if (fWindowContext) {
        return fWindowContext->getDisplayParams();
    } else {
        // fWindowContext doesn't exist because we haven't
        // initDisplay yet.
        return fDisplayParams;
    }
}

void Window_android::setTitle(const char* title) {
    fSkiaAndroidApp->setTitle(title);
}

bool Window_android::attach(BackEndType attachType, const DisplayParams& params) {
    if (kVulkan_BackendType != attachType) {
        return false;
    }

    fDisplayParams = params;

    // We delay the creation of fTestContext until Android informs us that
    // the native window is ready to use.
    return true;
}

void Window_android::initDisplay(ANativeWindow* window) {
    SkASSERT(window);
    ContextPlatformData_android platformData;
    platformData.fNativeWindow = window;
    fWindowContext = VulkanWindowContext::Create((void*)&platformData, fDisplayParams);
}

void Window_android::onDisplayDestroyed() {
    detach();
}

void Window_android::inval() {
    fSkiaAndroidApp->inval();
}

}   // namespace sk_app
