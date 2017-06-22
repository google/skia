/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "GrContextFactory.h"
#include "GrTest.h"
#include "Test.h"
#include "vk/GrVkGpu.h"

using sk_gpu_test::GrContextFactory;

void subheap_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    // memtype doesn't matter, we're just testing the suballocation algorithm so we'll use 0
    GrVkSubHeap heap(gpu, 0, 0, 64 * 1024, 32);
    GrVkAlloc alloc0, alloc1, alloc2, alloc3;
    // test full allocation and free
    REPORTER_ASSERT(reporter, heap.alloc(64 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, alloc0.fOffset == 0);
    REPORTER_ASSERT(reporter, alloc0.fSize == 64 * 1024);
    REPORTER_ASSERT(reporter, heap.freeSize() == 0 && heap.largestBlockSize() == 0);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 64*1024 && heap.largestBlockSize() == 64 * 1024);

    // now let's suballoc some memory
    REPORTER_ASSERT(reporter, heap.alloc(16 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(23 * 1024, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(18 * 1024, &alloc2));
    REPORTER_ASSERT(reporter, heap.freeSize() == 7 * 1024 && heap.largestBlockSize() == 7 * 1024);
    // free lone block
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.freeSize() == 30 * 1024 && heap.largestBlockSize() == 23 * 1024);
    // allocate into smallest free block
    REPORTER_ASSERT(reporter, heap.alloc(6 * 1024, &alloc3));
    REPORTER_ASSERT(reporter, heap.freeSize() == 24 * 1024 && heap.largestBlockSize() == 23 * 1024);
    // allocate into exact size free block
    REPORTER_ASSERT(reporter, heap.alloc(23 * 1024, &alloc1));
    REPORTER_ASSERT(reporter, heap.freeSize() == 1 * 1024 && heap.largestBlockSize() == 1 * 1024);
    // free lone block
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.freeSize() == 19 * 1024 && heap.largestBlockSize() == 18 * 1024);
    // free and merge with preceding block and following
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.freeSize() == 25 * 1024 && heap.largestBlockSize() == 25 * 1024);
    // free and merge with following block
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.freeSize() == 48 * 1024 && heap.largestBlockSize() == 48 * 1024);
    // free starting block and merge with following
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 64 * 1024 && heap.largestBlockSize() == 64 * 1024);

    // realloc
    REPORTER_ASSERT(reporter, heap.alloc(4 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(35 * 1024, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(10 * 1024, &alloc2));
    REPORTER_ASSERT(reporter, heap.freeSize() == 15 * 1024 && heap.largestBlockSize() == 15 * 1024);
    // free starting block and merge with following
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 19 * 1024 && heap.largestBlockSize() == 15 * 1024);
    // free block and merge with preceding
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.freeSize() == 54 * 1024 && heap.largestBlockSize() == 39 * 1024);
    // free block and merge with preceding and following
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.freeSize() == 64 * 1024 && heap.largestBlockSize() == 64 * 1024);

    // fragment
    REPORTER_ASSERT(reporter, heap.alloc(19 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(5 * 1024, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(15 * 1024, &alloc2));
    REPORTER_ASSERT(reporter, heap.alloc(3 * 1024, &alloc3));
    REPORTER_ASSERT(reporter, heap.freeSize() == 22 * 1024 && heap.largestBlockSize() == 22 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 41 * 1024 && heap.largestBlockSize() == 22 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.freeSize() == 56 * 1024 && heap.largestBlockSize() == 22 * 1024);
    REPORTER_ASSERT(reporter, !heap.alloc(40 * 1024, &alloc0));
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.freeSize() == 59 * 1024 && heap.largestBlockSize() == 40 * 1024);
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, heap.freeSize() == 19 * 1024 && heap.largestBlockSize() == 19 * 1024);
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.freeSize() == 24 * 1024 && heap.largestBlockSize() == 24 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 64 * 1024 && heap.largestBlockSize() == 64 * 1024);

    // unaligned sizes
    REPORTER_ASSERT(reporter, heap.alloc(19 * 1024 - 31, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(5 * 1024 - 5, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(15 * 1024 - 19, &alloc2));
    REPORTER_ASSERT(reporter, heap.alloc(3 * 1024 - 3, &alloc3));
    REPORTER_ASSERT(reporter, heap.freeSize() == 22 * 1024 && heap.largestBlockSize() == 22 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 41 * 1024 && heap.largestBlockSize() == 22 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.freeSize() == 56 * 1024 && heap.largestBlockSize() == 22 * 1024);
    REPORTER_ASSERT(reporter, !heap.alloc(40 * 1024, &alloc0));
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.freeSize() == 59 * 1024 && heap.largestBlockSize() == 40 * 1024);
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, &alloc0));
    REPORTER_ASSERT(reporter, heap.freeSize() == 19 * 1024 && heap.largestBlockSize() == 19 * 1024);
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.freeSize() == 24 * 1024 && heap.largestBlockSize() == 24 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.freeSize() == 64 * 1024 && heap.largestBlockSize() == 64 * 1024);
}

