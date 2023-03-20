/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/WindowContext.h"

#include "include/gpu/GrDirectContext.h"
#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#endif

namespace sk_app {

WindowContext::WindowContext(const DisplayParams& params)
        : fDisplayParams(params) {}

WindowContext::~WindowContext() {}

}   //namespace sk_app
