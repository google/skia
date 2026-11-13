/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSafeMath.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrBufferAllocPool.h"
#include "src/gpu/ganesh/GrCpuBuffer.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrVertexChunkArray.h"
#include "src/gpu/ganesh/mock/GrMockOpTarget.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

namespace {

class TestBufferAllocPool final : public GrBufferAllocPool {
public:
    explicit TestBufferAllocPool(GrGpu* gpu, sk_sp<CpuBufferCache> cache = nullptr)
            : GrBufferAllocPool(gpu, GrGpuBufferType::kVertex, std::move(cache)) {}

    using GrBufferAllocPool::makeSpace;
    using GrBufferAllocPool::makeSpaceAtLeast;
};

// GrMockOpTarget calls SK_ABORT if a request exceeds its 6 MiB static buffer. This subclass
// instead returns nullptr for requests above 64 KiB so GrVertexChunkBuilder::allocChunk's retry
// fallback can be unit-tested deterministically.
class CapLimitedMockTarget final : public GrMockOpTarget {
public:
    explicit CapLimitedMockTarget(sk_sp<GrDirectContext> mockContext)
            : GrMockOpTarget(std::move(mockContext)) {}

    void* makeVertexSpaceAtLeast(size_t vertexSize,
                                 int minVertexCount,
                                 int fallbackVertexCount,
                                 sk_sp<const GrBuffer>* buffer,
                                 int* startVertex,
                                 int* actualVertexCount) override {
        if (fFirstMinVertexCount == 0) {
            fFirstMinVertexCount = minVertexCount;
        }
        constexpr size_t kMaxBytes = 2 * GrBufferAllocPool::kDefaultBufferSize;
        SkSafeMath safe;
        size_t minBytes = safe.mul(vertexSize, static_cast<size_t>(minVertexCount));
        if (!safe.ok() || minBytes > kMaxBytes) {
            buffer->reset();
            *startVertex = 0;
            *actualVertexCount = 0;
            return nullptr;
        }
        int clampedFallback = static_cast<int>(kMaxBytes / vertexSize);
        return GrMockOpTarget::makeVertexSpaceAtLeast(vertexSize,
                                                      minVertexCount,
                                                      clampedFallback,
                                                      buffer,
                                                      startVertex,
                                                      actualVertexCount);
    }

    int firstMinVertexCount() const { return fFirstMinVertexCount; }

private:
    int fFirstMinVertexCount = 0;
};

}  // namespace

