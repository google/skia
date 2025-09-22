Add enum class `skgpu::graphite::MarkFrameBoundary` to be used to specify whether a submission is the last logical submission for a frame.

Add struct `skgpu::graphite::SubmitInfo` to hold metadata used for submitting workloads for execution. Allow specifying through `skgpu::graphite::SubmitInfo` whether submission is a frame boundary (last logical submission for a frame) and frameID (uint64_t) with default values to match prior behavior.

Use struct `skgpu::graphite::SubmitInfo` in `skgpu::graphite::QueueManager::submitToGpu`, `skgpu::graphite::QueueManager::onSubmitToGpu` and all derived classes.
