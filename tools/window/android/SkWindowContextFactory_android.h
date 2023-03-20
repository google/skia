/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_android_DEFINED
#define WindowContextFactory_android_DEFINED

#include <android/native_window_jni.h>

#include <memory>

class SkWindowContext;
struct SkDisplayParams;

namespace window_context_factory {

std::unique_ptr<SkWindowContext> MakeVulkanForAndroid(ANativeWindow*, const SkDisplayParams&);

std::unique_ptr<SkWindowContext> MakeGLForAndroid(ANativeWindow*, const SkDisplayParams&);

}  // namespace window_context_factory

#endif
