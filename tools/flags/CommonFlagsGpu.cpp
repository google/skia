/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/gpu/GrContextOptions.h"
#include "tools/flags/CommonFlags.h"

DEFINE_int(gpuThreads,
             2,
             "Create this many extra threads to assist with GPU work, "
             "including software path rendering. Defaults to two.");

static DEFINE_bool(cachePathMasks, true,
                   "Allows path mask textures to be cached in GPU configs.");
static DEFINE_bool(allPathsVolatile, false,
                   "Causes all GPU paths to be processed as if 'setIsVolatile' had been called.");

static DEFINE_bool(hwtess, false, "Enables support for tessellation shaders (if hw allows.).");

static DEFINE_int(maxTessellationSegments, 0,
                  "Overrides the max number of tessellation segments supported by the caps.");

static DEFINE_bool(alwaysHwTess, false,
        "Always try to use hardware tessellation, regardless of how small a path may be.");

static DEFINE_string(pr, "",
              "Set of enabled gpu path renderers. Defined as a list of: "
              "[~]none [~]dashline [~]aahairline [~]aaconvex [~]aalinearizing [~]small [~]tri "
              "[~]atlas [~]tess [~]all");

static DEFINE_int(internalSamples, -1,
        "Number of samples for internal draws that use MSAA, or default value if negative.");

static DEFINE_int(maxAtlasSize, -1,
        "Maximum width and height of internal texture atlases, or default value if negative.");

static DEFINE_bool(disableDriverCorrectnessWorkarounds, false,
                   "Disables all GPU driver correctness workarounds");

static DEFINE_bool(dontReduceOpsTaskSplitting, false,
                   "Don't reorder tasks to reduce render passes");

static DEFINE_bool(skgpuv2, false, "use the new GPU backend");

static DEFINE_int(gpuResourceCacheLimit, -1,
                  "Maximum number of bytes to use for budgeted GPU resources. "
                  "Default is -1, which means GrResourceCache::kDefaultMaxSize.");

static GpuPathRenderers get_named_pathrenderers_flags(const char* name) {
    if (!strcmp(name, "none")) {
        return GpuPathRenderers::kNone;
    } else if (!strcmp(name, "dashline")) {
        return GpuPathRenderers::kDashLine;
    } else if (!strcmp(name, "aahairline")) {
        return GpuPathRenderers::kAAHairline;
    } else if (!strcmp(name, "aaconvex")) {
        return GpuPathRenderers::kAAConvex;
    } else if (!strcmp(name, "aalinearizing")) {
        return GpuPathRenderers::kAALinearizing;
    } else if (!strcmp(name, "small")) {
        return GpuPathRenderers::kSmall;
    } else if (!strcmp(name, "tri")) {
        return GpuPathRenderers::kTriangulating;
    } else if (!strcmp(name, "atlas")) {
        return GpuPathRenderers::kAtlas;
    } else if (!strcmp(name, "tess")) {
        return GpuPathRenderers::kTessellation;
    } else if (!strcmp(name, "default")) {
        return GpuPathRenderers::kDefault;
    }
    SK_ABORT("error: unknown named path renderer \"%s\"\n", name);
}

static GpuPathRenderers collect_gpu_path_renderers_from_flags() {
    if (FLAGS_pr.isEmpty()) {
        return GpuPathRenderers::kDefault;
    }

    GpuPathRenderers gpuPathRenderers = ('~' == FLAGS_pr[0][0])
            ? GpuPathRenderers::kDefault
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
    ctxOptions->fAllowPathMaskCaching                = FLAGS_cachePathMasks;
    ctxOptions->fAllPathsVolatile                    = FLAGS_allPathsVolatile;
    ctxOptions->fEnableExperimentalHardwareTessellation = FLAGS_hwtess;
    ctxOptions->fMaxTessellationSegmentsOverride     = FLAGS_maxTessellationSegments;
    ctxOptions->fAlwaysPreferHardwareTessellation    = FLAGS_alwaysHwTess;
    ctxOptions->fGpuPathRenderers                    = collect_gpu_path_renderers_from_flags();
    ctxOptions->fDisableDriverCorrectnessWorkarounds = FLAGS_disableDriverCorrectnessWorkarounds;
    ctxOptions->fResourceCacheLimitOverride          = FLAGS_gpuResourceCacheLimit;

    if (FLAGS_internalSamples >= 0) {
        ctxOptions->fInternalMultisampleCount = FLAGS_internalSamples;
    }
    if (FLAGS_maxAtlasSize >= 0) {
        ctxOptions->fMaxTextureAtlasSize = FLAGS_maxAtlasSize;
    }

    if (FLAGS_dontReduceOpsTaskSplitting) {
        ctxOptions->fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
    } else {
        ctxOptions->fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
    }

    if (FLAGS_skgpuv2) {
        ctxOptions->fUseSkGpuV2 = GrContextOptions::Enable::kYes;
    }
}
