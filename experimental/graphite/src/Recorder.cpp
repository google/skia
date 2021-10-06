/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Recorder.h"

#include "experimental/graphite/src/Recording.h"

namespace skgpu {

Recorder::Recorder() {}
Recorder::~Recorder() {}

void Recorder::add(sk_sp<Task> task) {
    fGraph.add(std::move(task));
}

std::unique_ptr<Recording> Recorder::snap() {
    // TODO: transmute the task graph into the Recording's primitives
    fGraph.reset();
    return std::unique_ptr<Recording>(new Recording);
}

} // namespace skgpu
