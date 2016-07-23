/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef WindowContext_android_DEFINED
#define WindowContext_android_DEFINED

#include <android/native_window_jni.h>

namespace sk_app {

struct ContextPlatformData_android {
    ANativeWindow* fNativeWindow;
};

}   // namespace sk_app

#endif // WindowContext_android_DEFINED
