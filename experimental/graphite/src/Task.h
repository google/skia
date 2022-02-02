/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Task_DEFINED
#define skgpu_Task_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class CommandBuffer;
class ResourceProvider;

class Task : public SkRefCnt {
public:
    ~Task() override;

    virtual void addCommands(ResourceProvider*, CommandBuffer*) = 0;

protected:
    Task();

private:
};

} // namespace skgpu

#endif // skgpu_Task_DEFINED
