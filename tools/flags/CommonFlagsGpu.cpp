/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/gpu/GrContextOptions.h"
#include "tools/flags/CommonFlags.h"
#include "tools/gpu/MemoryCache.h"


DEFINE_int(gpuThreads,
             2,
             "Create this many extra threads to assist with GPU work, "
             "including software path rendering. Defaults to two.");

static DEFINE_bool(cachePathMasks, true,
                   "Allows path mask textures to be cached in GPU configs.");

static DEFINE_bool(noGS, false, "Disables support for geometry shaders.");

static DEFINE_bool(cc, false, "Allow coverage counting shortcuts to render paths?");

static DEFINE_string(pr, "",
              "Set of enabled gpu path renderers. Defined as a list of: "
              "[~]none [~]dashline [~]nvpr [~]ccpr [~]aahairline [~]aaconvex [~]aalinearizing "
              "[~]small [~]tess] [~]all");

static DEFINE_bool(disableDriverCorrectnessWorkarounds, false,
                   "Disables all GPU driver correctness workarounds");

static DEFINE_bool(reduceOpListSplitting, false, "Improve opList sorting");
static DEFINE_bool(dontReduceOpListSplitting, false, "Allow more opList splitting");

static DEFINE_bool(programBinaryCache, false, "Use in-memory program binary cache");

static GpuPathRenderers get_named_pathrenderers_flags(const char* name) {
    if (!strcmp(name, "none")) {
        return GpuPathRenderers::kNone;
    } else if (!strcmp(name, "dashline")) {
        return GpuPathRenderers::kDashLine;
    } else if (!strcmp(name, "nvpr")) {
        return GpuPathRenderers::kStencilAndCover;
    } else if (!strcmp(name, "ccpr")) {
        return GpuPathRenderers::kCoverageCounting;
    } else if (!strcmp(name, "aahairline")) {
        return GpuPathRenderers::kAAHairline;
    } else if (!strcmp(name, "aaconvex")) {
        return GpuPathRenderers::kAAConvex;
    } else if (!strcmp(name, "aalinearizing")) {
        return GpuPathRenderers::kAALinearizing;
    } else if (!strcmp(name, "small")) {
        return GpuPathRenderers::kSmall;
    } else if (!strcmp(name, "tess")) {
        return GpuPathRenderers::kTessellating;
    } else if (!strcmp(name, "all")) {
        return GpuPathRenderers::kAll;
    }
    SK_ABORT(SkStringPrintf("error: unknown named path renderer \"%s\"\n", name).c_str());
    return GpuPathRenderers::kNone;
}

static GpuPathRenderers collect_gpu_path_renderers_from_flags() {
    if (FLAGS_pr.isEmpty()) {
        return GpuPathRenderers::kAll;
    }

    GpuPathRenderers gpuPathRenderers = ('~' == FLAGS_pr[0][0])
            ? GpuPathRenderers::kAll
            : GpuPathRenderers::kNone;

    for (int i = 0; i < FLAGS_pr.count(); ++i) {
        const char* name = FLAGS_pr[i];
        if (name[0] == '~') {
            gpuPathRenderers &= ~get_named_pathrenderers_flags(&name[1]);
        } else {
            gpuPathRenderers |= get_named_pathrenderers_flags(name);
        }
    }
    return gpuPathRenderers;
}

void SetCtxOptionsFromCommonFlags(GrContextOptions* ctxOptions) {
    static std::unique_ptr<SkExecutor> gGpuExecutor = (0 != FLAGS_gpuThreads)
        ? SkExecutor::MakeFIFOThreadPool(FLAGS_gpuThreads)
        : nullptr;

    ctxOptions->fExecutor                            = gGpuExecutor.get();
    ctxOptions->fDisableCoverageCountingPaths        = !FLAGS_cc;
    ctxOptions->fAllowPathMaskCaching                = FLAGS_cachePathMasks;
    ctxOptions->fSuppressGeometryShaders             = FLAGS_noGS;
    ctxOptions->fGpuPathRenderers                    = collect_gpu_path_renderers_from_flags();
    ctxOptions->fDisableDriverCorrectnessWorkarounds = FLAGS_disableDriverCorrectnessWorkarounds;

    if (FLAGS_programBinaryCache) {
        static sk_gpu_test::MemoryCache gMemoryCache;
        ctxOptions->fPersistentCache = &gMemoryCache;
    }

    if (FLAGS_reduceOpListSplitting) {
        SkASSERT(!FLAGS_dontReduceOpListSplitting);
        ctxOptions->fReduceOpListSplitting = GrContextOptions::Enable::kYes;
    } else if (FLAGS_dontReduceOpListSplitting) {
        ctxOptions->fReduceOpListSplitting = GrContextOptions::Enable::kNo;
    }
}
