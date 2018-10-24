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

// We create Ops that write a value into a range of a buffer. We create ranges from
// kNumOpPositions starting positions x kRanges canonical ranges. We repeat each range kNumRepeats
// times (with a different value written by each of the repeats).
namespace {
struct Range {
    unsigned fOffset;
    unsigned fLength;
};

static constexpr int kNumOpPositions = 4;
static constexpr Range kRanges[] = {{0, 4,}, {1, 2}};
static constexpr int kNumRanges = (int)SK_ARRAY_COUNT(kRanges);
static constexpr int kNumRepeats = 2;
static constexpr int kNumOps = kNumRepeats * kNumOpPositions * kNumRanges;

static constexpr uint64_t fact(int n) {
    assert(n > 0);
    return n > 1 ? n * fact(n - 1) : 1;
}

// How wide should our result buffer be to hold values written by the ranges of the ops.
static constexpr unsigned result_width() {
    unsigned maxLength = 0;
    for (size_t i = 0; i < kNumRanges; ++i) {
        maxLength = maxLength > kRanges[i].fLength ? maxLength : kRanges[i].fLength;
    }
    return kNumOpPositions + maxLength - 1;
}

// Number of possible allowable binary chainings among the kNumOps ops.
static constexpr int kNumCombinableValues = fact(kNumOps) / fact(kNumOps - 2);
using Combinable = std::array<GrOp::CombineResult, kNumCombinableValues>;

/** What should the result be for combining op with value a with op with value b. */
static GrOp::CombineResult combine_result(int a, int b, const Combinable& combinable) {
    SkASSERT(b != a);
    // Each index gets kNumOps - 1 contiguous bools
    int aOffset = a * (kNumOps - 1);
    // Within a's range we have one value each other op, but not one for a itself.
    int64_t bIdxInA = b < a ? b : b - 1;
    return combinable[aOffset + bIdxInA];
}

/**
 * A simple test op. It has an integer position, p. When it executes it writes p into an array
 * of ints at index p and p+1. It takes a bitfield that indicates allowed pair-wise chainings.
 */
class TestOp : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<TestOp> Make(GrContext* context, int value, const Range& range,
                                        int result[], const Combinable* combinable) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
        return pool->allocate<TestOp>(value, range, result, combinable);
    }

    const char* name() const override { return "TestOp"; }

    void writeResult(int result[]) const {
        for (const auto& op : ChainRange<TestOp>(this)) {
            for (const auto& vr : op.fValueRanges) {
                for (unsigned i = 0; i < vr.fRange.fLength; ++i) {
                    result[vr.fRange.fOffset + i] = vr.fValue;
                }
            }
        }
    }

private:
    friend class ::GrOpMemoryPool;  // for ctor

    TestOp(int value, const Range& range, int result[], const Combinable* combinable)
            : INHERITED(ClassID()), fResult(result), fCombinable(combinable) {
        fValueRanges.push_back({value, range});
        this->setBounds(SkRect::MakeXYWH(range.fOffset, 0, range.fOffset + range.fLength, 1),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*) override {
        for (auto& op : ChainRange<TestOp>(this)) {
            op.writeResult(fResult);
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps&) override {
        auto that = t->cast<TestOp>();
        auto result =
                combine_result(fValueRanges[0].fValue, that->fValueRanges[0].fValue, *fCombinable);
        // Op chaining rules bar us from merging a chained that. GrOp asserts this.
        if (that->isChained() && result == CombineResult::kMerged) {
            return CombineResult::kCannotCombine;
        }
        if (result == GrOp::CombineResult::kMerged) {
            std::move(that->fValueRanges.begin(), that->fValueRanges.end(),
                      std::back_inserter(fValueRanges));
            this->joinBounds(*that);
        }
        return result;
    }

    struct ValueRange {
        int fValue;
        Range fRange;
    };
    std::vector<ValueRange> fValueRanges;
    int* fResult;
    const Combinable* fCombinable;

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
    int result[result_width()];
    int validResult[result_width()];

    int permutation[kNumOps];
    for (int i = 0; i < kNumOps; ++i) {
        permutation[i] = i;
    }
    static constexpr int kNumPermutations = 100;
    static constexpr int kNumCombinabilities = 100;
    SkRandom random;
    bool repeat = false;
    Combinable combinable;
    for (int p = 0; p < kNumPermutations; ++p) {
        for (int i = 0; i < kNumOps - 2 && !repeat; ++i) {
            // The current implementation of nextULessThan() is biased. :(
            unsigned j = i + random.nextULessThan(kNumOps - i);
            std::swap(permutation[i], permutation[j]);
        }
        for (int c = 0; c < kNumCombinabilities; ++c) {
            for (int i = 0; i < kNumCombinableValues && !repeat; ++i) {
                combinable[i] = static_cast<GrOp::CombineResult>(random.nextULessThan(3));
            }
            GrTokenTracker tracker;
            GrOpFlushState flushState(context->contextPriv().getGpu(),
                                      context->contextPriv().resourceProvider(), &tracker);
            GrRenderTargetOpList opList(context->contextPriv().resourceProvider(),
                                        sk_ref_sp(context->contextPriv().opMemoryPool()),
                                        proxy->asRenderTargetProxy(),
                                        context->contextPriv().getAuditTrail());
            // This assumes the particular values of kRanges.
            std::fill_n(result, result_width(), -1);
            std::fill_n(validResult, result_width(), -1);
            for (int i = 0; i < kNumOps; ++i) {
                int value = permutation[i];
                // factor out the repeats and then use the canonical starting position and range
                // to determine an actual range.
                int j = value % (kNumRanges * kNumOpPositions);
                int pos = j % kNumOpPositions;
                Range range = kRanges[j / kNumOpPositions];
                range.fOffset += pos;
                auto op = TestOp::Make(context.get(), value, range, result, &combinable);
                op->writeResult(validResult);
                opList.addOp(std::move(op), *context->contextPriv().caps());
            }
            opList.makeClosed(*context->contextPriv().caps());
            opList.prepare(&flushState);
            opList.execute(&flushState);
            opList.endFlush();
#if 0  // Useful to repeat a random configuration that fails the test while debugger attached.
            if (!std::equal(result, result + result_width(), validResult)) {
                repeat = true;
            }
#endif
            (void)repeat;
            REPORTER_ASSERT(reporter, std::equal(result, result + result_width(), validResult));
        }
    }
}
