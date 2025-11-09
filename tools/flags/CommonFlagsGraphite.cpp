/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsGraphite.h"

#include "include/core/SkExecutor.h"
#include "tools/graphite/TestOptions.h"

// Defined in CommonFlagsConfig
DECLARE_int(gpuThreads)
DECLARE_int(internalSamples)

namespace CommonFlags {
#if defined(SK_DAWN)
static DEFINE_bool(disable_tint_symbol_renaming, false, "Disable Tint WGSL symbol renaming when "
                                                        "using Dawn");
static DEFINE_bool(neverYieldToWebGPU, false, "Run Graphite with never-yield context option.");
static DEFINE_bool(useWGPUTextureView, false, "Run Graphite w/ a wrapped WGPU texture view as "
                                              "the destination");
#endif // SK_DAWN

DEFINE_int(internalMSAATileSize, 0, "Run Graphite w/ limited MSAA texture's size");
DEFINE_int(minMSAAPathSize, -1,
           "Graphite uses raster atlas for paths smaller than this, or default value if negative");

void SetTestOptions(skiatest::graphite::TestOptions* testOptions) {
    static std::unique_ptr<SkExecutor> gGpuExecutor =
            (0 != FLAGS_gpuThreads) ? SkExecutor::MakeFIFOThreadPool(FLAGS_gpuThreads)
                                    : nullptr;

    testOptions->fContextOptions.fExecutor = gGpuExecutor.get();

    if (FLAGS_internalSamples >= 0) {
        testOptions->fContextOptions.fInternalMultisampleCount = FLAGS_internalSamples;
    }
    if (FLAGS_internalMSAATileSize > 0) {
        testOptions->fContextOptions.fInternalMSAATileSize = {FLAGS_internalMSAATileSize,
                                                              FLAGS_internalMSAATileSize};
    }
    if (FLAGS_minMSAAPathSize >= 0) {
        testOptions->fContextOptions.fMinimumPathSizeForMSAA = FLAGS_minMSAAPathSize;
    }

#if defined(SK_DAWN)
    testOptions->fDisableTintSymbolRenaming = FLAGS_disable_tint_symbol_renaming;
    testOptions->fNeverYieldToWebGPU = FLAGS_neverYieldToWebGPU;
    testOptions->fUseWGPUTextureView = FLAGS_useWGPUTextureView;
#endif // SK_DAWN
}

}  // namespace CommonFlags
