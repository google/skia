/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphiteTypes_DEFINED
#define skgpu_graphite_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"

#include <memory>

class SkSurface;

namespace skgpu::graphite {

class Recording;
class Task;

using GpuFinishedContext = void*;
using GpuFinishedProc = void (*)(GpuFinishedContext finishedContext, CallbackResult);

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occured it will be called with CallbackResult::kFailed.
 *
 * The fTargetSurface, if provided, is used as a target for any draws recorded onto a deferred
 * canvas returned from Recorder::makeDeferredCanvas. This target surface must be provided iff
 * the Recording contains any such draws. It must be Graphite-backed and its backing texture's
 * TextureInfo must match the info provided to the Recorder when making the deferred canvas.
 */
struct InsertRecordingInfo {
    Recording* fRecording = nullptr;
    SkSurface* fTargetSurface = nullptr;
    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
};

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occured it will be called with CallbackResult::kFailed.
 */
struct InsertFinishInfo {
    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
};

/**
 * Actually submit work to the GPU and track its completion
 */
enum class SyncToCpu : bool {
    kYes = true,
    kNo = false
};

/**
 * Is the texture mipmapped or not
 */
enum class Mipmapped : bool {
    kNo = false,
    kYes = true,
};

/*
 * For Promise Images - should the Promise Image be fulfilled every time a Recording that references
 * it is inserted into the Context.
 */
enum class Volatile : bool {
    kNo = false,              // only fulfilled once
    kYes = true               // fulfilled on every insertion call
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphiteTypes_DEFINED