// Verifies that makeSpaceAtLeast rejects minSize + pad size_t wraparound on both partially used
// and completely full active blocks (where fBytesFree == 0 < pad) without corrupting existing data
// or writing out-of-bounds padding.
DEF_GANESH_TEST(GrBufferAllocPool_PaddingAdditionOverflow,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, options);
    if (!context) {
        ERRORF(reporter, "Could not create mock GrDirectContext.");
        return;
    }
    GrGpu* gpu = context->priv().getGpu();
    auto cache = GrBufferAllocPool::CpuBufferCache::Make(/*maxBuffersToCache=*/2);

    constexpr size_t kAlignment = 48;
    // Both 2^32 and 2^64 are congruent to 16 (mod 48), so (SIZE_MAX - 15) is an exact multiple of
    // 48 and adding 16 bytes of padding wraps size_t to 0.
    constexpr size_t kWrappingMinSize = std::numeric_limits<size_t>::max() - 15;
    static_assert(kWrappingMinSize % kAlignment == 0);

    // 1. Partially used active block (usedBytes == 32, pad == 16).
    {
        TestBufferAllocPool pool(gpu, cache);

        sk_sp<const GrBuffer> firstBuffer;
        size_t firstOffset = 999;
        auto* firstPtr = static_cast<uint8_t*>(
                pool.makeSpace(/*size=*/32, /*alignment=*/16, &firstBuffer, &firstOffset));
        REPORTER_ASSERT(reporter, firstPtr != nullptr);
        REPORTER_ASSERT(reporter, firstBuffer != nullptr);
        REPORTER_ASSERT(reporter, firstOffset == 0u);
        std::memset(firstPtr, 0x5A, 32);

        sk_sp<const GrBuffer> secondBuffer = firstBuffer;
        size_t secondOffset = 123;
        size_t actualSize = 456;
        void* secondPtr = pool.makeSpaceAtLeast(kWrappingMinSize,
                                                kWrappingMinSize,
                                                kAlignment,
                                                &secondBuffer,
                                                &secondOffset,
                                                &actualSize);
        REPORTER_ASSERT(reporter, secondPtr == nullptr);
        REPORTER_ASSERT(reporter, secondBuffer == nullptr);
        REPORTER_ASSERT(reporter, secondOffset == 0u);
        REPORTER_ASSERT(reporter, actualSize == 0u);

        // Subsequent valid allocations reuse the active block cleanly.
        sk_sp<const GrBuffer> thirdBuffer;
        size_t thirdOffset = 0;
        size_t thirdActualSize = 0;
        void* thirdPtr = pool.makeSpaceAtLeast(/*minSize=*/96,
                                               /*fallbackSize=*/192,
                                               kAlignment,
                                               &thirdBuffer,
                                               &thirdOffset,
                                               &thirdActualSize);
        REPORTER_ASSERT(reporter, thirdPtr != nullptr);
        REPORTER_ASSERT(reporter, thirdBuffer.get() == firstBuffer.get());
        REPORTER_ASSERT(reporter, thirdOffset == 48u);
        REPORTER_ASSERT(reporter, thirdActualSize >= 96u);
        REPORTER_ASSERT(reporter, thirdActualSize % kAlignment == 0u);
        for (size_t i = 0; i < 32; ++i) {
            REPORTER_ASSERT(reporter, firstPtr[i] == 0x5Au);
        }
        REPORTER_ASSERT(reporter, pool.unmap());
    }

    // 2. Completely full active default block (kDefaultBufferSize == 32,768 bytes ->
    //    fBytesFree == 0 and 32,768 % 48 == 32 -> pad == 16 > fBytesFree).
    {
        constexpr size_t kBlockSize = GrBufferAllocPool::kDefaultBufferSize;
        static_assert(kBlockSize % kAlignment == 32);

        TestBufferAllocPool pool(gpu, cache);
        sk_sp<const GrBuffer> fullBuffer;
        size_t fullOffset = 999;
        auto* fullPtr = static_cast<uint8_t*>(
                pool.makeSpace(kBlockSize, /*alignment=*/1, &fullBuffer, &fullOffset));
        REPORTER_ASSERT(reporter, fullPtr != nullptr);
        REPORTER_ASSERT(reporter, fullBuffer != nullptr);
        REPORTER_ASSERT(reporter, fullBuffer->size() == kBlockSize);
        REPORTER_ASSERT(reporter, fullOffset == 0u);
        std::memset(fullPtr, 0xA5, kBlockSize);

        sk_sp<const GrBuffer> overflowBuffer = fullBuffer;
        size_t overflowOffset = 123;
        size_t overflowActual = 456;
        void* overflowPtr = pool.makeSpaceAtLeast(kWrappingMinSize,
                                                  kWrappingMinSize,
                                                  kAlignment,
                                                  &overflowBuffer,
                                                  &overflowOffset,
                                                  &overflowActual);
        REPORTER_ASSERT(reporter, overflowPtr == nullptr);
        REPORTER_ASSERT(reporter, overflowBuffer == nullptr);
        REPORTER_ASSERT(reporter, overflowOffset == 0u);
        REPORTER_ASSERT(reporter, overflowActual == 0u);

        for (size_t i = 0; i < kBlockSize; ++i) {
            if (fullPtr[i] != 0xA5u) {
                ERRORF(reporter, "Corrupted byte at %zu: 0x%02x", i, fullPtr[i]);
                break;
            }
        }
        REPORTER_ASSERT(reporter, pool.unmap());
    }
}

