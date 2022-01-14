/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/WindowContext.h"

#include "include/gpu/GrDirectContext.h"
#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#endif

namespace sk_app {

WindowContext::WindowContext(const DisplayParams& params)
        : fDisplayParams(params) {}

WindowContext::~WindowContext() {}

}   //namespace sk_app
