/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Recorder.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Recording.h"

namespace skgpu {

Recorder::Recorder(sk_sp<Context> context) : fContext(std::move(context)) {}
Recorder::~Recorder() {}

void Recorder::add(sk_sp<Task> task) {
    fGraph.add(std::move(task));
}

std::unique_ptr<Recording> Recorder::snap() {
    // TODO: need to create a CommandBuffer from the Tasks
    fGraph.reset();
    return std::unique_ptr<Recording>(new Recording(nullptr));
}

} // namespace skgpu
