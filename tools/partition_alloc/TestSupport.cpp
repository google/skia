/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/partition_alloc/TestSupport.h"

#include <partition_alloc/buildflags.h>

#if PA_BUILDFLAG(USE_ALLOCATOR_SHIM)
#include <partition_alloc/shim/allocator_shim.h>
#include <partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h>
#endif

#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
#include <partition_alloc/dangling_raw_ptr_checks.h>
#include "include/core/SkTypes.h"
#endif

namespace skiatest {

void InitializePartitionAllocForTesting() {
#if PA_BUILDFLAG(USE_ALLOCATOR_SHIM)
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

void InitializeDanglingPointerChecksForTesting() {
#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
    // TODO(351867706): Similar to Chrome, record a stacktrace during free when
    // a pointer becomes dangling, and a stacktrace when the dangling pointer
    // is released. Join the two and present them to Skia developers so they
    // know both the location of free and the location of the dangling pointer.
    partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t ptr) {
        SK_ABORT("DanglingPointerDetector: A pointer was dangling!");
    });
#endif
}

}  // namespace skiatest
