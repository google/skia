/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_android_DEFINED
#define Window_android_DEFINED

#include "../Window.h"
#include <android_native_app_glue.h>

namespace sk_app {

enum {
    /**
     * Leave plenty of space between this item and the ones defined in the glue layer
     */
    APP_CMD_INVAL_WINDOW = 64,
};

class Window_android : public Window {
public:
    Window_android() : Window() {}
    ~Window_android() override {}

    bool init(android_app* app_state);
    void initDisplay(ANativeWindow* window);

    void setTitle(const char*) override;
    void show() override {}

    bool attach(BackEndType attachType, int msaaSampleCount, bool deepColor) override;
    void inval() override;

    void paintIfNeeded();

    bool scaleContentToFit() const override { return true; }
    bool supportsContentRect() const override { return true; }
    SkRect getContentRect() override { return mContentRect; }
    void setContentRect(int l, int t, int r, int b) { mContentRect.set(l,t,r,b); }

private:
    android_app* mApp = nullptr;
    SkRect mContentRect;
    int mSampleCount = 0;
};

}   // namespace sk_app

#endif
