/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/android/Window_android.h"
#include "tools/window/WindowContext.h"
#include "tools/window/android/WindowContextFactory_android.h"

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

void Window_android::setTitle(const char* title) {
    fSkiaAndroidApp->setTitle(title);
}

void Window_android::setUIState(const char* state) {
    fSkiaAndroidApp->setUIState(state);
}

bool Window_android::attach(BackendType attachType) {
    fBackendType = attachType;

    // We delay the creation of fWindowContext until Android informs us that
    // the native window is ready to use.
    // The creation will be done in initDisplay, which is initiated by kSurfaceCreated event.
    return true;
}

void Window_android::initDisplay(ANativeWindow* window) {
    SkASSERT(window);
    switch (fBackendType) {
#ifdef SK_GL
        case kNativeGL_BackendType:
        default:
            fWindowContext = skwindow::MakeGLForAndroid(window, fRequestedDisplayParams);
            break;
#else
        default:
#endif
        case kRaster_BackendType:
            fWindowContext = skwindow::MakeRasterForAndroid(window, fRequestedDisplayParams);
            break;
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = skwindow::MakeVulkanForAndroid(window, fRequestedDisplayParams);
            break;
#if defined(SK_GRAPHITE)
        case kGraphiteVulkan_BackendType:
            fWindowContext = skwindow::MakeGraphiteVulkanForAndroid(window,
                                                                    fRequestedDisplayParams);
            break;
#endif
#endif
    }
    this->onBackendCreated();
}

void Window_android::onDisplayDestroyed() {
    detach();
}

void Window_android::onInval() {
    fSkiaAndroidApp->postMessage(Message(kContentInvalidated));
}

void Window_android::paintIfNeeded() {
    if (fWindowContext) { // Check if initDisplay has already been called
        onPaint();
    } else {
        markInvalProcessed();
    }
}

}   // namespace sk_app
