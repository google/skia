/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/android/surface_glue_android.h"

#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/looper.h>
#include <android/native_window_jni.h>

#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "tools/ResourceFactory.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/android/Window_android.h"


namespace sk_app {

static void config_resource_mgr(JNIEnv* env, jobject assetManager) {
    static AAssetManager* gAAssetManager = nullptr;
    SkASSERT(assetManager);
    gAAssetManager = AAssetManager_fromJava(env, assetManager);
    SkASSERT(gAAssetManager);
    gResourceFactory = [](const char* resource) -> sk_sp<SkData> {
        if (!gAAssetManager) {
            return nullptr;
        }
        SkString path = SkStringPrintf("resources/%s", resource);
        AAsset* asset = AAssetManager_open(gAAssetManager, path.c_str(), AASSET_MODE_STREAMING);
        if (!asset) {
            return nullptr;
        }
        size_t size = SkToSizeT(AAsset_getLength(asset));
        sk_sp<SkData> data = SkData::MakeUninitialized(size);
        (void)AAsset_read(asset, data->writable_data(), size);
        AAsset_close(asset);
        return data;
    };
}

static const int LOOPER_ID_MESSAGEPIPE = 1;

static const std::unordered_map<int, skui::Key> ANDROID_TO_WINDOW_KEYMAP({
    {AKEYCODE_SOFT_LEFT,  skui::Key::kLeft },
    {AKEYCODE_SOFT_RIGHT, skui::Key::kRight}
});

static const std::unordered_map<int, skui::InputState> ANDROID_TO_WINDOW_STATEMAP({
    {AMOTION_EVENT_ACTION_DOWN,         skui::InputState::kDown },
    {AMOTION_EVENT_ACTION_POINTER_DOWN, skui::InputState::kDown },
    {AMOTION_EVENT_ACTION_UP,           skui::InputState::kUp   },
    {AMOTION_EVENT_ACTION_POINTER_UP,   skui::InputState::kUp   },
    {AMOTION_EVENT_ACTION_MOVE,         skui::InputState::kMove },
    {AMOTION_EVENT_ACTION_CANCEL,       skui::InputState::kUp   },
});

SkiaAndroidApp::SkiaAndroidApp(JNIEnv* env, jobject androidApp) {
    env->GetJavaVM(&fJavaVM);
    fAndroidApp = env->NewGlobalRef(androidApp);
    jclass cls = env->GetObjectClass(fAndroidApp);
    fSetTitleMethodID = env->GetMethodID(cls, "setTitle", "(Ljava/lang/String;)V");
    fSetStateMethodID = env->GetMethodID(cls, "setState", "(Ljava/lang/String;)V");
    fNativeWindow = nullptr;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

SkiaAndroidApp::~SkiaAndroidApp() {
    fPThreadEnv->DeleteGlobalRef(fAndroidApp);
    if (fWindow) {
        fWindow->detach();
    }
    if (fNativeWindow) {
        ANativeWindow_release(fNativeWindow);
        fNativeWindow = nullptr;
    }
    if (fApp) {
        delete fApp;
    }
}

void SkiaAndroidApp::setTitle(const char* title) const {
    jstring titleString = fPThreadEnv->NewStringUTF(title);
    fPThreadEnv->CallVoidMethod(fAndroidApp, fSetTitleMethodID, titleString);
    fPThreadEnv->DeleteLocalRef(titleString);
}

void SkiaAndroidApp::setUIState(const char* state) const {
    jstring jstr = fPThreadEnv->NewStringUTF(state);
    fPThreadEnv->CallVoidMethod(fAndroidApp, fSetStateMethodID, jstr);
    fPThreadEnv->DeleteLocalRef(jstr);
}

void SkiaAndroidApp::postMessage(const Message& message) const {
    SkDEBUGCODE(auto writeSize =) write(fPipes[1], &message, sizeof(message));
    SkASSERT(writeSize == sizeof(message));
}

void SkiaAndroidApp::readMessage(Message* message) const {
    SkDEBUGCODE(auto readSize =) read(fPipes[0], message, sizeof(Message));
    SkASSERT(readSize == sizeof(Message));
}

int SkiaAndroidApp::message_callback(int fd, int events, void* data) {
    auto skiaAndroidApp = (SkiaAndroidApp*)data;
    Message message;
    skiaAndroidApp->readMessage(&message);
    SkASSERT(message.fType != kUndefined);

    switch (message.fType) {
        case kDestroyApp: {
            delete skiaAndroidApp;
            pthread_exit(nullptr);
        }
        case kContentInvalidated: {
            ((Window_android*)skiaAndroidApp->fWindow)->paintIfNeeded();
            break;
        }
        case kSurfaceCreated: {
            SkASSERT(!skiaAndroidApp->fNativeWindow && message.fNativeWindow);
            skiaAndroidApp->fNativeWindow = message.fNativeWindow;
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            window_android->initDisplay(skiaAndroidApp->fNativeWindow);
            ((Window_android*)skiaAndroidApp->fWindow)->paintIfNeeded();
            break;
        }
        case kSurfaceChanged: {
            SkASSERT(message.fNativeWindow);
            int width = ANativeWindow_getWidth(skiaAndroidApp->fNativeWindow);
            int height = ANativeWindow_getHeight(skiaAndroidApp->fNativeWindow);
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            if (message.fNativeWindow != skiaAndroidApp->fNativeWindow) {
                window_android->onDisplayDestroyed();
                ANativeWindow_release(skiaAndroidApp->fNativeWindow);
                skiaAndroidApp->fNativeWindow = message.fNativeWindow;
                window_android->initDisplay(skiaAndroidApp->fNativeWindow);
            }
            window_android->onResize(width, height);
            window_android->paintIfNeeded();
            break;
        }
        case kSurfaceDestroyed: {
            if (skiaAndroidApp->fNativeWindow) {
                auto window_android = (Window_android*)skiaAndroidApp->fWindow;
                window_android->onDisplayDestroyed();
                ANativeWindow_release(skiaAndroidApp->fNativeWindow);
                skiaAndroidApp->fNativeWindow = nullptr;
            }
            break;
        }
        case kKeyPressed: {
            auto it = ANDROID_TO_WINDOW_KEYMAP.find(message.fKeycode);
            SkASSERT(it != ANDROID_TO_WINDOW_KEYMAP.end());
            // No modifier is supported so far
            skiaAndroidApp->fWindow->onKey(it->second, skui::InputState::kDown, skui::ModifierKey::kNone);
            skiaAndroidApp->fWindow->onKey(it->second, skui::InputState::kUp, skui::ModifierKey::kNone);
            break;
        }
        case kTouched: {
            auto it = ANDROID_TO_WINDOW_STATEMAP.find(message.fTouchState);
            if (it != ANDROID_TO_WINDOW_STATEMAP.end()) {
                skiaAndroidApp->fWindow->onTouch(message.fTouchOwner, it->second, message.fTouchX,
                                                 message.fTouchY);
            } else {
                SkDebugf("Unknown Touch State: %d\n", message.fTouchState);
            }
            break;
        }
        case kUIStateChanged: {
            skiaAndroidApp->fWindow->onUIStateChanged(*message.stateName, *message.stateValue);
            delete message.stateName;
            delete message.stateValue;
            break;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

void* SkiaAndroidApp::pthread_main(void* arg) {
    SkDebugf("pthread_main begins");

    auto skiaAndroidApp = (SkiaAndroidApp*)arg;

    // Because JNIEnv is thread sensitive, we need AttachCurrentThread to set our fPThreadEnv
    skiaAndroidApp->fJavaVM->AttachCurrentThread(&(skiaAndroidApp->fPThreadEnv), nullptr);

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    pipe(skiaAndroidApp->fPipes);
    ALooper_addFd(looper, skiaAndroidApp->fPipes[0], LOOPER_ID_MESSAGEPIPE, ALOOPER_EVENT_INPUT,
                  message_callback, skiaAndroidApp);

    static const char* gCmdLine[] = {
        "viewer",
        // TODO: figure out how to use am start with extra params to pass in additional arguments at
        // runtime. Or better yet make an in app switch to enable
        // "--atrace",
    };

    skiaAndroidApp->fApp = Application::Create(std::size(gCmdLine),
                                               const_cast<char**>(gCmdLine),
                                               skiaAndroidApp);

    while (true) {
        int ident = 0;
        while (ident == ALOOPER_POLL_CALLBACK) {
            ident = ALooper_pollOnce(0, nullptr, nullptr, nullptr);
        }

        if (ident >= 0) {
            SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        } else {
            skiaAndroidApp->fApp->onIdle();
        }
    }
}

extern "C"  // extern "C" is needed for JNI (although the method itself is in C++)
    JNIEXPORT jlong JNICALL
    Java_org_skia_viewer_ViewerApplication_createNativeApp(JNIEnv* env,
                                                           jobject application,
                                                           jobject assetManager) {
    config_resource_mgr(env, assetManager);
    SkiaAndroidApp* skiaAndroidApp = new SkiaAndroidApp(env, application);
    return (jlong)((size_t)skiaAndroidApp);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerApplication_destroyNativeApp(
    JNIEnv* env, jobject application, jlong handle) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    skiaAndroidApp->postMessage(Message(kDestroyApp));
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceCreated(
    JNIEnv* env, jobject activity, jlong handle, jobject surface) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kSurfaceCreated);
    message.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceChanged(
    JNIEnv* env, jobject activity, jlong handle, jobject surface) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kSurfaceChanged);
    message.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceDestroyed(
    JNIEnv* env, jobject activity, jlong handle) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    skiaAndroidApp->postMessage(Message(kSurfaceDestroyed));
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onKeyPressed(JNIEnv* env,
                                                                                   jobject activity,
                                                                                   jlong handle,
                                                                                   jint keycode) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kKeyPressed);
    message.fKeycode = keycode;
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onTouched(
    JNIEnv* env, jobject activity, jlong handle, jint owner, jint state, jfloat x, jfloat y) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kTouched);
    message.fTouchOwner = owner;
    message.fTouchState = state;
    message.fTouchX = x;
    message.fTouchY = y;
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onUIStateChanged(
    JNIEnv* env, jobject activity, jlong handle, jstring stateName, jstring stateValue) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kUIStateChanged);
    const char* nameChars = env->GetStringUTFChars(stateName, nullptr);
    const char* valueChars = env->GetStringUTFChars(stateValue, nullptr);
    message.stateName = new SkString(nameChars);
    message.stateValue = new SkString(valueChars);
    skiaAndroidApp->postMessage(message);
    env->ReleaseStringUTFChars(stateName, nameChars);
    env->ReleaseStringUTFChars(stateValue, valueChars);
}

}  // namespace sk_app
