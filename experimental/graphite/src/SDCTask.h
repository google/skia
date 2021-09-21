/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SDCTask_DEFINED
#define skgpu_SDCTask_DEFINED

#include "experimental/graphite/src/Task.h"

namespace skgpu {

class SDCTask final : public Task {
public:
    static sk_sp<SDCTask> Make();

    ~SDCTask() override;

private:
    SDCTask();
};

} // namespace skgpu

#endif // skgpu_SDCTask_DEFINED
