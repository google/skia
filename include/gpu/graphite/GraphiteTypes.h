/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphiteTypes_DEFINED
#define skgpu_graphite_GraphiteTypes_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"

#include <memory>

class SkSurface;

namespace skgpu {
class MutableTextureState;
}

namespace skgpu::graphite {

class BackendSemaphore;
class Recording;
class Task;

using GpuFinishedContext = void*;
using GpuFinishedProc = void (*)(GpuFinishedContext finishedContext, CallbackResult);

using GpuFinishedWithStatsProc = void (*)(GpuFinishedContext finishedContext,
                                          CallbackResult,
                                          const GpuStats&);

// NOTE: This can be converted to just an `enum class InsertStatus {}` once clients are migrated
// off of assuming `Context::insertRecording()` returns a boolean.
class InsertStatus {
public:
    // Do not refer to V directly; use these constants as if InsertStatus were a class enum, e.g.
    // InsertStatus::kSuccess.
    enum V {
        // Everything successfully added to underlying CommandBuffer
        kSuccess,
        // Recording or InsertRecordingInfo invalid, no CB changes
        kInvalidRecording,
        // Promise image instantiation failed, no CB changes
        kPromiseImageInstantiationFailed,
        // Internal failure, CB partially modified, state unrecoverable or unknown (e.g. dependent
        // texture uploads for future Recordings may or may not get executed)
        kAddCommandsFailed,
        // Internal failure, shader pipeline compilation failed (driver issue, or disk corruption),
        // state unrecoverable.
        kAsyncShaderCompilesFailed
    };

    constexpr InsertStatus() : fValue(kSuccess) {}
    /*implicit*/ constexpr InsertStatus(V v) : fValue(v) {}

    operator InsertStatus::V() const {
        return fValue;
    }

    // Assist migration from old bool return value of insertRecording; kSuccess is true,
    // all other error statuses are false.
    // NOTE: This is intentionally not explicit so that InsertStatus can be assigned correctly to
    // a bool or returned as a bool, since these are not boolean contexts that automatically apply
    // explicit bool operators (e.g. inside an if condition).
    operator bool() const {
        return fValue == kSuccess;
    }

private:
    V fValue;
};

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occurred it will be called with CallbackResult::kFailed.
 *
 * Alternatively, the client can provide fFinishedProcWithStats. This provides additional
 * information about execution of the recording on the GPU. Only the stats requested using
 * fStatsFlags will be valid and only if CallbackResult is kSuccess. If both fFinishedProc
 * and fFinishedProcWithStats are provided the latter is preferred and the former won't be
 * called.
 *
 * The fTargetSurface, if provided, is used as a target for any draws recorded onto a deferred
 * canvas returned from Recorder::makeDeferredCanvas. This target surface must be provided iff
 * the Recording contains any such draws. It must be Graphite-backed and its backing texture's
 * TextureInfo must match the info provided to the Recorder when making the deferred canvas.
 *
 * fTargetTranslation is an additional translation applied to draws targeting fTargetSurface.
 *
 * fTargetClip is an additional clip applied to draws targeting fTargetSurface. It is defined in the
 * local replay space, that is, with fTargetTranslation applied. An empty clip will not be applied.
 *
 * The client may pass in two arrays of initialized BackendSemaphores to be included in the
 * command stream. At some time before issuing commands in the Recording, the fWaitSemaphores will
 * be waited on by the gpu. We only guarantee these wait semaphores block transfer and fragment
 * shader work. Similarly, at some time after issuing the Recording's commands, the
 * fSignalSemaphores will be signaled by the gpu. Depending on the platform, the timing of the wait
 * and signal operations will either be immediately before or after the given Recording's command
 * stream, respectively, or before and after the entire CommandBuffer's command stream. The
 * semaphores are not sent to the GPU until the next Context::submit call is made.
 *
 * The client will own and be responsible for deleting the underlying semaphore objects after the
 * submission completes, however the BackendSemaphore objects themselves can be deleted as soon
 * as this function returns.
 */
struct InsertRecordingInfo {
    Recording* fRecording = nullptr;

    SkSurface* fTargetSurface = nullptr;
    SkIVector fTargetTranslation = {0, 0};
    SkIRect fTargetClip = {0, 0, 0, 0};
    MutableTextureState* fTargetTextureState = nullptr;

    size_t fNumWaitSemaphores = 0;
    BackendSemaphore* fWaitSemaphores = nullptr;
    size_t fNumSignalSemaphores = 0;
    BackendSemaphore* fSignalSemaphores = nullptr;

    GpuStatsFlags fGpuStatsFlags = GpuStatsFlags::kNone;
    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
    GpuFinishedWithStatsProc fFinishedWithStatsProc = nullptr;

