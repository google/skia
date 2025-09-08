/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipelineHandle_DEFINED
#define skgpu_graphite_GraphicsPipelineHandle_DEFINED

#include "include/core/SkRefCnt.h"

#include <variant>

namespace skgpu::graphite {

class GraphicsPipeline;
class PipelineCreationTask;

/*
 * The PipelineHandle holds a ref to either a Pipeline or the Task that will create the Pipeline.
 * In the latter case, ResourceProvider::resolveHandle can be used to wait for the Task to
 * complete. How this works is:
 *   At Recorder::snap time, the DrawPass will create GraphicsPipelineHandles and will
 *       kick off all the tasks (using ResourceProvider::kickOffTask)
 *   Upon Context::insertRecording, all the Handles will be exchanged for GraphicsPipelines in
 *       the DrawPass::addResourceRefs. This will also release any Tasks being held by the Handles.
 *
 * Note that the Tasks lock the generated Pipelines in the cache until they are deleted. This avoids
 * any race conditions where a Pipeline could be purged between when it was created on a thread
 * and when it was actually requested by a DrawPass.
 */
class GraphicsPipelineHandle {
private:
    friend class PipelineManager;  // for ctors

    GraphicsPipelineHandle(sk_sp<PipelineCreationTask> task);

    GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline);

    std::variant<sk_sp<PipelineCreationTask>, sk_sp<GraphicsPipeline>> fTaskOrPipeline;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipelineHandle_DEFINED
