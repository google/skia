/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window_android.h"

#include "VulkanTestContext_android.h"

Window* Window::CreateNativeWindow(void* platformData) {
    Window_android* window = new Window_android();
    if (!window->init((android_app*)platformData)) {
        delete window;
        return nullptr;
    }
    return window;
}

static void handle_cmd(struct android_app* app, int32_t cmd);
static int32_t handle_input(struct android_app* app, AInputEvent* event);

bool Window_android::init(android_app* app) {
    SkASSERT(app);
    mApp = app;
    mApp->userData = this;
    mApp->onAppCmd = handle_cmd;
    mApp->onInputEvent = handle_input;
    return true;
}

void Window_android::setTitle(const char* title) {
    //todo
    SkDebugf("Title: %s", title);
}

bool Window_android::attach(BackEndType attachType, int msaaSampleCount) {
    if (kVulkan_BackendType != attachType) {
        return false;
    }

    mSampleCount = msaaSampleCount;

    // We delay the creation of fTestContext until Android informs us that
    // the native window is ready to use.
    return true;
}

void Window_android::initDisplay(ANativeWindow* window) {
    SkASSERT(window);
    ContextPlatformData_android platformData;
    platformData.fNativeWindow = window;
    fTestContext = VulkanTestContext::Create((void*)&platformData, mSampleCount);
}

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
    if (write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        SkDebugf("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}

void Window_android::inval() {
	android_app_write_cmd(mApp, APP_CMD_INVAL_WINDOW);
}

void Window_android::paintIfNeeded() {
    if (mApp->window || !mContentRect.isEmpty()) {
        this->onPaint();
    }
}

/**
 * Process the next main command.
 */
static void handle_cmd(struct android_app* app, int32_t cmd) {
    Window_android* window = (Window_android*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            SkASSERT(app->window);
            window->initDisplay(app->window);
            window->paintIfNeeded();
            break;
        case APP_CMD_WINDOW_RESIZED: {
            int width = ANativeWindow_getWidth(app->window);
            int height = ANativeWindow_getHeight(app->window);
            window->onResize(width, height);
            break;
        }
        case APP_CMD_CONTENT_RECT_CHANGED:
            window->setContentRect(app->contentRect.left, app->contentRect.top,
                                   app->contentRect.right, app->contentRect.bottom);
            window->paintIfNeeded();
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            window->detach();
            break;
        case APP_CMD_INVAL_WINDOW:
            window->paintIfNeeded();
            break;
    }
}

static Window::Key get_key(int32_t keycode) {
    static const struct {
        int32_t     fAndroidKey;
        Window::Key fWindowKey;
    } gPair[] = {
        { AKEYCODE_BACK, Window::kBack_Key },
        { AKEYCODE_VOLUME_UP, Window::kLeft_Key },
        { AKEYCODE_VOLUME_DOWN, Window::kRight_Key }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fAndroidKey == keycode) {
            return gPair[i].fWindowKey;
        }
    }
    return Window::kNONE_Key;
}

static Window::InputState get_action(int32_t action) {
    static const struct {
        int32_t            fAndroidAction;
        Window::InputState fInputState;
    } gPair[] = {
        { AKEY_STATE_DOWN, Window::kDown_InputState },
        { AKEY_STATE_UP, Window::kUp_InputState },
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fAndroidAction == action) {
            return gPair[i].fInputState;
        }
    }
    return Window::kMove_InputState;
}

static int32_t get_key_modifiers(AInputEvent* event) {
    static const struct {
        int32_t fAndroidState;
        int32_t fWindowModifier;
    } gPair[] = {
        { AMETA_SHIFT_ON, Window::kShift_ModifierKey },
        { AMETA_CTRL_ON, Window::kControl_ModifierKey },
    };

    int32_t metaState = AKeyEvent_getMetaState(event);
    int32_t modifiers = 0;

    if (AKeyEvent_getRepeatCount(event) == 0) {
    	modifiers |= Window::kFirstPress_ModifierKey;
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fAndroidState == metaState) {
            modifiers |= gPair[i].fWindowModifier;
        }
    }
    return modifiers;
}

/**
 * Process the next input event.
 */
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    Window_android* window = (Window_android*)app->userData;
    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_MOTION:
            break;
        case AINPUT_EVENT_TYPE_KEY:
            Window::Key key = get_key(AKeyEvent_getKeyCode(event));
            Window::InputState state = get_action(AKeyEvent_getAction(event));
            int32_t mod = get_key_modifiers(event);
            window->onKey(key, state, mod);
            return true; // eat all key events
    }
    return 0;
}