    // For unit testing purposes, this can be used to induce a known failure status from
    // Context::insertRecording(). When this set to anything other than kSuccess, insertRecording()
    // will operate as normal until the first condition that would normally return the simulated
    // status is encountered. At that point, operations are treated as if that condition had failed.
    // This leaves the Context in a state consistent with encountering the InsertStatus in a normal
    // application.
    //
    // NOTE: If the simulated failure status is one of the later error codes but the inserted
    // Recording would fail with an earlier error code normally, that error is still returned.
    InsertStatus fSimulatedStatus = InsertStatus::kSuccess;
};

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occurred it will be called with CallbackResult::kFailed.
 */
struct InsertFinishInfo {
    InsertFinishInfo() = default;
    InsertFinishInfo(GpuFinishedContext context, GpuFinishedProc proc)
            : fFinishedContext{context}, fFinishedProc{proc} {}
    InsertFinishInfo(GpuFinishedContext context, GpuFinishedWithStatsProc proc)
            : fFinishedContext{context}, fFinishedWithStatsProc{proc} {}
    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
    GpuFinishedWithStatsProc fFinishedWithStatsProc = nullptr;
    GpuStatsFlags fGpuStatsFlags = GpuStatsFlags::kNone;
};

/**
 * Actually submit work to the GPU and track its completion
 */
enum class SyncToCpu : bool {
    kYes = true,
    kNo = false
};

/*
 * For Promise Images - should the Promise Image be fulfilled every time a Recording that references
 * it is inserted into the Context.
 */
enum class Volatile : bool {
    kNo = false,              // only fulfilled once
    kYes = true               // fulfilled on every insertion call
};

enum class DepthStencilFlags : int {
    kNone         = 0b000,
    kDepth        = 0b001,
    kStencil      = 0b010,
    kDepthStencil = kDepth | kStencil,
};

/*
 * This enum allows mapping from a set of observed RenderSteps (e.g., from a GraphicsPipeline
 * printout) to the correct 'drawTypes' parameter needed by the Precompilation API.
 */
enum DrawTypeFlags : uint16_t {

    kNone             = 0,

    // kBitmapText_Mask should be used for the BitmapTextRenderStep[mask] RenderStep
    kBitmapText_Mask  = 1 << 0,
    // kBitmapText_LCD should be used for the BitmapTextRenderStep[LCD] RenderStep
    kBitmapText_LCD   = 1 << 1,
    // kBitmapText_Color should be used for the BitmapTextRenderStep[color] RenderStep
    kBitmapText_Color = 1 << 2,
    // kSDFText should be used for the SDFTextRenderStep RenderStep
    kSDFText          = 1 << 3,
    // kSDFText_LCD should be used for the SDFTextLCDRenderStep RenderStep
    kSDFText_LCD      = 1 << 4,

    // kDrawVertices should be used to generate Pipelines that use the following RenderSteps:
    //    VerticesRenderStep[*] for:
    //        [Tris], [TrisTexCoords], [TrisColor], [TrisColorTexCoords],
    //        [Tristrips], [TristripsTexCoords], [TristripsColor], [TristripsColorTexCoords]
    kDrawVertices     = 1 << 5,

    // kCircularArc renders filled circular arcs, with or without the center included, and
    // stroked circular arcs with butt or round caps that don't include the center point.
    // It corresponds to the CircularArcRenderStep.
    kCircularArc      = 1 << 6,

    // kSimpleShape should be used to generate Pipelines that use the following RenderSteps:
    //    AnalyticRRectRenderStep
    //    PerEdgeAAQuadRenderStep
    //    CoverBoundsRenderStep[NonAAFill]
    kAnalyticRRect    = 1 << 7,
    kPerEdgeAAQuad    = 1 << 8,
    kNonAAFillRect    = 1 << 9,

    kSimpleShape      = kAnalyticRRect | kPerEdgeAAQuad | kNonAAFillRect,

    // kNonSimpleShape should be used to generate Pipelines that use the following RenderSteps:
    //    CoverageMaskRenderStep
    //    CoverBoundsRenderStep[*] for [InverseCover], [RegularCover]
    //    TessellateStrokeRenderStep
    //    TessellateWedgesRenderStep[*] for [Convex], [EvenOdd], [Winding]
    //    TessellateCurvesRenderStep[*] for [EvenOdd], [Winding]
    //    MiddleOutFanRenderStep[*] for [EvenOdd], [Winding]
    kNonSimpleShape   = 1 << 10,

    // This draw type covers all the methods Skia uses to draw drop shadows. It can be used to
    // generate Pipelines which, as part of their labels, have:
    //     the AnalyticBlurRenderStep
    //     VerticesRenderStep[TrisColor] with a GaussianColorFilter
    // For this draw type the PaintOptions parameter to Precompile() will be ignored.
    kDropShadows      = 1 << 11,

    // kAnalyticClip should be combined with the primary drawType for Pipelines that contain
    // either of the following sub-strings:
    //    AnalyticClip
    //    AnalyticAndAtlasClip
    kAnalyticClip     = 1 << 12,

    kLast = kAnalyticClip,
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphiteTypes_DEFINED
