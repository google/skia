/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrOp.h"
#include "tests/Test.h"
#include <iterator>

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

/**
 * The index in Combinable for the result for combining op 'b' into op 'a', i.e. the result of
 * op[a]->combineIfPossible(op[b]).
 */
int64_t combinable_index(int a, int b) {
    SkASSERT(b != a);
    // Each index gets kNumOps - 1 contiguous bools
    int64_t aOffset = a * (kNumOps - 1);
    // Within a's range we have one value each other op, but not one for a itself.
    int64_t bIdxInA = b < a ? b : b - 1;
    return aOffset + bIdxInA;
}

/**
 * Creates a legal set of combinability results for the ops. The likelihood that any two ops
 * in a group can merge is randomly chosen.
 */
static void init_combinable(int numGroups, Combinable* combinable, SkRandom* random) {
    SkScalar mergeProbability = random->nextUScalar1();
    std::fill_n(combinable->begin(), kNumCombinableValues, GrOp::CombineResult::kCannotCombine);
    SkTDArray<int> groups[kNumOps];
    for (int i = 0; i < kNumOps; ++i) {
        auto& group = groups[random->nextULessThan(numGroups)];
        for (int g = 0; g < group.count(); ++g) {
            int j = group[g];
            if (random->nextUScalar1() < mergeProbability) {
                (*combinable)[combinable_index(i, j)] = GrOp::CombineResult::kMerged;
            } else {
                (*combinable)[combinable_index(i, j)] = GrOp::CombineResult::kMayChain;
            }
            if (random->nextUScalar1() < mergeProbability) {
                (*combinable)[combinable_index(j, i)] = GrOp::CombineResult::kMerged;
            } else {
                (*combinable)[combinable_index(j, i)] = GrOp::CombineResult::kMayChain;
            }
        }
        group.push_back(i);
    }
}

/**
 * A simple test op. It has an integer position, p. When it executes it writes p into an array
 * of ints at index p and p+1. It takes a bitfield that indicates allowed pair-wise chainings.
 */
class TestOp : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context, int value, const Range& range,
                            int result[], const Combinable* combinable) {
        return GrOp::Make<TestOp>(context, value, range, result, combinable);
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
    friend class ::GrOp;  // for ctor

    TestOp(int value, const Range& range, int result[], const Combinable* combinable)
            : INHERITED(ClassID()), fResult(result), fCombinable(combinable) {
        fValueRanges.push_back({value, range});
        this->setBounds(SkRect::MakeXYWH(range.fOffset, 0, range.fOffset + range.fLength, 1),
                        HasAABloat::kNo, IsHairline::kNo);
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override {
        for (auto& op : ChainRange<TestOp>(this)) {
            op.writeResult(fResult);
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc* arenas, const GrCaps&) override {
        // This op doesn't use the arenas, but make sure the GrOpsTask is sending it
        SkASSERT(arenas);
        (void) arenas;
        auto that = t->cast<TestOp>();
        int v0 = fValueRanges[0].fValue;
        int v1 = that->fValueRanges[0].fValue;
        auto result = (*fCombinable)[combinable_index(v0, v1)];
        if (result == GrOp::CombineResult::kMerged) {
            std::move(that->fValueRanges.begin(), that->fValueRanges.end(),
                      std::back_inserter(fValueRanges));
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

    using INHERITED = GrOp;
};
}  // namespace

/**
 * Tests adding kNumOps to an op list with all possible allowed chaining configurations. Tests
 * adding the ops in all possible orders and verifies that the chained executions don't violate
 * painter's order.
 */
DEF_GPUTEST(OpChainTest, reporter, /*ctxInfo*/) {
    sk_sp<GrDirectContext> dContext = GrDirectContext::MakeMock(nullptr);
    SkASSERT(dContext);
    const GrCaps* caps = dContext->priv().caps();
    static constexpr SkISize kDims = {kNumOps + 1, 1};

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kYes);

    static const GrSurfaceOrigin kOrigin = kTopLeft_GrSurfaceOrigin;
    auto proxy = dContext->priv().proxyProvider()->createProxy(
            format, kDims, GrRenderable::kYes, 1, GrMipmapped::kNo, SkBackingFit::kExact,
            SkBudgeted::kNo, GrProtected::kNo, GrInternalSurfaceFlags::kNone);
    SkASSERT(proxy);
    proxy->instantiate(dContext->priv().resourceProvider());

    GrSwizzle writeSwizzle = caps->getWriteSwizzle(format, GrColorType::kRGBA_8888);

    int result[result_width()];
    int validResult[result_width()];

    int permutation[kNumOps];
    for (int i = 0; i < kNumOps; ++i) {
        permutation[i] = i;
    }
    // Op order permutations.
    static constexpr int kNumPermutations = 100;
    // For a given number of chainability groups, this is the number of random combinability reuslts
    // we will test.
    static constexpr int kNumCombinabilitiesPerGrouping = 20;
    SkRandom random;
    bool repeat = false;
    Combinable combinable;
    GrDrawingManager* drawingMgr = dContext->priv().drawingManager();
    for (int p = 0; p < kNumPermutations; ++p) {
        for (int i = 0; i < kNumOps - 2 && !repeat; ++i) {
            // The current implementation of nextULessThan() is biased. :(
            unsigned j = i + random.nextULessThan(kNumOps - i);
            std::swap(permutation[i], permutation[j]);
        }
        // g is the number of chainable groups that we partition the ops into.
        for (int g = 1; g < kNumOps; ++g) {
            for (int c = 0; c < kNumCombinabilitiesPerGrouping; ++c) {
                init_combinable(g, &combinable, &random);
                GrTokenTracker tracker;
                GrOpFlushState flushState(dContext->priv().getGpu(),
                                          dContext->priv().resourceProvider(),
                                          &tracker);
                GrOpsTask opsTask(drawingMgr,
                                  dContext->priv().arenas(),
                                  GrSurfaceProxyView(proxy, kOrigin, writeSwizzle),
                                  dContext->priv().auditTrail());
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
                    auto op = TestOp::Make(dContext.get(), value, range, result, &combinable);
                    TestOp* testOp = (TestOp*)op.get();
                    testOp->writeResult(validResult);
                    opsTask.addOp(drawingMgr, std::move(op),
                                  GrTextureResolveManager(dContext->priv().drawingManager()),
                                  *caps);
                }
                opsTask.makeClosed(*caps);
                opsTask.prepare(&flushState);
                opsTask.execute(&flushState);
                opsTask.endFlush(drawingMgr);
                opsTask.disown(drawingMgr);
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
}
