/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_android_DEFINED
#define Window_android_DEFINED

#include <android/native_window_jni.h>

#include "../Window.h"
#include "surface_glue_android.h"

namespace sk_app {

struct ContextPlatformData_android {
    ANativeWindow* fNativeWindow;
};

class Window_android : public Window {
public:
    Window_android() : Window() {}
    ~Window_android() override {}

    bool init(SkiaAndroidApp* skiaAndroidApp);
    void initDisplay(ANativeWindow* window);
    void onDisplayDestroyed();

    const DisplayParams& getDisplayParams() override;
    void setTitle(const char*) override;
    void show() override {}

    bool attach(BackendType attachType, const DisplayParams& params) override;
    void onInval() override;
    void setUIState(const Json::Value& state) override;

    void paintIfNeeded();

    bool scaleContentToFit() const override { return true; }
    bool supportsContentRect() const override { return true; }
    SkRect getContentRect() override { return fContentRect; }
    void setContentRect(int l, int t, int r, int b) { fContentRect.set(l,t,r,b); }

private:
    // We need fNativeWindow for attaching with another backend.
    // (in that case, attach is called without initDisplay being called later)
    ANativeWindow* fNativeWindow = nullptr;
    SkiaAndroidApp* fSkiaAndroidApp = nullptr;
    SkRect fContentRect;
    DisplayParams fDisplayParams;
    BackendType fBackendType;
};

}   // namespace sk_app

#endif
