`GrDirectContext` now provides an interface to report the GPU time spent in a flush. The client uses a new callback
type, `GrGpuFinishedWithStatsProc` and sets a flag on `GrFlushInfo`. The new callback signature takes a struct that
provides the GPU time. This is implemented for GL.
