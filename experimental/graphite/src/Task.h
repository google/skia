/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Task_DEFINED
#define skgpu_graphite_Task_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class CommandBuffer;
class ResourceProvider;

class Task : public SkRefCnt {
public:
    ~Task() override;

    // Returns true on success; false on failure.
    virtual bool addCommands(ResourceProvider*, CommandBuffer*) = 0;

protected:
    Task();

private:
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Task_DEFINED
