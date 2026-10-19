/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/cpu/Recorder.h"

#include "include/core/SkTypes.h"
#include "src/capture/SkCaptureManager.h"

#include <memory>

namespace skcpu {

Recorder* Recorder::TODO() {
    static Recorder* gRecorder = std::make_unique<Recorder>().release();
    return gRecorder;
}

void Recorder::createCaptureBreakpoint(SkSurface*) {
}

}  // namespace skcpu
