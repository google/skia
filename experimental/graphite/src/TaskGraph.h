/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_TaskGraph_DEFINED
#define skgpu_TaskGraph_DEFINED

#include <vector>
#include "experimental/graphite/src/Task.h"

namespace skgpu {

class TaskGraph {
public:
    TaskGraph();
    ~TaskGraph();

    void add(sk_sp<Task>);
    void reset();

protected:
private:
    std::vector<sk_sp<Task>> fTasks;
};

} // namespace skgpu

#endif // skgpu_TaskGraph_DEFINED