void suballoc_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    // memtype/heap index don't matter, we're just testing the allocation algorithm so we'll use 0
    GrVkHeap heap(gpu, GrVkHeap::kSubAlloc_Strategy, 64 * 1024);
    GrVkAlloc alloc0, alloc1, alloc2, alloc3;
    const VkDeviceSize kAlignment = 16;
    const uint32_t kMemType = 0;
    const uint32_t kHeapIndex = 0;

    REPORTER_ASSERT(reporter, heap.allocSize() == 0 && heap.usedSize() == 0);

    // fragment allocations so we need to grow heap
    REPORTER_ASSERT(reporter, heap.alloc(19 * 1024 - 3, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(5 * 1024 - 9, kAlignment, kMemType, kHeapIndex, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(15 * 1024 - 15, kAlignment, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.alloc(3 * 1024 - 6, kAlignment, kMemType, kHeapIndex, &alloc3));
    REPORTER_ASSERT(reporter, heap.allocSize() == 64 * 1024 && heap.usedSize() == 42 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.allocSize() == 64 * 1024 && heap.usedSize() == 23 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.allocSize() == 64 * 1024 && heap.usedSize() == 8 * 1024);
    // we expect the heap to grow here
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 48 * 1024);
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 45 * 1024);
    // heap should not grow here (first subheap has exactly enough room)
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, kAlignment, kMemType, kHeapIndex, &alloc3));
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 85 * 1024);
    // heap should not grow here (second subheap has room)
    REPORTER_ASSERT(reporter, heap.alloc(22 * 1024, kAlignment, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 107 * 1024);
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 102 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 62 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 40 * 1024);
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 0 * 1024);
    // heap should not grow here (allocating more than subheap size)
    REPORTER_ASSERT(reporter, heap.alloc(128 * 1024, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, 0 == alloc0.fSize);
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 0 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.allocSize() == 128 * 1024 && heap.usedSize() == 24 * 1024);
    // heap should alloc a new subheap because the memory type is different
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, kAlignment, kMemType+1, kHeapIndex, &alloc1));
    REPORTER_ASSERT(reporter, heap.allocSize() == 192 * 1024 && heap.usedSize() == 48 * 1024);
    // heap should alloc a new subheap because the alignment is different
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, 128, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.allocSize() == 256 * 1024 && heap.usedSize() == 72 * 1024);
    heap.free(alloc2);
    heap.free(alloc0);
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.allocSize() == 256 * 1024 && heap.usedSize() == 0 * 1024);
}

void singlealloc_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    // memtype/heap index don't matter, we're just testing the allocation algorithm so we'll use 0
    GrVkHeap heap(gpu, GrVkHeap::kSingleAlloc_Strategy, 64 * 1024);
    GrVkAlloc alloc0, alloc1, alloc2, alloc3;
    const VkDeviceSize kAlignment = 64;
    const uint32_t kMemType = 0;
    const uint32_t kHeapIndex = 0;

    REPORTER_ASSERT(reporter, heap.allocSize() == 0 && heap.usedSize() == 0);

    // make a few allocations
    REPORTER_ASSERT(reporter, heap.alloc(49 * 1024 - 3, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.alloc(5 * 1024 - 37, kAlignment, kMemType, kHeapIndex, &alloc1));
    REPORTER_ASSERT(reporter, heap.alloc(15 * 1024 - 11, kAlignment, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.alloc(3 * 1024 - 29, kAlignment, kMemType, kHeapIndex, &alloc3));
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 72 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 23 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 8 * 1024);
    // heap should not grow here (first subheap has room)
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 48 * 1024);
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 45 * 1024);
    // check for exact fit -- heap should not grow here (third subheap has room)
    REPORTER_ASSERT(reporter, heap.alloc(15 * 1024 - 63, kAlignment, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 60 * 1024);
    heap.free(alloc2);
    REPORTER_ASSERT(reporter, heap.allocSize() == 72 * 1024 && heap.usedSize() == 45 * 1024);
    // heap should grow here (no subheap has room)
    REPORTER_ASSERT(reporter, heap.alloc(40 * 1024, kAlignment, kMemType, kHeapIndex, &alloc3));
    REPORTER_ASSERT(reporter, heap.allocSize() == 112 * 1024 && heap.usedSize() == 85 * 1024);
    heap.free(alloc1);
    REPORTER_ASSERT(reporter, heap.allocSize() == 112 * 1024 && heap.usedSize() == 80 * 1024);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.allocSize() == 112 * 1024 && heap.usedSize() == 40 * 1024);
    heap.free(alloc3);
    REPORTER_ASSERT(reporter, heap.allocSize() == 112 * 1024 && heap.usedSize() == 0 * 1024);
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, kAlignment, kMemType, kHeapIndex, &alloc0));
    REPORTER_ASSERT(reporter, heap.allocSize() == 112 * 1024 && heap.usedSize() == 24 * 1024);
    // heap should alloc a new subheap because the memory type is different
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, kAlignment, kMemType + 1, kHeapIndex, &alloc1));
    REPORTER_ASSERT(reporter, heap.allocSize() == 136 * 1024 && heap.usedSize() == 48 * 1024);
    // heap should alloc a new subheap because the alignment is different
    REPORTER_ASSERT(reporter, heap.alloc(24 * 1024, 128, kMemType, kHeapIndex, &alloc2));
    REPORTER_ASSERT(reporter, heap.allocSize() == 160 * 1024 && heap.usedSize() == 72 * 1024);
    heap.free(alloc1);
    heap.free(alloc2);
    heap.free(alloc0);
    REPORTER_ASSERT(reporter, heap.allocSize() == 160 * 1024 && heap.usedSize() == 0 * 1024);
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkHeapTests, reporter, ctxInfo) {
    subheap_test(reporter, ctxInfo.grContext());
    suballoc_test(reporter, ctxInfo.grContext());
    singlealloc_test(reporter, ctxInfo.grContext());
}

#endif