// Verifies GrVertexBufferAllocPool::makeSpaceAtLeast when vertexSize * minVertexCount fits in
// size_t without multiplying overflow, while adding alignment padding wraps modulo
// 2^(8 * sizeof(size_t)) to 0.
DEF_GANESH_TEST(GrBufferAllocPool_VertexPoolPaddingWrap,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, options);
    if (!context) {
        ERRORF(reporter, "Could not create mock GrDirectContext.");
        return;
    }
    GrGpu* gpu = context->priv().getGpu();
    auto cache = GrBufferAllocPool::CpuBufferCache::Make(/*maxBuffersToCache=*/2);

    GrVertexBufferAllocPool pool(gpu, cache);

    // Exhaust a 32,768-byte default block (fBytesFree == 0) in both 64-bit and 32-bit builds.
    sk_sp<const GrBuffer> initialBuffer;
    int initialStart = -1;
    void* initialPtr = pool.makeSpace(/*vertexSize=*/1,
                                      static_cast<int>(GrBufferAllocPool::kDefaultBufferSize),
                                      &initialBuffer,
                                      &initialStart);
    REPORTER_ASSERT(reporter, initialPtr != nullptr);
    REPORTER_ASSERT(reporter, initialBuffer != nullptr);
    REPORTER_ASSERT(reporter, initialBuffer->size() == GrBufferAllocPool::kDefaultBufferSize);
    REPORTER_ASSERT(reporter, initialStart == 0);

    if constexpr (sizeof(size_t) == 8) {
        // 2^64 + 32768 = 2^15 * (2^49 + 1), and 129 divides (2^49 + 1), so
        // vertexSize * minVertexCount fits in 64-bit size_t while adding
        // pad = vertexSize - 32768 wraps modulo 2^64 to 0.
        constexpr int kMinVertices = (8192 * 129) - 1;
        constexpr uint64_t kVertexSize64 =
                uint64_t{4} * (((uint64_t{1} << 49) + 1) / uint64_t{129});

        sk_sp<const GrBuffer> buffer = initialBuffer;
        int startVertex = 77;
        int actualVertexCount = 88;
        void* ptr = pool.makeSpaceAtLeast(static_cast<size_t>(kVertexSize64),
                                          kMinVertices,
                                          kMinVertices,
                                          &buffer,
                                          &startVertex,
                                          &actualVertexCount);
        REPORTER_ASSERT(reporter, ptr == nullptr);
        REPORTER_ASSERT(reporter, buffer == nullptr);
        REPORTER_ASSERT(reporter, startVertex == 0);
        REPORTER_ASSERT(reporter, actualVertexCount == 0);
    } else {
        // 32,768 % 48 == 32 -> pad == 16; 48 * 89,478,485 == 0xFFFFFFF0 fits in 32-bit size_t and
        // wraps to 0 when adding 16.
        constexpr int kMinVertices = 89478485;
        constexpr size_t kVertexSize = 48;

        sk_sp<const GrBuffer> buffer = initialBuffer;
        int startVertex = 77;
        int actualVertexCount = 88;
        void* ptr = pool.makeSpaceAtLeast(kVertexSize,
                                          kMinVertices,
                                          kMinVertices,
                                          &buffer,
                                          &startVertex,
                                          &actualVertexCount);
        REPORTER_ASSERT(reporter, ptr == nullptr);
        REPORTER_ASSERT(reporter, buffer == nullptr);
        REPORTER_ASSERT(reporter, startVertex == 0);
        REPORTER_ASSERT(reporter, actualVertexCount == 0);
    }

    // Because the initial block was completely full, a subsequent normal allocation creates a new
    // block starting at vertex 0.
    sk_sp<const GrBuffer> recoveryBuffer;
    int recoveryStart = -1;
    void* recoveryPtr = pool.makeSpace(
            /*vertexSize=*/16, /*vertexCount=*/8, &recoveryBuffer, &recoveryStart);
    REPORTER_ASSERT(reporter, recoveryPtr != nullptr);
    REPORTER_ASSERT(reporter, recoveryBuffer != nullptr);
    REPORTER_ASSERT(reporter, recoveryBuffer.get() != initialBuffer.get());
    REPORTER_ASSERT(reporter, recoveryStart == 0);
    REPORTER_ASSERT(reporter, pool.unmap());
}

