Graphite's `Context` now provides an interface to report the GPU time spent processing a recording. The client provides
a different finished proc of type `skgpu::graphite::GpuFinishedWithStatsProc` using
`skgpu::graphite::InsertRecordingInfo::fFinishedWithStatsProc` and sets the flag
`skgpu::graphite::InsertRecordingInfo::fGpuStatsFlag` to `skgpu::GpuStatsFlags::kElapsedTime`. The new callback takes a
new struct, `skgpu::GpuStats`, which has an `elapsedTime` field that will indicate the amount of GPU time used by the
recording. This is implemented for the Dawn backend only. In WASM on WebGPU the reported time excludes any GPU transfers
that occur before the first render/compute pass or after the last pass because of limitations in the WebGPU timestamp
query API.

`GrDirectContext` provides a similar interface to report the GPU time spent in a flush. The client uses a new callback
type, `GrGpuFinishedWithStatsProc` and sets the same flag on `GrFlushInfo`. This is implemented for GL
(including GLES and WebGL).
