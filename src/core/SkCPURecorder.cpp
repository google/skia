/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCPURecorder.h"

#include "include/core/SkTypes.h"
#include "src/core/SkCPUContextImpl.h"

#include <memory>

namespace skcpu {

Recorder* Recorder::TODO() {
    static Recorder* gRecorder = ContextImpl::TODO()->makeRecorder().release();
    return gRecorder;
}

}  // namespace skcpu
