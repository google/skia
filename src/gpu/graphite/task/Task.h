/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_Task_DEFINED
#define skgpu_graphite_task_Task_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/Token.h"
#include "src/gpu/graphite/DebugUtils.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"

#include <functional>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class GraphicsPipeline;
class ResourceProvider;
class ScratchResourceManager;
class Texture;
class TextureProxy;

class Task : public SkRefCnt {
public:
    // Holds a render target and translation to use in the task's work, if necessary.
    struct ReplayTargetData {
        const Texture* fTarget;
        SkIVector fTranslation;
        SkIRect fClip;
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
                                    sk_sp<const RuntimeEffectDictionary>) = 0;

    // Returns true on success; false on failure.
    virtual Status addCommands(Context*, CommandBuffer*, ReplayTargetData) = 0;

    // Visit all pipelines or proxies until `visitor` returns false to end early. By default assume
    // the task uses none. Because these are functions are virtual, they cannot be templated.
    //
    // WARNING: These visit functions will visit all tasks and their children, including revisiting
    // anything that was added multiple times. Ideally the task graph should be visited after
    // prepareResources() has been called because that will clean out cycles and re-references.
    virtual bool visitPipelines(const std::function<bool(const GraphicsPipeline*)>& visitor) {
        return true;
    }

    virtual bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                                                       bool readsOnly) {
        return true;
    }

#if defined(SK_DUMP_TASKS)
    virtual void dump(int index, const char* prefix = "") const {
        const char* taskName = this->getTaskName();
        if (index >= 0) {
            SkDebugf("%s%d: %s\n", prefix, index, taskName);
        } else {
            SkDebugf("%s%s\n", prefix, taskName);
        }
    }

    skgpu::Token fFlushToken = skgpu::Token::InvalidToken();

    virtual const char* getTaskName() const { return "Base Task (unknown type)"; }
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_Task_DEFINED
