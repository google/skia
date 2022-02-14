/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This is an experimental (and probably temporary) solution that allows
 * to compare performance SkVM blitters vs RasterPipeline blitters.
 * In addition to measuring performance (which is questionable) it also produces
 * other counts (pixels, scanlines) and more detailed traces that
 * can explain the current results (SkVM is slower) and help improve it.
 * The entire code is hidden under build flag skia_compare_vm_vs_rp=true
 * and will not appear at all without it.
 *
 * In order to collect the tracing information SkVM Blitters should run with SKVM_BLITTER_TRACE_ON
 * and RasterPipeline Blitters with RASTER_PIPELINE_BLITTER_TRACE_ON.
 */

#ifndef SkBlitterTrace_DEFINED
#define SkBlitterTrace_DEFINED

#include <inttypes.h>
#include <unordered_map>
#include "src/utils/SkBlitterTraceCommon.h"

#ifdef SKIA_COMPARE_VM_VS_RP

#if !defined(SK_BLITTER_TRACE_IS_SKVM) && !defined(SK_BLITTER_TRACE_IS_RASTER_PIPELINE)
#error "One blitter trace type should be defined if we have flag skia_compare_vm_vs_rp flag = true."
#endif

#if defined(SK_BLITTER_TRACE_IS_SKVM) && defined(SK_BLITTER_TRACE_IS_RASTER_PIPELINE)
#error "Only one blitter trace type should be defined."
#endif

#ifdef SK_BLITTER_TRACE_IS_SKVM
SkBlitterTrace gSkVMBlitterTrace("VM", false);
#define SK_BLITTER_TRACE_STEP(name, trace, scanlines, pixels) \
    SkBlitterTrace::Step name(trace ? &gSkVMBlitterTrace : nullptr, \
                              #name,     \
                              scanlines, \
                              pixels)
#endif

#ifdef SK_BLITTER_TRACE_IS_RASTER_PIPELINE
SkBlitterTrace gSkRPBlitterTrace("RP", false);
#define SK_BLITTER_TRACE_STEP(name, trace, scanlines, pixels) \
    SkBlitterTrace::Step name(trace ? &gSkRPBlitterTrace : nullptr, \
                              #name,     \
                              scanlines, \
                              pixels)
#endif

#define SK_BLITTER_TRACE_STEP_ACCUMULATE(step, pixels) \
        step.add(/*scanlines=*/0, /*pixels=*/run)
#else
#define INITIATE_BLITTER_TRACE(type) SK_BLITTER_TRACE_NO_CODE
#define SK_BLITTER_TRACE_STEP(name, trace, scanlines, pixels) SK_BLITTER_TRACE_NO_CODE
#define SK_BLITTER_TRACE_STEP_ACCUMULATE(step, pixels) SK_BLITTER_TRACE_NO_CODE
#endif

#endif // SkBlitterTrace_DEFINED
