/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/SkWindowContext.h"

#include "include/gpu/GrDirectContext.h"
#ifdef SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#endif

SkWindowContext::SkWindowContext(const SkDisplayParams& params)
        : fDisplayParams(params) {}

SkWindowContext::~SkWindowContext() {}