// Verifies GrCpuBuffer::Make and CpuBufferCache::makeBuffer fallibility and the kMaxBlockSize
// single-block allocation cap.
DEF_GANESH_TEST(GrBufferAllocPool_CpuBufferAndBlockSizeCap,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, options);
    if (!context) {
        ERRORF(reporter, "Could not create mock GrDirectContext.");
        return;
    }
    GrGpu* gpu = context->priv().getGpu();
    auto cache = GrBufferAllocPool::CpuBufferCache::Make(/*maxBuffersToCache=*/2);

    REPORTER_ASSERT(reporter, GrCpuBuffer::Make(/*size=*/0) == nullptr);
    REPORTER_ASSERT(reporter,
                    GrCpuBuffer::Make(std::numeric_limits<size_t>::max() - 8) == nullptr);
    REPORTER_ASSERT(reporter, cache->makeBuffer(/*size=*/0, /*mustBeInitialized=*/true) == nullptr);
    REPORTER_ASSERT(reporter,
                    cache->makeBuffer(std::numeric_limits<size_t>::max() - 8,
                                      /*mustBeInitialized=*/true) == nullptr);

    // Requests exceeding GrBufferAllocPool::kMaxBlockSize are rejected by createBlock.
    // (Note: createBlock's resetCpuData OOM rollback path after a successful getBuffer call
    // requires allocator fault injection and is not exercised here.)
    GrVertexBufferAllocPool vertexPool(gpu, nullptr);
    sk_sp<const GrBuffer> buffer = GrCpuBuffer::Make(/*size=*/16);
    int startVertex = 11;
    int actualCount = 22;
    constexpr size_t kVertexSize = 40;
    constexpr int kOverMaxVertices =
            static_cast<int>(GrBufferAllocPool::kMaxBlockSize / kVertexSize) + 2;

    REPORTER_ASSERT(reporter,
                    vertexPool.makeSpaceAtLeast(kVertexSize,
                                                kOverMaxVertices,
                                                kOverMaxVertices,
                                                &buffer,
                                                &startVertex,
                                                &actualCount) == nullptr);
    REPORTER_ASSERT(reporter, buffer == nullptr);
    REPORTER_ASSERT(reporter, startVertex == 0);
    REPORTER_ASSERT(reporter, actualCount == 0);

    // Index pool request exceeding kMaxBlockSize (INT_MAX * 2 > kMaxBlockSize).
    GrIndexBufferAllocPool indexPool(gpu, nullptr);
    buffer = GrCpuBuffer::Make(/*size=*/16);
    int startIndex = 33;
    int actualIndices = 44;
    REPORTER_ASSERT(reporter,
                    indexPool.makeSpaceAtLeast(std::numeric_limits<int>::max(),
                                               std::numeric_limits<int>::max(),
                                               &buffer,
                                               &startIndex,
                                               &actualIndices) == nullptr);
    REPORTER_ASSERT(reporter, buffer == nullptr);
    REPORTER_ASSERT(reporter, startIndex == 0);
    REPORTER_ASSERT(reporter, actualIndices == 0);

    // Indirect draw pool requests that overflow on 32-bit or exceed kMaxBlockSize on 64-bit.
    GrDrawIndirectBufferAllocPool indirectPool(gpu, nullptr);
    buffer = GrCpuBuffer::Make(/*size=*/16);
    size_t indirectOffset = 99;
    GrDrawIndirectWriter writer =
            indirectPool.makeSpace(std::numeric_limits<int>::max(), &buffer, &indirectOffset);
    REPORTER_ASSERT(reporter, !static_cast<bool>(writer));
    REPORTER_ASSERT(reporter, buffer == nullptr);
    REPORTER_ASSERT(reporter, indirectOffset == 0u);

    buffer = GrCpuBuffer::Make(/*size=*/16);
    indirectOffset = 99;
    GrDrawIndexedIndirectWriter indexedWriter = indirectPool.makeIndexedSpace(
            std::numeric_limits<int>::max(), &buffer, &indirectOffset);
    REPORTER_ASSERT(reporter, !static_cast<bool>(indexedWriter));
    REPORTER_ASSERT(reporter, buffer == nullptr);
    REPORTER_ASSERT(reporter, indirectOffset == 0u);
}

