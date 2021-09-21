/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/SDCTask.h"

namespace skgpu {

sk_sp<SDCTask> SDCTask::Make() {
    return sk_sp<SDCTask>(new SDCTask());
}

SDCTask::SDCTask() {}
SDCTask::~SDCTask() {}

} // namespace skgpu
