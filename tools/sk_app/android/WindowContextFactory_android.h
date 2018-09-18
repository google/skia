
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_android_DEFINED
#define WindowContextFactory_android_DEFINED

#include <android/native_window_jni.h>


namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

WindowContext* NewVulkanForAndroid(ANativeWindow*, const DisplayParams&);

WindowContext* NewGLForAndroid(ANativeWindow*, const DisplayParams&);

WindowContext* NewRasterForAndroid(ANativeWindow*, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
