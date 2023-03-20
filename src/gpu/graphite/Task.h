/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Task_DEFINED
#define skgpu_graphite_Task_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class Texture;

class Task : public SkRefCnt {
public:
    ~Task() override;

    // Holds a render target and translation to use in the task's work, if necessary.
    struct ReplayTargetData {
        const Texture* fTarget;
        SkIVector fTranslation;
    };

    // Instantiate and prepare any Resources that must happen while the Task is still on the
    // Recorder.
    virtual bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) = 0;

    // Returns true on success; false on failure.
    virtual bool addCommands(Context*, CommandBuffer*, ReplayTargetData) = 0;

protected:
    Task();

private:
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Task_DEFINED
