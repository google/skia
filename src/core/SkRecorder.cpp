/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRecorder.h"

#include "include/cpu/Recorder.h"

SkRecorder* SkRecorder::TODO() {
    return skcpu::Recorder::TODO();
}
