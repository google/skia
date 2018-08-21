/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetOpList.h"
#include "Test.h"
#include "ops/GrOp.h"

// The number of configurations is 2^Permutations(kNumOps, 2) * kNumOps!. To make this any larger
// we'd have to probabilistically sample the space. At four this finishes in a very reasonable
// amount of time but at five it takes nearly 2 minutes in a Release build on a Z840. Four seems
// like it generates sufficiently complex test cases.
static constexpr int kNumOps = 4;

static constexpr int fact(int n) {
    assert(n > 0);
    return n > 1 ? n * fact(n - 1) : 1;
}

// Number of possible allowable binary chainings among the kNumOps ops.
static constexpr int kNumChainabilityBits = fact(kNumOps) / fact(kNumOps - 2);

// We store the chainability booleans as a 32 bit bitfield.
GR_STATIC_ASSERT(kNumChainabilityBits <= 32);

static constexpr uint64_t kNumChainabilityPermutations = 1 << kNumChainabilityBits;

/** Is op a chainable to op b as indicated by the bitfield? */
static bool is_chainable(int a, int b, uint32_t chainabilityBits) {
    SkASSERT(b != a);
    // Each index gets kNumOps - 1 contiguous bits
    int aOffset = a * (kNumOps - 1);
    // Within a's range we give one bit each other op, but not one for itself.
    int bIdxInA = b < a ? b : b - 1;
    return chainabilityBits & (1 << (aOffset + bIdxInA));
}

namespace {
/**
 * A simple test op. It has an integer position, p. When it executes it writes p into an array
 * of ints at index p and p+1. It takes a bitfield that indicates allowed pair-wise chainings.
 */
class TestOp : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<TestOp> Make(GrContext* context, int pos, int result[],
                                        uint32_t chainability) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
        return pool->allocate<TestOp>(pos, result, chainability);
    }

    const char* name() const override { return "TestOp"; }

    void writeResult(int result[]) const { result[fPos + 1] = result[fPos] = fPos; }

private:
    friend class ::GrOpMemoryPool;  // for ctor

    TestOp(int pos, int result[], uint32_t chainability)
            : INHERITED(ClassID()), fPos(pos), fResult(result), fChainability(chainability) {
        // Each op writes 2 values (at pos and pos+1) in a (kNumOps + 1) x 1 buffer.
        this->setBounds(SkRect::MakeXYWH(pos, 0, 2, 1), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*) override {
        for (auto& op : ChainRange<TestOp>(this)) {
            op.writeResult(fResult);
        }
    }

    CombineResult onCombineIfPossible(GrOp* that, const GrCaps&) override {
        return is_chainable(fPos, that->cast<TestOp>()->fPos, fChainability)
                       ? CombineResult::kMayChain
                       : CombineResult::kCannotCombine;
    }

    int fPos;
    int* fResult;
    uint32_t fChainability;

    typedef GrOp INHERITED;
};
}  // namespace

/**
 * Tests adding kNumOps to an op list with all possible allowed chaining configurations. Tests
 * adding the ops in all possible orders and verifies that the chained executions don't violate
 * painter's order.
 */
DEF_GPUTEST(OpChainTest, reporter, /*ctxInfo*/) {
    auto context = GrContext::MakeMock(nullptr);
    SkASSERT(context);
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth = kNumOps + 1;
    desc.fHeight = 1;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;

    auto proxy = context->contextPriv().proxyProvider()->createProxy(
            desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, SkBackingFit::kExact, SkBudgeted::kNo,
            GrInternalSurfaceFlags::kNone);
    SkASSERT(proxy);
    proxy->instantiate(context->contextPriv().resourceProvider());
    int result[kNumOps + 1];
    int validResult[kNumOps + 1];

    int permutation[kNumOps];
    for (int i = 0; i < kNumOps; ++i) {
        permutation[i] = i;
    }
    do {
        for (uint32_t chainabilityBits = 0; chainabilityBits < kNumChainabilityPermutations;
             ++chainabilityBits) {
            GrTokenTracker tracker;
            GrOpFlushState flushState(context->contextPriv().getGpu(),
                                      context->contextPriv().resourceProvider(), &tracker);
            GrRenderTargetOpList opList(context->contextPriv().resourceProvider(),
                                        sk_ref_sp(context->contextPriv().opMemoryPool()),
                                        proxy->asRenderTargetProxy(),
                                        context->contextPriv().getAuditTrail());
            std::fill(result, result + kNumOps + 1, -1);
            std::fill(validResult, validResult + kNumOps + 1, -1);
            for (int i = 0; i < kNumOps; ++i) {
                auto op = TestOp::Make(context.get(), permutation[i], result, chainabilityBits);
                op->writeResult(validResult);
                opList.addOp(std::move(op), *context->contextPriv().caps());
            }
            opList.makeClosed(*context->contextPriv().caps());
            opList.prepare(&flushState);
            opList.execute(&flushState);
            opList.endFlush();
            REPORTER_ASSERT(reporter, std::equal(result, result + kNumOps + 1, validResult));
        }
    } while (std::next_permutation(permutation, permutation + kNumOps));
}
