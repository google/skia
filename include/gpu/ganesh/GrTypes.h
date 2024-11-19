/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypes_DEFINED
#define GrTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTo.h" // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
class GrBackendSemaphore;

///////////////////////////////////////////////////////////////////////////////

/**
 * Possible 3D APIs that may be used by Ganesh.
 */
enum class GrBackendApi : unsigned {
    kOpenGL,
    kVulkan,
    kMetal,
    kDirect3D,

    /**
     * Mock is a backend that does not draw anything. It is used for unit tests
     * and to measure CPU overhead.
     */
    kMock,

    /**
     * Ganesh doesn't support some context types (e.g. Dawn) and will return Unsupported.
     */
    kUnsupported,

    /**
     * Added here to support the legacy GrBackend enum value and clients who referenced it using
     * GrBackend::kOpenGL_GrBackend.
     */
    kOpenGL_GrBackend = kOpenGL,
};

/**
 * Previously the above enum was not an enum class but a normal enum. To support the legacy use of
 * the enum values we define them below so that no clients break.
 */
typedef GrBackendApi GrBackend;

static constexpr GrBackendApi kMetal_GrBackend = GrBackendApi::kMetal;
static constexpr GrBackendApi kVulkan_GrBackend = GrBackendApi::kVulkan;
static constexpr GrBackendApi kMock_GrBackend = GrBackendApi::kMock;

///////////////////////////////////////////////////////////////////////////////

/*
 * Can a GrBackendObject be rendered to?
 */
using GrRenderable = skgpu::Renderable;

/*
 * Used to say whether texture is backed by protected memory.
 */
using GrProtected = skgpu::Protected;

///////////////////////////////////////////////////////////////////////////////

/**
 * GPU SkImage and SkSurfaces can be stored such that (0, 0) in texture space may correspond to
 * either the top-left or bottom-left content pixel.
 */
enum GrSurfaceOrigin : int {
    kTopLeft_GrSurfaceOrigin,
    kBottomLeft_GrSurfaceOrigin,
};

/**
 * A GrContext's cache of backend context state can be partially invalidated.
 * These enums are specific to the GL backend and we'd add a new set for an alternative backend.
 */
enum GrGLBackendState {
    kRenderTarget_GrGLBackendState     = 1 << 0,
    // Also includes samplers bound to texture units.
    kTextureBinding_GrGLBackendState   = 1 << 1,
    // View state stands for scissor and viewport
    kView_GrGLBackendState             = 1 << 2,
    kBlend_GrGLBackendState            = 1 << 3,
    kMSAAEnable_GrGLBackendState       = 1 << 4,
    kVertex_GrGLBackendState           = 1 << 5,
    kStencil_GrGLBackendState          = 1 << 6,
    kPixelStore_GrGLBackendState       = 1 << 7,
    kProgram_GrGLBackendState          = 1 << 8,
    kFixedFunction_GrGLBackendState    = 1 << 9,
    kMisc_GrGLBackendState             = 1 << 10,
    kALL_GrGLBackendState              = 0xffff
};

/**
 * This value translates to reseting all the context state for any backend.
 */
static const uint32_t kAll_GrBackendState = 0xffffffff;

typedef void* GrGpuFinishedContext;
typedef void (*GrGpuFinishedProc)(GrGpuFinishedContext finishedContext);
typedef void (*GrGpuFinishedWithStatsProc)(GrGpuFinishedContext finishedContext,
                                           const skgpu::GpuStats&);

typedef void* GrGpuSubmittedContext;
typedef void (*GrGpuSubmittedProc)(GrGpuSubmittedContext submittedContext, bool success);

typedef void* GrDirectContextDestroyedContext;
typedef void (*GrDirectContextDestroyedProc)(GrDirectContextDestroyedContext destroyedContext);

/**
 * Struct to supply options to flush calls.
 *
 * After issuing all commands, fNumSemaphore semaphores will be signaled by the gpu. The client
 * passes in an array of fNumSemaphores GrBackendSemaphores. In general these GrBackendSemaphore's
 * can be either initialized or not. If they are initialized, the backend uses the passed in
 * semaphore. If it is not initialized, a new semaphore is created and the GrBackendSemaphore
 * object is initialized with that semaphore. The semaphores are not sent to the GPU until the next
 * GrContext::submit call is made. See the GrContext::submit for more information.
 *
 * The client will own and be responsible for deleting the underlying semaphores that are stored
 * and returned in initialized GrBackendSemaphore objects. The GrBackendSemaphore objects
 * themselves can be deleted as soon as this function returns.
 *
 * If a finishedProc or finishedWithStatsProc is provided, the proc will be called when all work
 * submitted to the gpu from this flush call and all previous flush calls has finished on the GPU.
 * If the flush call fails due to an error and nothing ends up getting sent to the GPU, the finished
 * proc is called immediately. If both types of proc are provided then finishedWithStatsProc is
 * preferred.
 *
 * When finishedWithStatsProc is called the GpuStats passed will contain valid values for stats
 * by requested by gpuStatsFlags, assuming the stats are supported by the underlying backend GPU
 * context and the GPU work completed successfully.
 *
 * If a submittedProc is provided, the submittedProc will be called when all work from this flush
 * call is submitted to the GPU. If the flush call fails due to an error and nothing will get sent
 * to the GPU, the submitted proc is called immediately. It is possibly that when work is finally
 * submitted, that the submission actual fails. In this case we will not reattempt to do the
 * submission. Skia notifies the client of these via the success bool passed into the submittedProc.
 * The submittedProc is useful to the client to know when semaphores that were sent with the flush
 * have actually been submitted to the GPU so that they can be waited on (or deleted if the submit
 * fails).
 * GrBackendSemaphores are not supported for the GL backend and will be ignored if set.
 */
struct GrFlushInfo {
    size_t fNumSemaphores = 0;
    skgpu::GpuStatsFlags fGpuStatsFlags = skgpu::GpuStatsFlags::kNone;
    GrBackendSemaphore* fSignalSemaphores = nullptr;
    GrGpuFinishedProc fFinishedProc = nullptr;
    GrGpuFinishedWithStatsProc fFinishedWithStatsProc = nullptr;
    GrGpuFinishedContext fFinishedContext = nullptr;
    GrGpuSubmittedProc fSubmittedProc = nullptr;
    GrGpuSubmittedContext fSubmittedContext = nullptr;
};

/**
 * Enum used as return value when flush with semaphores so the client knows whether the valid
 * semaphores will be submitted on the next GrContext::submit call.
 */
enum class GrSemaphoresSubmitted : bool {
    kNo = false,
    kYes = true
};

enum class GrPurgeResourceOptions {
    kAllResources,
    kScratchResourcesOnly,
};

enum class GrSyncCpu : bool {
    kNo = false,
    kYes = true,
};

enum class GrMarkFrameBoundary : bool {
    kNo = false,
    kYes = true,
};

struct GrSubmitInfo {
    GrSyncCpu fSync = GrSyncCpu::kNo;
    GrMarkFrameBoundary fMarkBoundary = GrMarkFrameBoundary::kNo;
    uint64_t fFrameID = 0;
};

#endif