// Verifies invalid size/alignment/count rejection, 64-bit alignment handling, unaligned minSize
// rounding, and abandoned context failures.
DEF_GANESH_TEST(GrBufferAllocPool_InvalidInputsAndAbandonedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, options);
    if (!context) {
        ERRORF(reporter, "Could not create mock GrDirectContext.");
        return;
    }
    GrGpu* gpu = context->priv().getGpu();
    auto cache = GrBufferAllocPool::CpuBufferCache::Make(/*maxBuffersToCache=*/2);

    // 1. Base pool invalid sizes/alignments and 64-bit >32-bit alignment check.
    {
        TestBufferAllocPool rawPool(gpu, cache);
        sk_sp<const GrBuffer> buf = GrCpuBuffer::Make(/*size=*/16);
        size_t offset = 55;
        size_t actualSize = 66;

        REPORTER_ASSERT(reporter,
                        rawPool.makeSpace(/*size=*/0, /*alignment=*/16, &buf, &offset) == nullptr);
        REPORTER_ASSERT(reporter, buf == nullptr);
        REPORTER_ASSERT(reporter, offset == 0u);

        buf = GrCpuBuffer::Make(/*size=*/16);
        offset = 55;
        REPORTER_ASSERT(reporter,
                        rawPool.makeSpace(/*size=*/16, /*alignment=*/0, &buf, &offset) == nullptr);
        REPORTER_ASSERT(reporter, buf == nullptr);
        REPORTER_ASSERT(reporter, offset == 0u);

        buf = GrCpuBuffer::Make(/*size=*/16);
        offset = 55;
        actualSize = 66;
        REPORTER_ASSERT(reporter,
                        rawPool.makeSpaceAtLeast(/*minSize=*/64,
                                                 /*fallbackSize=*/32,
                                                 /*alignment=*/16,
                                                 &buf,
                                                 &offset,
                                                 &actualSize) == nullptr);
        REPORTER_ASSERT(reporter, buf == nullptr);
        REPORTER_ASSERT(reporter, offset == 0u);
        REPORTER_ASSERT(reporter, actualSize == 0u);

        if constexpr (sizeof(size_t) == 8) {
            buf = GrCpuBuffer::Make(/*size=*/16);
            offset = 55;
            actualSize = 66;
            // Alignment >= 2^32 exceeds kMaxBlockSize and is rejected without truncating alignment
            // to 32 bits.
            constexpr uint64_t kAlignment4GiB = uint64_t{1} << 32;
            REPORTER_ASSERT(reporter,
                            rawPool.makeSpaceAtLeast(/*minSize=*/1,
                                                     /*fallbackSize=*/1,
                                                     static_cast<size_t>(kAlignment4GiB),
                                                     &buf,
                                                     &offset,
                                                     &actualSize) == nullptr);
            REPORTER_ASSERT(reporter, buf == nullptr);
            REPORTER_ASSERT(reporter, offset == 0u);
            REPORTER_ASSERT(reporter, actualSize == 0u);
        }

        // Unaligned minSize near SIZE_MAX overflows minAlignedSize rounding.
        buf = GrCpuBuffer::Make(/*size=*/16);
        offset = 55;
        actualSize = 66;
        REPORTER_ASSERT(reporter,
                        rawPool.makeSpaceAtLeast(std::numeric_limits<size_t>::max() - 5,
                                                 std::numeric_limits<size_t>::max(),
                                                 /*alignment=*/16,
                                                 &buf,
                                                 &offset,
                                                 &actualSize) == nullptr);
        REPORTER_ASSERT(reporter, buf == nullptr);
        REPORTER_ASSERT(reporter, offset == 0u);
        REPORTER_ASSERT(reporter, actualSize == 0u);
    }

    // 2. Unaligned minSize rounds up to alignment before checking active-block capacity.
    {
        TestBufferAllocPool pool(gpu, cache);
        sk_sp<const GrBuffer> firstBuf;
        size_t firstOffset = 0;
        REPORTER_ASSERT(reporter,
                        pool.makeSpace(/*size=*/16, /*alignment=*/1, &firstBuf, &firstOffset) !=
                                nullptr);
        // Consume enough of firstBuf so usedBytes % 40 == 14 (giving pad == 26) and fBytesFree is
        // in [50, 89], leaving [24, 63] bytes after padding.
        const size_t bufSize = firstBuf->size();
        size_t targetUsed = bufSize - 50;
        size_t rem = targetUsed % 40;
        size_t delta = (rem >= 14) ? (rem - 14) : (40 + rem - 14);
        targetUsed -= delta;
        sk_sp<const GrBuffer> fillBuf;
        size_t fillOffset = 0;
        REPORTER_ASSERT(reporter,
                        pool.makeSpace(targetUsed - 16, /*alignment=*/1, &fillBuf, &fillOffset) !=
                                nullptr);
        // Requesting minSize = 45 with alignment = 40 requires minAlignedSize = 80, which exceeds
        // the [24, 63] bytes remaining after padding in firstBuf and must allocate a new block.
        sk_sp<const GrBuffer> secondBuf;
        size_t secondOffset = 99;
        size_t actualSize = 0;
        void* secondPtr = pool.makeSpaceAtLeast(/*minSize=*/45,
                                                /*fallbackSize=*/80,
                                                /*alignment=*/40,
                                                &secondBuf,
                                                &secondOffset,
                                                &actualSize);
        REPORTER_ASSERT(reporter, secondPtr != nullptr);
        REPORTER_ASSERT(reporter, secondBuf.get() != firstBuf.get());
        REPORTER_ASSERT(reporter, secondOffset == 0u);
        REPORTER_ASSERT(reporter, actualSize >= 80u);
        REPORTER_ASSERT(reporter, actualSize % 40 == 0u);
        REPORTER_ASSERT(reporter, pool.unmap());
    }

    // 3. Vertex pool multiplication overflow, zero vertexSize, and abandoned context failure.
    {
        GrVertexBufferAllocPool vertexPool(gpu, nullptr);
        sk_sp<const GrBuffer> buffer = GrCpuBuffer::Make(/*size=*/16);
        int startVertex = 11;
        int actualCount = 22;

        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpace(std::numeric_limits<size_t>::max() / 2 + 2,
                                             /*vertexCount=*/4,
                                             &buffer,
                                             &startVertex) == nullptr);
        REPORTER_ASSERT(reporter, buffer == nullptr);
        REPORTER_ASSERT(reporter, startVertex == 0);

        buffer = GrCpuBuffer::Make(/*size=*/16);
        startVertex = 11;
        actualCount = 22;
        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpaceAtLeast(std::numeric_limits<size_t>::max() / 2 + 2,
                                                    /*minVertexCount=*/4,
                                                    /*fallbackVertexCount=*/4,
                                                    &buffer,
                                                    &startVertex,
                                                    &actualCount) == nullptr);
        REPORTER_ASSERT(reporter, buffer == nullptr);
        REPORTER_ASSERT(reporter, startVertex == 0);
        REPORTER_ASSERT(reporter, actualCount == 0);

        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpace(
                                /*vertexSize=*/0, /*vertexCount=*/8, &buffer, &startVertex) ==
                                nullptr);
        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpaceAtLeast(/*vertexSize=*/0,
                                                    /*minVertexCount=*/4,
                                                    /*fallbackVertexCount=*/8,
                                                    &buffer,
                                                    &startVertex,
                                                    &actualCount) == nullptr);

