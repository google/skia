/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recorder_DEFINED
#define skgpu_Recorder_DEFINED

#include "experimental/graphite/src/TaskGraph.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class Context;
class Recording;

class Recorder final : public SkRefCnt {
public:
    Recorder(sk_sp<Context>);
    ~Recorder() override;

    void add(sk_sp<Task>);

    std::unique_ptr<Recording> snap();

protected:
private:
    sk_sp<Context> fContext;
    TaskGraph fGraph;
};

} // namespace skgpu

#endif // skgpu_Recorder_DEFINED
