/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_GPU_H
#define SK_COMMON_FLAGS_GPU_H

#include "CommandLineFlags.h"
#include "GrTypesPriv.h"
#include "SkTypes.h"

DECLARE_int32(gpuThreads);
DECLARE_bool(cachePathMasks);
DECLARE_bool(noGS);
DECLARE_string(pr);
DECLARE_bool(reduceOpListSplitting);

inline GpuPathRenderers get_named_pathrenderers_flags(const char* name) {
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

inline GpuPathRenderers CollectGpuPathRenderersFromFlags() {
    if (FLAGS_pr.isEmpty()) {
        return GpuPathRenderers::kDefault;
    }
    GpuPathRenderers gpuPathRenderers =
            ('~' == FLAGS_pr[0][0]) ? GpuPathRenderers::kDefault : GpuPathRenderers::kNone;
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

/**
 *  Helper to set GrContextOptions from common GPU flags.
 */
void SetCtxOptionsFromCommonFlags(struct GrContextOptions*);

#endif