#if !defined(SK_DEBUG)
        // Non-positive and inverted counts SkASSERT in debug builds and return nullptr in release.
        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpace(
                                /*vertexSize=*/16, /*vertexCount=*/0, &buffer, &startVertex) ==
                                nullptr);
        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpace(
                                /*vertexSize=*/16, /*vertexCount=*/-4, &buffer, &startVertex) ==
                                nullptr);
        REPORTER_ASSERT(reporter,
                        vertexPool.makeSpaceAtLeast(/*vertexSize=*/16,
                                                    /*minVertexCount=*/8,
                                                    /*fallbackVertexCount=*/4,
                                                    &buffer,
                                                    &startVertex,
                                                    &actualCount) == nullptr);

        GrIndexBufferAllocPool indexPool(gpu, nullptr);
        int startIndex = 33;
        int actualIndices = 44;
        REPORTER_ASSERT(reporter,
                        indexPool.makeSpace(/*indexCount=*/0, &buffer, &startIndex) == nullptr);
        REPORTER_ASSERT(reporter,
                        indexPool.makeSpace(/*indexCount=*/-1, &buffer, &startIndex) == nullptr);
        REPORTER_ASSERT(reporter,
                        indexPool.makeSpaceAtLeast(/*minIndexCount=*/12,
                                                   /*fallbackIndexCount=*/6,
                                                   &buffer,
                                                   &startIndex,
                                                   &actualIndices) == nullptr);

        GrDrawIndirectBufferAllocPool indirectPool(gpu, nullptr);
        size_t indirectOffset = 99;
        REPORTER_ASSERT(reporter,
                        !static_cast<bool>(indirectPool.makeSpace(
                                /*drawCount=*/0, &buffer, &indirectOffset)));
        REPORTER_ASSERT(reporter, indirectOffset == 0u);
        indirectOffset = 99;
        REPORTER_ASSERT(reporter,
                        !static_cast<bool>(indirectPool.makeSpace(
                                /*drawCount=*/-2, &buffer, &indirectOffset)));
        REPORTER_ASSERT(reporter, indirectOffset == 0u);
        indirectOffset = 99;
        REPORTER_ASSERT(reporter,
                        !static_cast<bool>(indirectPool.makeIndexedSpace(
                                /*drawCount=*/0, &buffer, &indirectOffset)));
        REPORTER_ASSERT(reporter, indirectOffset == 0u);
