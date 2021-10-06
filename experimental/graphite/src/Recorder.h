/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recorder_DEFINED
#define skgpu_Recorder_DEFINED

#include "experimental/graphite/src/TaskGraph.h"

namespace skgpu {

class Recording;

class Recorder {
public:
    Recorder();
    ~Recorder();

    void add(sk_sp<Task>);

    std::unique_ptr<Recording> snap();

protected:
private:
    TaskGraph fGraph;
};

} // namespace skgpu

#endif // skgpu_Recorder_DEFINED

