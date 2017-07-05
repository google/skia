/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_PATH_RENDERER_H
#define SK_COMMON_FLAGS_PATH_RENDERER_H

#if SK_SUPPORT_GPU

#include "GrContextOptions.h"
#include "SkCommandLineFlags.h"
#include "SkTypes.h"

DECLARE_string(pr);

#define DEFINE_pathrenderer_flag                                                   \
    DEFINE_string(pr, "all",                                                       \
                  "Set of enabled gpu path renderers. Defined as a list of: "      \
                  "[[~]all [~]dashline [~]nvpr [~]msaa [~]aahairline [~]aaconvex " \
                  "[~]aalinearizing [~]small [~]tess [~]grdefault]")

inline GrContextOptions::GpuPathRenderers get_named_pathrenderers_flags(const char* name) {
    using GpuPathRenderers = GrContextOptions::GpuPathRenderers;
    if (!strcmp(name, "all")) {
        return GpuPathRenderers::kAll;
    } else if (!strcmp(name, "dashline")) {
        return GpuPathRenderers::kDashLine;
    } else if (!strcmp(name, "nvpr")) {
        return GpuPathRenderers::kStencilAndCover;
    } else if (!strcmp(name, "msaa")) {
        return GpuPathRenderers::kMSAA;
    } else if (!strcmp(name, "aahairline")) {
        return GpuPathRenderers::kAAHairline;
    } else if (!strcmp(name, "aaconvex")) {
        return GpuPathRenderers::kAAConvex;
    } else if (!strcmp(name, "aalinearizing")) {
        return GpuPathRenderers::kAALinearizing;
    } else if (!strcmp(name, "small")) {
        return GpuPathRenderers::kSmall;
    } else if (!strcmp(name, "ccpr")) {
        return GpuPathRenderers::kCoverageCounting;
    } else if (!strcmp(name, "tess")) {
        return GpuPathRenderers::kTessellating;
    } else if (!strcmp(name, "grdefault")) {
        return GpuPathRenderers::kDefault;
    }
    SK_ABORT(SkStringPrintf("error: unknown named path renderer \"%s\"\n", name).c_str());
    return GpuPathRenderers::kNone;
}

inline GrContextOptions::GpuPathRenderers CollectGpuPathRenderersFromFlags() {
    using GpuPathRenderers = GrContextOptions::GpuPathRenderers;
    if (FLAGS_pr.isEmpty()) {
        return GpuPathRenderers::kAll;
    }
    GpuPathRenderers gpuPathRenderers = '~' == FLAGS_pr[0][0] ?
                                        GpuPathRenderers::kAll : GpuPathRenderers::kNone;
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

#endif // SK_SUPPORT_GPU

#endif