#endif

        sk_sp<GrDirectContext> abandonedCtx = GrDirectContext::MakeMock(&mockOptions, options);
        if (!abandonedCtx) {
            ERRORF(reporter, "Could not create mock GrDirectContext for abandon test.");
            return;
        }
        abandonedCtx->abandonContext();
        GrGpu* abandonedGpu = abandonedCtx->priv().getGpu();
        GrVertexBufferAllocPool failedVertexPool(abandonedGpu, nullptr);
        buffer = GrCpuBuffer::Make(/*size=*/16);
        startVertex = 42;
        actualCount = 99;
        REPORTER_ASSERT(reporter,
                        failedVertexPool.makeSpaceAtLeast(/*vertexSize=*/16,
                                                          /*minVertexCount=*/8,
                                                          /*fallbackVertexCount=*/32,
                                                          &buffer,
                                                          &startVertex,
                                                          &actualCount) == nullptr);
        REPORTER_ASSERT(reporter, buffer == nullptr);
        REPORTER_ASSERT(reporter, startVertex == 0);
        REPORTER_ASSERT(reporter, actualCount == 0);
    }
}

// Verifies that GrVertexChunkBuilder caps heuristic preallocation counts on the real
// GrOpFlushState path (without multi-gigabyte allocations) and retries with a default-sized chunk
// when the target rejects the initial preallocation request.
DEF_GANESH_TEST(GrBufferAllocPool_ChunkBuilderPreallocCapAndFallback,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, options);
    if (!context) {
        ERRORF(reporter, "Could not create mock GrDirectContext.");
        return;
    }
    GrGpu* gpu = context->priv().getGpu();
    auto cache = GrBufferAllocPool::CpuBufferCache::Make(/*maxBuffersToCache=*/2);

    const int hugePrealloc = skgpu::tess::FixedCountStrokes::PreallocCount(53687087);

    // 1. Real GrOpFlushState path: hugePrealloc is capped to 512 KiB (16 * kDefaultBufferSize)
    //    on the first chunk, and subsequent chunks double in capacity when filled with vertices.
    {
        GrOpFlushState flushState(gpu, context->priv().resourceProvider(), nullptr, cache);
        sk_sp<const GrBuffer> fillBuf;
        int fillStart = -1;
        void* fillPtr = flushState.makeVertexSpace(
                /*vertexSize=*/1,
                static_cast<int>(GrBufferAllocPool::kDefaultBufferSize),
                &fillBuf,
                &fillStart);
        REPORTER_ASSERT(reporter, fillPtr != nullptr);

        constexpr size_t kStride = 40;
        constexpr int kInitialCapVertices =
                static_cast<int>((16 * GrBufferAllocPool::kDefaultBufferSize) / kStride);
        GrVertexChunkArray chunks;
        int appendedVertices = 0;
        {
            GrVertexChunkBuilder chunkBuilder(
                    &flushState, &chunks, kStride, /*minVerticesPerChunk=*/hugePrealloc);
            // Append vertices until the first chunk fills and spills into a second, doubled chunk
            // (with an upper limit on iterations so the test does not depend on exact bin sizes).
            constexpr int kMaxIterations = kInitialCapVertices * 4;
            for (; appendedVertices < kMaxIterations && chunks.size() < 2; ++appendedVertices) {
                skgpu::VertexWriter vw = chunkBuilder.appendVertices(1);
                if (!vw) {
                    ERRORF(reporter, "Failed to append vertex %d", appendedVertices);
                    break;
                }
            }
        }
        REPORTER_ASSERT(reporter, chunks.size() == 2);
        if (chunks.size() == 2) {
            REPORTER_ASSERT(reporter, chunks[0].fBuffer != nullptr);
            REPORTER_ASSERT(reporter, chunks[1].fBuffer != nullptr);
            REPORTER_ASSERT(
                    reporter,
                    chunks[0].fBuffer->size() <= 16 * GrBufferAllocPool::kDefaultBufferSize);
            REPORTER_ASSERT(reporter, chunks[1].fBuffer->size() >= 2 * chunks[0].fBuffer->size());
            REPORTER_ASSERT(reporter, chunks[0].fCount + chunks[1].fCount == appendedVertices);
            REPORTER_ASSERT(reporter, chunks[1].fCount == 1);
        }
    }

    // 2. Retry fallback when the target rejects the 512 KiB preallocation hint. Also verifies that
    //    the first request is clamped to (16 * kDefaultBufferSize) / stride vertices.
    {
        constexpr int kExpectedInitialVertices =
                static_cast<int>((16 * GrBufferAllocPool::kDefaultBufferSize) / 40);
        CapLimitedMockTarget target(context);
        GrVertexChunkArray chunks;
        {
            GrVertexChunkBuilder chunkBuilder(
                    &target, &chunks, /*stride=*/40, /*minVerticesPerChunk=*/hugePrealloc);
            skgpu::VertexWriter vw = chunkBuilder.appendVertices(1);
            REPORTER_ASSERT(reporter, static_cast<bool>(vw));
        }
        REPORTER_ASSERT(reporter, target.firstMinVertexCount() == kExpectedInitialVertices);
        REPORTER_ASSERT(reporter, chunks.size() == 1);
        REPORTER_ASSERT(reporter, chunks[0].fCount == 1);
        REPORTER_ASSERT(reporter, chunks[0].fBuffer != nullptr);
    }
}
