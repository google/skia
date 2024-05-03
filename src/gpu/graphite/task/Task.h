/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_Task_DEFINED
#define skgpu_graphite_task_Task_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;
class Texture;

class Task : public SkRefCnt {
public:
    // Holds a render target and translation to use in the task's work, if necessary.
    struct ReplayTargetData {
        const Texture* fTarget;
        SkIVector fTranslation;
    };

    enum class Status {
        // The task step (prepareResources or addCommands) succeeded, proceed to the next task.
        // If the Recording is replayed, this task should be executed again.
        kSuccess,
        // The task step succeeded, but it was a one-time-only operation and should be removed from
        // the task list. If this is returned from prepareResources(), the task is removed before
        // addCommands() will ever be called. If this is returned from addCommands(), it will not
        // be part of any replayed Recording, but any added commands from the first call will be
        // executed once.
        //
        // NOTE: If a task step needs to be conditionally processed but repeatable, it should
        // internally skip work and still return kSuccess instead of kDiscard.
        kDiscard,
        // The step failed and cannot be recovered so the Recording is invalidated.
        kFail
    };

    // Instantiate and prepare any Resources that must happen while the Task is still on the
    // Recorder.
    virtual Status prepareResources(ResourceProvider*,
                                    ScratchResourceManager*,
                                    const RuntimeEffectDictionary*) = 0;

    // Returns true on success; false on failure.
    virtual Status addCommands(Context*, CommandBuffer*, ReplayTargetData) = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_Task_DEFINED
