
/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClipStack.h"
#include "tests/Test.h"

#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

namespace {

class TestCaseBuilder;
class ElementsBuilder;

enum class SavePolicy {
    kNever,
    kAtStart,
    kAtEnd,
    kBetweenEveryOp
};
// TODO: We could add a RestorePolicy enum that tests different places to restore, but that would
// make defining the test expectations and order independence more cumbersome.

class TestCase {
public:
    // Provides fluent API to describe actual clip commands and expected clip elements:
    // TestCase test = TestCase::Build("example", deviceBounds)
    //                          .actual().rect(r, GrAA::kYes, SkClipOp::kIntersect)
    //                                   .localToDevice(matrix)
    //                                   .nonAA()
    //                                   .difference()
    //                                   .path(p1)
    //                                   .path(p2)
    //                                   .finishElements()
    //                          .expectedState(kDeviceRect)
    //                          .expectedBounds(r.roundOut())
    //                          .expect().rect(r, GrAA::kYes, SkClipOp::kIntersect)
    //                                   .finishElements()
    //                          .finishTest();
    static TestCaseBuilder Build(const char* name, const SkIRect& deviceBounds);

    void run(const std::vector<int>& order, SavePolicy policy, skiatest::Reporter* reporter) const;

    const SkIRect& deviceBounds() const { return fDeviceBounds; }
    GrClipStack::ClipState expectedState() const { return fExpectedState; }
    const std::vector<GrClipStack::Element>& initialElements() const { return fElements; }
    const std::vector<GrClipStack::Element>& expectedElements() const { return fExpectedElements; }

private:
    friend class TestCaseBuilder;

    TestCase(SkString name,
             const SkIRect& deviceBounds,
             GrClipStack::ClipState expectedState,
             std::vector<GrClipStack::Element> actual,
             std::vector<GrClipStack::Element> expected)
        : fName(name)
        , fElements(std::move(actual))
        , fDeviceBounds(deviceBounds)
        , fExpectedElements(std::move(expected))
        , fExpectedState(expectedState) {}

    SkString getTestName(const std::vector<int>& order, SavePolicy policy) const;

    // This may be tighter than GrClipStack::getConservativeBounds() because this always accounts
    // for difference ops, whereas GrClipStack only sometimes can subtract the inner bounds for a
    // difference op.
    std::pair<SkIRect, bool> getOptimalBounds() const;

    SkString fName;

    // The input shapes+state to GrClipStack
    std::vector<GrClipStack::Element> fElements;
    SkIRect fDeviceBounds;

    // The expected output of iterating over the GrClipStack after all fElements are added, although
    // order is not important
    std::vector<GrClipStack::Element> fExpectedElements;
    GrClipStack::ClipState fExpectedState;
};

class ElementsBuilder {
public:
    // Update the default matrix, aa, and op state for elements that are added.
    ElementsBuilder& localToDevice(const SkMatrix& m) {  fLocalToDevice = m; return *this; }
    ElementsBuilder& aa() { fAA = GrAA::kYes; return *this; }
    ElementsBuilder& nonAA() { fAA = GrAA::kNo; return *this; }
    ElementsBuilder& intersect() { fOp = SkClipOp::kIntersect; return *this; }
    ElementsBuilder& difference() { fOp = SkClipOp::kDifference; return *this; }

    // Add rect, rrect, or paths to the list of elements, possibly overriding the last set
    // matrix, aa, and op state.
    ElementsBuilder& rect(const SkRect& rect) {
        return this->rect(rect, fLocalToDevice, fAA, fOp);
    }
    ElementsBuilder& rect(const SkRect& rect, GrAA aa, SkClipOp op) {
        return this->rect(rect, fLocalToDevice, aa, op);
    }
    ElementsBuilder& rect(const SkRect& rect, const SkMatrix& m, GrAA aa, SkClipOp op) {
        fElements->push_back({GrShape(rect), m, op, aa});
        return *this;
    }

    ElementsBuilder& rrect(const SkRRect& rrect) {
        return this->rrect(rrect, fLocalToDevice, fAA, fOp);
    }
    ElementsBuilder& rrect(const SkRRect& rrect, GrAA aa, SkClipOp op) {
        return this->rrect(rrect, fLocalToDevice, aa, op);
    }
    ElementsBuilder& rrect(const SkRRect& rrect, const SkMatrix& m, GrAA aa, SkClipOp op) {
        fElements->push_back({GrShape(rrect), m, op, aa});
        return *this;
    }

    ElementsBuilder& path(const SkPath& path) {
        return this->path(path, fLocalToDevice, fAA, fOp);
    }
    ElementsBuilder& path(const SkPath& path, GrAA aa, SkClipOp op) {
        return this->path(path, fLocalToDevice, aa, op);
    }
    ElementsBuilder& path(const SkPath& path, const SkMatrix& m, GrAA aa, SkClipOp op) {
        fElements->push_back({GrShape(path), m, op, aa});
        return *this;
    }

    // Finish and return the original test case builder
    TestCaseBuilder& finishElements() {
        return *fBuilder;
    }

private:
    friend class TestCaseBuilder;

    ElementsBuilder(TestCaseBuilder* builder, std::vector<GrClipStack::Element>* elements)
            : fBuilder(builder)
            , fElements(elements) {}

    SkMatrix fLocalToDevice = SkMatrix::I();
    GrAA     fAA = GrAA::kNo;
    SkClipOp fOp = SkClipOp::kIntersect;

    TestCaseBuilder*                   fBuilder;
    std::vector<GrClipStack::Element>* fElements;
};

class TestCaseBuilder {
public:
    ElementsBuilder actual() { return ElementsBuilder(this, &fActualElements); }
    ElementsBuilder expect() { return ElementsBuilder(this, &fExpectedElements); }

    TestCaseBuilder& expectActual() {
        fExpectedElements = fActualElements;
        return *this;
    }

    TestCaseBuilder& state(GrClipStack::ClipState state) {
        fExpectedState = state;
        return *this;
    }

    TestCase finishTest() {
        TestCase test(fName, fDeviceBounds, fExpectedState,
                      std::move(fActualElements), std::move(fExpectedElements));

        fExpectedState = GrClipStack::ClipState::kWideOpen;
        return test;
    }

private:
    friend class TestCase;

    explicit TestCaseBuilder(const char* name, const SkIRect& deviceBounds)
            : fName(name)
            , fDeviceBounds(deviceBounds)
            , fExpectedState(GrClipStack::ClipState::kWideOpen) {}

    SkString fName;
    SkIRect  fDeviceBounds;
    GrClipStack::ClipState fExpectedState;

    std::vector<GrClipStack::Element> fActualElements;
    std::vector<GrClipStack::Element> fExpectedElements;
};

TestCaseBuilder TestCase::Build(const char* name, const SkIRect& deviceBounds) {
    return TestCaseBuilder(name, deviceBounds);
}

SkString TestCase::getTestName(const std::vector<int>& order, SavePolicy policy) const {
    SkString name = fName;

    SkString policyName;
    switch(policy) {
        case SavePolicy::kNever:
            policyName = "never";
            break;
        case SavePolicy::kAtStart:
            policyName = "start";
            break;
        case SavePolicy::kAtEnd:
            policyName = "end";
            break;
        case SavePolicy::kBetweenEveryOp:
            policyName = "between";
            break;
    }

    name.appendf("(save %s, order [", policyName.c_str());
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) {
            name.append(",");
        }
        name.appendf("%d", order[i]);
    }
    name.append("])");
    return name;
}

std::pair<SkIRect, bool> TestCase::getOptimalBounds() const {
    if (fExpectedState == GrClipStack::ClipState::kEmpty) {
        return {SkIRect::MakeEmpty(), true};
    }

    bool expectOptimal = true;
    SkRegion region(fDeviceBounds);
    for (const GrClipStack::Element& e : fExpectedElements) {
        bool intersect = (e.fOp == SkClipOp::kIntersect && !e.fShape.inverted()) ||
                         (e.fOp == SkClipOp::kDifference && e.fShape.inverted());

        SkIRect elementBounds;
        SkRegion::Op op;
        if (intersect) {
            op = SkRegion::kIntersect_Op;
            expectOptimal &= e.fLocalToDevice.isIdentity();
            elementBounds = GrClip::GetPixelIBounds(e.fLocalToDevice.mapRect(e.fShape.bounds()),
                                                    e.fAA, GrClip::BoundsType::kExterior);
        } else {
            op = SkRegion::kDifference_Op;
            expectOptimal = false;
            if (e.fShape.isRect() && e.fLocalToDevice.isIdentity()) {
                elementBounds = GrClip::GetPixelIBounds(e.fShape.rect(), e.fAA,
                                                        GrClip::BoundsType::kInterior);
            } else if (e.fShape.isRRect() && e.fLocalToDevice.isIdentity()) {
                elementBounds = GrClip::GetPixelIBounds(SkRRectPriv::InnerBounds(e.fShape.rrect()),
                                                        e.fAA, GrClip::BoundsType::kInterior);
            } else {
                elementBounds = SkIRect::MakeEmpty();
            }
        }

        region.op(SkRegion(elementBounds), op);
    }
    return {region.getBounds(), expectOptimal};
}

static bool compare_elements(const GrClipStack::Element& a, const GrClipStack::Element& b) {
    if (a.fAA != b.fAA || a.fOp != b.fOp || a.fLocalToDevice != b.fLocalToDevice ||
        a.fShape.type() != b.fShape.type()) {
        return false;
    }
    switch(a.fShape.type()) {
        case GrShape::Type::kRect:
            return a.fShape.rect() == b.fShape.rect();
        case GrShape::Type::kRRect:
            return a.fShape.rrect() == b.fShape.rrect();
        case GrShape::Type::kPath:
            // A path's points are never transformed, the only modification is fill type which does
            // not change the generation ID. For convex polygons, we check == so that more complex
            // test cases can be evaluated.
            return a.fShape.path().getGenerationID() == b.fShape.path().getGenerationID() ||
                   (a.fShape.convex() &&
                    a.fShape.segmentMask() == SkPathSegmentMask::kLine_SkPathSegmentMask &&
                    a.fShape.path() == b.fShape.path());
        default:
            SkDEBUGFAIL("Shape type not handled by test case yet.");
            return false;
    }
}

void TestCase::run(const std::vector<int>& order, SavePolicy policy,
                   skiatest::Reporter* reporter) const {
    SkASSERT(fElements.size() == order.size());

    SkSimpleMatrixProvider matrixProvider(SkMatrix::I());
    GrClipStack cs(fDeviceBounds, &matrixProvider, false);

    if (policy == SavePolicy::kAtStart) {
        cs.save();
    }

    for (int i : order) {
        if (policy == SavePolicy::kBetweenEveryOp) {
            cs.save();
        }
        const GrClipStack::Element& e = fElements[i];
        switch(e.fShape.type()) {
            case GrShape::Type::kRect:
                cs.clipRect(e.fLocalToDevice, e.fShape.rect(), e.fAA, e.fOp);
                break;
            case GrShape::Type::kRRect:
                cs.clipRRect(e.fLocalToDevice, e.fShape.rrect(), e.fAA, e.fOp);
                break;
            case GrShape::Type::kPath:
                cs.clipPath(e.fLocalToDevice, e.fShape.path(), e.fAA, e.fOp);
                break;
            default:
                SkDEBUGFAIL("Shape type not handled by test case yet.");
        }
    }

    if (policy == SavePolicy::kAtEnd) {
        cs.save();
    }

    // Now validate
    SkString name = this->getTestName(order, policy);
    REPORTER_ASSERT(reporter, cs.clipState() == fExpectedState,
                    "%s, clip state expected %d, actual %d",
                    name.c_str(), (int) fExpectedState, (int) cs.clipState());
    SkIRect actualBounds = cs.getConservativeBounds();
    SkIRect optimalBounds;
    bool expectOptimal;
    std::tie(optimalBounds, expectOptimal) = this->getOptimalBounds();

    if (expectOptimal) {
        REPORTER_ASSERT(reporter, actualBounds == optimalBounds,
                "%s, bounds expected [%d %d %d %d], actual [%d %d %d %d]",
                name.c_str(), optimalBounds.fLeft, optimalBounds.fTop,
                optimalBounds.fRight, optimalBounds.fBottom,
                actualBounds.fLeft, actualBounds.fTop,
                actualBounds.fRight, actualBounds.fBottom);
    } else {
        REPORTER_ASSERT(reporter, actualBounds.contains(optimalBounds),
                "%s, bounds are not conservative, optimal [%d %d %d %d], actual [%d %d %d %d]",
                name.c_str(), optimalBounds.fLeft, optimalBounds.fTop,
                optimalBounds.fRight, optimalBounds.fBottom,
                actualBounds.fLeft, actualBounds.fTop,
                actualBounds.fRight, actualBounds.fBottom);
    }

    size_t matchedElements = 0;
    for (const GrClipStack::Element& a : cs) {
        bool found = false;
        for (const GrClipStack::Element& e : fExpectedElements) {
            if (compare_elements(a, e)) {
                // shouldn't match multiple expected elements or it's a bad test case
                SkASSERT(!found);
                found = true;
            }
        }

        REPORTER_ASSERT(reporter, found,
                        "%s, unexpected clip element in stack: shape %d, aa %d, op %d",
                        name.c_str(), (int) a.fShape.type(), (int) a.fAA, (int) a.fOp);
        matchedElements += found ? 1 : 0;
    }
    REPORTER_ASSERT(reporter, matchedElements == fExpectedElements.size(),
                    "%s, did not match all expected elements: expected %d but matched only %d",
                    name.c_str(), fExpectedElements.size(), matchedElements);

    // Validate restoration behavior
    if (policy == SavePolicy::kAtEnd) {
        GrClipStack::ClipState oldState = cs.clipState();
        cs.restore();
        REPORTER_ASSERT(reporter, cs.clipState() == oldState,
                        "%s, restoring an empty save record should not change clip state: "
                        "expected %d but got %d", (int) oldState, (int) cs.clipState());
    } else if (policy != SavePolicy::kNever) {
        int restoreCount = policy == SavePolicy::kAtStart ? 1 : (int) order.size();
        for (int i = 0; i < restoreCount; ++i) {
            cs.restore();
        }
        // Should be wide open if everything is restored to base state
        REPORTER_ASSERT(reporter, cs.clipState() == GrClipStack::ClipState::kWideOpen,
                        "%s, restore should make stack become wide-open, not %d",
                        (int) cs.clipState());
    }
}

// All clip operations are commutative so applying actual elements in every possible order should
// always produce the same set of expected elements.
static void run_test_case(skiatest::Reporter* r, const TestCase& test) {
    int n = (int) test.initialElements().size();
    std::vector<int> order(n);
    std::vector<int> stack(n);

    // Initial order sequence and zeroed stack
    for (int i = 0; i < n; ++i) {
        order[i] = i;
        stack[i] = 0;
    }

    auto runTest = [&]() {
        static const SavePolicy kPolicies[] = { SavePolicy::kNever, SavePolicy::kAtStart,
                                                SavePolicy::kAtEnd, SavePolicy::kBetweenEveryOp };
        for (auto policy : kPolicies) {
            test.run(order, policy, r);
        }
    };

    // Heap's algorithm (non-recursive) to generate every permutation over the test case's elements
    // https://en.wikipedia.org/wiki/Heap%27s_algorithm
    runTest();

    static constexpr int kMaxRuns = 720; // Don't run more than 6! configurations, even if n > 6
    int testRuns = 1;

    int i = 0;
    while (i < n && testRuns < kMaxRuns) {
        if (stack[i] < i) {
            using std::swap;
            if (i % 2 == 0) {
                swap(order[0], order[i]);
            } else {
                swap(order[stack[i]], order[i]);
            }

            runTest();
            stack[i]++;
            i = 0;
            testRuns++;
        } else {
            stack[i] = 0;
            ++i;
        }
    }
}

static SkPath make_octagon(const SkRect& r, SkScalar lr, SkScalar tb) {
    SkPath p;
    p.moveTo(r.fLeft + lr, r.fTop);
    p.lineTo(r.fRight - lr, r.fTop);
    p.lineTo(r.fRight, r.fTop + tb);
    p.lineTo(r.fRight, r.fBottom - tb);
    p.lineTo(r.fRight - lr, r.fBottom);
    p.lineTo(r.fLeft + lr, r.fBottom);
    p.lineTo(r.fLeft, r.fBottom - tb);
    p.lineTo(r.fLeft, r.fTop + tb);
    p.close();
    return p;
}

static SkPath make_octagon(const SkRect& r) {
    SkScalar lr = 0.3f * r.width();
    SkScalar tb = 0.3f * r.height();
    return make_octagon(r, lr, tb);
}

static constexpr SkIRect kDeviceBounds = {0, 0, 100, 100};

class NoOp : public GrDrawOp {
public:
    static NoOp* Get() {
        static NoOp gNoOp;
        return &gNoOp;
    }
private:
    DEFINE_OP_CLASS_ID
    NoOp() : GrDrawOp(ClassID()) {}
    const char* name() const override { return "NoOp"; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*, const
                      GrDstProxyView&, GrXferBarrierFlags, GrLoadOp) override {}
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*, const SkRect&) override {}
};

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
// These tests use the TestCase infrastructure to define clip stacks and
// associated expectations.

// Tests that the initialized state of the clip stack is wide-open
DEF_TEST(GrClipStack_InitialState, r) {
    run_test_case(r, TestCase::Build("initial-state", SkIRect::MakeWH(100, 100)).finishTest());
}

// Tests that intersection of rects combine to a single element when they have the same AA type,
// or are pixel-aligned.
DEF_TEST(GrClipStack_RectRectAACombine, r) {
    SkRect pixelAligned = {0, 0, 10, 10};
    SkRect fracRect1 = pixelAligned.makeOffset(5.3f, 3.7f);
    SkRect fracRect2 = {fracRect1.fLeft + 0.75f * fracRect1.width(),
                        fracRect1.fTop + 0.75f * fracRect1.height(),
                        fracRect1.fRight, fracRect1.fBottom};

    SkRect fracIntersect;
    SkAssertResult(fracIntersect.intersect(fracRect1, fracRect2));
    SkRect alignedIntersect;
    SkAssertResult(alignedIntersect.intersect(pixelAligned, fracRect1));

    // Both AA combine to one element
    run_test_case(r, TestCase::Build("aa", kDeviceBounds)
                              .actual().aa().intersect()
                                       .rect(fracRect1).rect(fracRect2)
                                       .finishElements()
                              .expect().aa().intersect().rect(fracIntersect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Both non-AA combine to one element
    run_test_case(r, TestCase::Build("nonaa", kDeviceBounds)
                              .actual().nonAA().intersect()
                                       .rect(fracRect1).rect(fracRect2)
                                       .finishElements()
                              .expect().nonAA().intersect().rect(fracIntersect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Pixel-aligned AA and non-AA combine
    run_test_case(r, TestCase::Build("aligned-aa+nonaa", kDeviceBounds)
                             .actual().intersect()
                                      .aa().rect(pixelAligned).nonAA().rect(fracRect1)
                                      .finishElements()
                             .expect().nonAA().intersect().rect(alignedIntersect).finishElements()
                             .state(GrClipStack::ClipState::kDeviceRect)
                             .finishTest());

    // AA and pixel-aligned non-AA combine
    run_test_case(r, TestCase::Build("aa+aligned-nonaa", kDeviceBounds)
                              .actual().intersect()
                                       .aa().rect(fracRect1).nonAA().rect(pixelAligned)
                                       .finishElements()
                              .expect().aa().intersect().rect(alignedIntersect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Other mixed AA modes do not combine
    run_test_case(r, TestCase::Build("aa+nonaa", kDeviceBounds)
                              .actual().intersect()
                                       .aa().rect(fracRect1).nonAA().rect(fracRect2)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that an intersection and a difference op do not combine, even if they would have if both
// were intersection ops.
DEF_TEST(GrClipStack_DifferenceNoCombine, r) {
    SkRect r1 = {15.f, 14.f, 23.22f, 58.2f};
    SkRect r2 = r1.makeOffset(5.f, 8.f);
    SkASSERT(r1.intersects(r2));

    run_test_case(r, TestCase::Build("no-combine", kDeviceBounds)
                              .actual().aa().intersect().rect(r1)
                                       .difference().rect(r2)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that intersection of rects in the same coordinate space can still be combined, but do not
// when the spaces differ.
DEF_TEST(GrClipStack_RectRectNonAxisAligned, r) {
    SkRect pixelAligned = {0, 0, 10, 10};
    SkRect fracRect1 = pixelAligned.makeOffset(5.3f, 3.7f);
    SkRect fracRect2 = {fracRect1.fLeft + 0.75f * fracRect1.width(),
                        fracRect1.fTop + 0.75f * fracRect1.height(),
                        fracRect1.fRight, fracRect1.fBottom};

    SkRect fracIntersect;
    SkAssertResult(fracIntersect.intersect(fracRect1, fracRect2));

    SkMatrix lm = SkMatrix::RotateDeg(45.f);

    // Both AA combine
    run_test_case(r, TestCase::Build("aa", kDeviceBounds)
                              .actual().aa().intersect().localToDevice(lm)
                                       .rect(fracRect1).rect(fracRect2)
                                       .finishElements()
                              .expect().aa().intersect().localToDevice(lm)
                                       .rect(fracIntersect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Both non-AA combine
    run_test_case(r, TestCase::Build("nonaa", kDeviceBounds)
                              .actual().nonAA().intersect().localToDevice(lm)
                                       .rect(fracRect1).rect(fracRect2)
                                       .finishElements()
                              .expect().nonAA().intersect().localToDevice(lm)
                                       .rect(fracIntersect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Integer-aligned coordinates under a local matrix with mixed AA don't combine, though
    run_test_case(r, TestCase::Build("local-aa", kDeviceBounds)
                              .actual().intersect().localToDevice(lm)
                                       .aa().rect(pixelAligned).nonAA().rect(fracRect1)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that intersection of two round rects can simplify to a single round rect when they have
// the same AA type.
DEF_TEST(GrClipStack_RRectRRectAACombine, r) {
    SkRRect r1 = SkRRect::MakeRectXY(SkRect::MakeWH(12, 12), 2.f, 2.f);
    SkRRect r2 = r1.makeOffset(6.f, 6.f);

    SkRRect intersect = SkRRectPriv::ConservativeIntersect(r1, r2);
    SkASSERT(!intersect.isEmpty());

    // Both AA combine
    run_test_case(r, TestCase::Build("aa", kDeviceBounds)
                              .actual().aa().intersect()
                                       .rrect(r1).rrect(r2)
                                       .finishElements()
                              .expect().aa().intersect().rrect(intersect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Both non-AA combine
    run_test_case(r, TestCase::Build("nonaa", kDeviceBounds)
                              .actual().nonAA().intersect()
                                       .rrect(r1).rrect(r2)
                                       .finishElements()
                              .expect().nonAA().intersect().rrect(intersect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Mixed do not combine
    run_test_case(r, TestCase::Build("aa+nonaa", kDeviceBounds)
                              .actual().intersect()
                                       .aa().rrect(r1).nonAA().rrect(r2)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Same AA state can combine in the same local coordinate space
    SkMatrix lm = SkMatrix::RotateDeg(45.f);
    run_test_case(r, TestCase::Build("local-aa", kDeviceBounds)
                              .actual().aa().intersect().localToDevice(lm)
                                       .rrect(r1).rrect(r2)
                                       .finishElements()
                              .expect().aa().intersect().localToDevice(lm)
                                       .rrect(intersect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("local-nonaa", kDeviceBounds)
                              .actual().nonAA().intersect().localToDevice(lm)
                                       .rrect(r1).rrect(r2)
                                       .finishElements()
                              .expect().nonAA().intersect().localToDevice(lm)
                                       .rrect(intersect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that intersection of a round rect and rect can simplify to a new round rect or even a rect.
DEF_TEST(GrClipStack_RectRRectCombine, r) {
    SkRRect rrect = SkRRect::MakeRectXY({0, 0, 10, 10}, 2.f, 2.f);
    SkRect cutTop = {-10, -10, 10, 4};
    SkRect cutMid = {-10, 3, 10, 7};

    // Rect + RRect becomes a round rect with some square corners
    SkVector cutCorners[4] = {{2.f, 2.f}, {2.f, 2.f}, {0, 0}, {0, 0}};
    SkRRect cutRRect;
    cutRRect.setRectRadii({0, 0, 10, 4}, cutCorners);
    run_test_case(r, TestCase::Build("still-rrect", kDeviceBounds)
                              .actual().intersect().aa().rrect(rrect).rect(cutTop).finishElements()
                              .expect().intersect().aa().rrect(cutRRect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Rect + RRect becomes a rect
    SkRect cutRect = {0, 3, 10, 7};
    run_test_case(r, TestCase::Build("to-rect", kDeviceBounds)
                               .actual().intersect().aa().rrect(rrect).rect(cutMid).finishElements()
                               .expect().intersect().aa().rect(cutRect).finishElements()
                               .state(GrClipStack::ClipState::kDeviceRect)
                               .finishTest());

    // But they can only combine when the intersecting shape is representable as a [r]rect.
    cutRect = {0, 0, 1.5f, 5.f};
    run_test_case(r, TestCase::Build("no-combine", kDeviceBounds)
                              .actual().intersect().aa().rrect(rrect).rect(cutRect).finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that a rect shape is actually pre-clipped to the device bounds
DEF_TEST(GrClipStack_RectDeviceClip, r) {
    SkRect crossesDeviceEdge = {20.f, kDeviceBounds.fTop - 13.2f,
                                kDeviceBounds.fRight + 15.5f, 30.f};
    SkRect insideDevice = {20.f, kDeviceBounds.fTop, kDeviceBounds.fRight, 30.f};

    run_test_case(r, TestCase::Build("device-aa-rect", kDeviceBounds)
                              .actual().intersect().aa().rect(crossesDeviceEdge).finishElements()
                              .expect().intersect().aa().rect(insideDevice).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    run_test_case(r, TestCase::Build("device-nonaa-rect", kDeviceBounds)
                              .actual().intersect().nonAA().rect(crossesDeviceEdge).finishElements()
                              .expect().intersect().nonAA().rect(insideDevice).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());
}

// Tests that other shapes' bounds are contained by the device bounds, even if their shape is not.
DEF_TEST(GrClipStack_ShapeDeviceBoundsClip, r) {
    SkRect crossesDeviceEdge = {20.f, kDeviceBounds.fTop - 13.2f,
                                kDeviceBounds.fRight + 15.5f, 30.f};

    // RRect
    run_test_case(r, TestCase::Build("device-rrect", kDeviceBounds)
                              .actual().intersect().aa()
                                       .rrect(SkRRect::MakeRectXY(crossesDeviceEdge, 4.f, 4.f))
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Path
    run_test_case(r, TestCase::Build("device-path", kDeviceBounds)
                              .actual().intersect().aa()
                                       .path(make_octagon(crossesDeviceEdge))
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that a simplifiable path turns into a simpler element type
DEF_TEST(GrClipStack_PathSimplify, r) {
    // Empty, point, and line paths -> empty
    SkPath empty;
    run_test_case(r, TestCase::Build("empty", kDeviceBounds)
                              .actual().path(empty).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    SkPath point;
    point.moveTo({0.f, 0.f});
    run_test_case(r, TestCase::Build("point", kDeviceBounds)
                              .actual().path(point).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    SkPath line;
    line.moveTo({0.f, 0.f});
    line.lineTo({10.f, 5.f});
    run_test_case(r, TestCase::Build("line", kDeviceBounds)
                              .actual().path(line).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // Rect path -> rect element
    SkRect rect = {0.f, 2.f, 10.f, 15.4f};
    SkPath rectPath;
    rectPath.addRect(rect);
    run_test_case(r, TestCase::Build("rect", kDeviceBounds)
                              .actual().path(rectPath).finishElements()
                              .expect().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Oval path -> rrect element
    SkPath ovalPath;
    ovalPath.addOval(rect);
    run_test_case(r, TestCase::Build("oval", kDeviceBounds)
                              .actual().path(ovalPath).finishElements()
                              .expect().rrect(SkRRect::MakeOval(rect)).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // RRect path -> rrect element
    SkRRect rrect = SkRRect::MakeRectXY(rect, 2.f, 2.f);
    SkPath rrectPath;
    rrectPath.addRRect(rrect);
    run_test_case(r, TestCase::Build("rrect", kDeviceBounds)
                              .actual().path(rrectPath).finishElements()
                              .expect().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());
}

// Tests that repeated identical clip operations are idempotent
DEF_TEST(GrClipStack_RepeatElement, r) {
    // Same rect
    SkRect rect = {5.3f, 62.f, 20.f, 85.f};
    run_test_case(r, TestCase::Build("same-rects", kDeviceBounds)
                              .actual().rect(rect).rect(rect).rect(rect).finishElements()
                              .expect().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());
    SkMatrix lm;
    lm.setRotate(30.f, rect.centerX(), rect.centerY());
    run_test_case(r, TestCase::Build("same-local-rects", kDeviceBounds)
                              .actual().localToDevice(lm).rect(rect).rect(rect).rect(rect)
                                       .finishElements()
                              .expect().localToDevice(lm).rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Same rrect
    SkRRect rrect = SkRRect::MakeRectXY(rect, 5.f, 2.5f);
    run_test_case(r, TestCase::Build("same-rrects", kDeviceBounds)
                              .actual().rrect(rrect).rrect(rrect).rrect(rrect).finishElements()
                              .expect().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());
    run_test_case(r, TestCase::Build("same-local-rrects", kDeviceBounds)
                              .actual().localToDevice(lm).rrect(rrect).rrect(rrect).rrect(rrect)
                                       .finishElements()
                              .expect().localToDevice(lm).rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Same convex path, by ==
    run_test_case(r, TestCase::Build("same-convex", kDeviceBounds)
                              .actual().path(make_octagon(rect)).path(make_octagon(rect))
                                       .finishElements()
                              .expect().path(make_octagon(rect)).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("same-local-convex", kDeviceBounds)
                              .actual().localToDevice(lm)
                                       .path(make_octagon(rect)).path(make_octagon(rect))
                                       .finishElements()
                              .expect().localToDevice(lm).path(make_octagon(rect))
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Same complicated path by gen-id but not ==
    SkPath path; // an hour glass
    path.moveTo({0.f, 0.f});
    path.lineTo({20.f, 20.f});
    path.lineTo({0.f, 20.f});
    path.lineTo({20.f, 0.f});
    path.close();

    run_test_case(r, TestCase::Build("same-path", kDeviceBounds)
                              .actual().path(path).path(path).path(path).finishElements()
                              .expect().path(path).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("same-local-path", kDeviceBounds)
                              .actual().localToDevice(lm)
                                       .path(path).path(path).path(path).finishElements()
                              .expect().localToDevice(lm).path(path)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that inverse-filled paths are canonicalized to a regular fill and a swapped clip op
DEF_TEST(GrClipStack_InverseFilledPath, r) {
    SkRect rect = {0.f, 0.f, 16.f, 17.f};
    SkPath rectPath;
    rectPath.addRect(rect);

    SkPath inverseRectPath = rectPath;
    inverseRectPath.toggleInverseFillType();

    SkPath complexPath = make_octagon(rect);
    SkPath inverseComplexPath = complexPath;
    inverseComplexPath.toggleInverseFillType();

    // Inverse filled rect + intersect -> diff rect
    run_test_case(r, TestCase::Build("inverse-rect-intersect", kDeviceBounds)
                              .actual().aa().intersect().path(inverseRectPath).finishElements()
                              .expect().aa().difference().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Inverse filled rect + difference -> int. rect
    run_test_case(r, TestCase::Build("inverse-rect-difference", kDeviceBounds)
                              .actual().aa().difference().path(inverseRectPath).finishElements()
                              .expect().aa().intersect().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Inverse filled path + intersect -> diff path
    run_test_case(r, TestCase::Build("inverse-path-intersect", kDeviceBounds)
                              .actual().aa().intersect().path(inverseComplexPath).finishElements()
                              .expect().aa().difference().path(complexPath).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Inverse filled path + difference -> int. path
    run_test_case(r, TestCase::Build("inverse-path-difference", kDeviceBounds)
                              .actual().aa().difference().path(inverseComplexPath).finishElements()
                              .expect().aa().intersect().path(complexPath).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that clip operations that are offscreen either make the clip empty or stay wide open
DEF_TEST(GrClipStack_Offscreen, r) {
    SkRect offscreenRect = {kDeviceBounds.fRight + 10.f, kDeviceBounds.fTop + 20.f,
                            kDeviceBounds.fRight + 40.f, kDeviceBounds.fTop + 60.f};
    SkASSERT(!offscreenRect.intersects(SkRect::Make(kDeviceBounds)));

    SkRRect offscreenRRect = SkRRect::MakeRectXY(offscreenRect, 5.f, 5.f);
    SkPath offscreenPath = make_octagon(offscreenRect);

    // Intersect -> empty
    run_test_case(r, TestCase::Build("intersect-combo", kDeviceBounds)
                              .actual().aa().intersect()
                                       .rect(offscreenRect)
                                       .rrect(offscreenRRect)
                                       .path(offscreenPath)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("intersect-rect", kDeviceBounds)
                              .actual().aa().intersect()
                                       .rect(offscreenRect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("intersect-rrect", kDeviceBounds)
                              .actual().aa().intersect()
                                       .rrect(offscreenRRect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("intersect-path", kDeviceBounds)
                              .actual().aa().intersect()
                                       .path(offscreenPath)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // Difference -> wide open
    run_test_case(r, TestCase::Build("difference-combo", kDeviceBounds)
                              .actual().aa().difference()
                                       .rect(offscreenRect)
                                       .rrect(offscreenRRect)
                                       .path(offscreenPath)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
    run_test_case(r, TestCase::Build("difference-rect", kDeviceBounds)
                              .actual().aa().difference()
                                       .rect(offscreenRect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
    run_test_case(r, TestCase::Build("difference-rrect", kDeviceBounds)
                              .actual().aa().difference()
                                       .rrect(offscreenRRect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
    run_test_case(r, TestCase::Build("difference-path", kDeviceBounds)
                              .actual().aa().difference()
                                       .path(offscreenPath)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
}

// Tests that an empty shape updates the clip state directly without needing an element
DEF_TEST(GrClipStack_EmptyShape, r) {
    // Intersect -> empty
    run_test_case(r, TestCase::Build("empty-intersect", kDeviceBounds)
                              .actual().intersect().rect(SkRect::MakeEmpty()).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // Difference -> no-op
    run_test_case(r, TestCase::Build("empty-difference", kDeviceBounds)
                              .actual().difference().rect(SkRect::MakeEmpty()).finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());

    SkRRect rrect = SkRRect::MakeRectXY({4.f, 10.f, 16.f, 32.f}, 2.f, 2.f);
    run_test_case(r, TestCase::Build("noop-difference", kDeviceBounds)
                              .actual().difference().rrect(rrect).rect(SkRect::MakeEmpty())
                                       .finishElements()
                              .expect().difference().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that sufficiently large difference operations can shrink the conservative bounds
DEF_TEST(GrClipStack_DifferenceBounds, r) {
    SkRect rightSide = {50.f, -10.f, 2.f * kDeviceBounds.fRight, kDeviceBounds.fBottom + 10.f};
    SkRect clipped = rightSide;
    SkAssertResult(clipped.intersect(SkRect::Make(kDeviceBounds)));

    run_test_case(r, TestCase::Build("difference-cut", kDeviceBounds)
                              .actual().nonAA().difference().rect(rightSide).finishElements()
                              .expect().nonAA().difference().rect(clipped).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that intersections can combine even if there's a difference operation in the middle
DEF_TEST(GrClipStack_NoDifferenceInterference, r) {
    SkRect intR1 = {0.f, 0.f, 30.f, 30.f};
    SkRect intR2 = {15.f, 15.f, 45.f, 45.f};
    SkRect intCombo = {15.f, 15.f, 30.f, 30.f};
    SkRect diff = {20.f, 6.f, 50.f, 50.f};

    run_test_case(r, TestCase::Build("cross-diff-combine", kDeviceBounds)
                              .actual().rect(intR1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(diff, GrAA::kYes, SkClipOp::kDifference)
                                       .rect(intR2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(intCombo, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(diff, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that multiple path operations are all recorded, but not otherwise consolidated
DEF_TEST(GrClipStack_MultiplePaths, r) {
    // Chosen to be greater than the number of inline-allocated elements and save records of the
    // GrClipStack so that we test heap allocation as well.
    static constexpr int kNumOps = 16;

    auto b = TestCase::Build("many-paths-difference", kDeviceBounds);
    SkRect d = {0.f, 0.f, 12.f, 12.f};
    for (int i = 0; i < kNumOps; ++i) {
        b.actual().path(make_octagon(d), GrAA::kNo, SkClipOp::kDifference);

        d.offset(15.f, 0.f);
        if (d.fRight > kDeviceBounds.fRight) {
            d.fLeft = 0.f;
            d.fRight = 12.f;
            d.offset(0.f, 15.f);
        }
    }

    run_test_case(r, b.expectActual()
                      .state(GrClipStack::ClipState::kComplex)
                      .finishTest());

    b = TestCase::Build("many-paths-intersect", kDeviceBounds);
    d = {0.f, 0.f, 12.f, 12.f};
    for (int i = 0; i < kNumOps; ++i) {
        b.actual().path(make_octagon(d), GrAA::kYes, SkClipOp::kIntersect);
        d.offset(0.01f, 0.01f);
    }

    run_test_case(r, b.expectActual()
                      .state(GrClipStack::ClipState::kComplex)
                      .finishTest());
}

// Tests that a single rect is treated as kDeviceRect state when it's axis-aligned and intersect.
DEF_TEST(GrClipStack_DeviceRect, r) {
    // Axis-aligned + intersect -> kDeviceRect
    SkRect rect = {0, 0, 20, 20};
    run_test_case(r, TestCase::Build("device-rect", kDeviceBounds)
                              .actual().intersect().aa().rect(rect).finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // Not axis-aligned -> kComplex
    SkMatrix lm = SkMatrix::RotateDeg(15.f);
    run_test_case(r, TestCase::Build("unaligned-rect", kDeviceBounds)
                              .actual().localToDevice(lm).intersect().aa().rect(rect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Not intersect -> kComplex
    run_test_case(r, TestCase::Build("diff-rect", kDeviceBounds)
                              .actual().difference().aa().rect(rect).finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that a single rrect is treated as kDeviceRRect state when it's axis-aligned and intersect.
DEF_TEST(GrClipStack_DeviceRRect, r) {
    // Axis-aligned + intersect -> kDeviceRRect
    SkRect rect = {0, 0, 20, 20};
    SkRRect rrect = SkRRect::MakeRectXY(rect, 5.f, 5.f);
    run_test_case(r, TestCase::Build("device-rrect", kDeviceBounds)
                              .actual().intersect().aa().rrect(rrect).finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Not axis-aligned -> kComplex
    SkMatrix lm = SkMatrix::RotateDeg(15.f);
    run_test_case(r, TestCase::Build("unaligned-rrect", kDeviceBounds)
                              .actual().localToDevice(lm).intersect().aa().rrect(rrect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Not intersect -> kComplex
    run_test_case(r, TestCase::Build("diff-rrect", kDeviceBounds)
                              .actual().difference().aa().rrect(rrect).finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that scale+translate matrices are pre-applied to rects and rrects, which also then allows
// elements with different scale+translate matrices to be consolidated as if they were in the same
// coordinate space.
DEF_TEST(GrClipStack_ScaleTranslate, r) {
    SkMatrix lm = SkMatrix::Scale(2.f, 4.f);
    lm.postTranslate(15.5f, 14.3f);
    SkASSERT(lm.preservesAxisAlignment() && lm.isScaleTranslate());

    // Rect -> matrix is applied up front
    SkRect rect = {0.f, 0.f, 10.f, 10.f};
    run_test_case(r, TestCase::Build("st+rect", kDeviceBounds)
                              .actual().rect(rect, lm, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(lm.mapRect(rect), GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // RRect -> matrix is applied up front
    SkRRect localRRect = SkRRect::MakeRectXY(rect, 2.f, 2.f);
    SkRRect deviceRRect;
    SkAssertResult(localRRect.transform(lm, &deviceRRect));
    run_test_case(r, TestCase::Build("st+rrect", kDeviceBounds)
                              .actual().rrect(localRRect, lm, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rrect(deviceRRect, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Path -> matrix is NOT applied
    run_test_case(r, TestCase::Build("st+path", kDeviceBounds)
                              .actual().intersect().localToDevice(lm).path(make_octagon(rect))
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that rect-stays-rect matrices that are not scale+translate matrices are pre-applied.
DEF_TEST(GrClipStack_PreserveAxisAlignment, r) {
    SkMatrix lm = SkMatrix::RotateDeg(90.f);
    lm.postTranslate(15.5f, 14.3f);
    SkASSERT(lm.preservesAxisAlignment() && !lm.isScaleTranslate());

    // Rect -> matrix is applied up front
    SkRect rect = {0.f, 0.f, 10.f, 10.f};
    run_test_case(r, TestCase::Build("r90+rect", kDeviceBounds)
                              .actual().rect(rect, lm, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(lm.mapRect(rect), GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // RRect -> matrix is applied up front
    SkRRect localRRect = SkRRect::MakeRectXY(rect, 2.f, 2.f);
    SkRRect deviceRRect;
    SkAssertResult(localRRect.transform(lm, &deviceRRect));
    run_test_case(r, TestCase::Build("r90+rrect", kDeviceBounds)
                              .actual().rrect(localRRect, lm, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rrect(deviceRRect, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Path -> matrix is NOT applied
    run_test_case(r, TestCase::Build("r90+path", kDeviceBounds)
                              .actual().intersect().localToDevice(lm).path(make_octagon(rect))
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that a convex path element can contain a rect or round rect, allowing the stack to be
// simplified
DEF_TEST(GrClipStack_ConvexPathContains, r) {
    SkRect rect = {15.f, 15.f, 30.f, 30.f};
    SkRRect rrect = SkRRect::MakeRectXY(rect, 5.f, 5.f);
    SkPath bigPath = make_octagon(rect.makeOutset(10.f, 10.f), 5.f, 5.f);

    // Intersect -> path element isn't kept
    run_test_case(r, TestCase::Build("convex+rect-intersect", kDeviceBounds)
                              .actual().aa().intersect().rect(rect).path(bigPath).finishElements()
                              .expect().aa().intersect().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex+rrect-intersect", kDeviceBounds)
                              .actual().aa().intersect().rrect(rrect).path(bigPath).finishElements()
                              .expect().aa().intersect().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // Difference -> path element is the only one left
    run_test_case(r, TestCase::Build("convex+rect-difference", kDeviceBounds)
                              .actual().aa().difference().rect(rect).path(bigPath).finishElements()
                              .expect().aa().difference().path(bigPath).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex+rrect-difference", kDeviceBounds)
                              .actual().aa().difference().rrect(rrect).path(bigPath)
                                       .finishElements()
                              .expect().aa().difference().path(bigPath).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Intersect small shape + difference big path -> empty
    run_test_case(r, TestCase::Build("convex-diff+rect-int", kDeviceBounds)
                              .actual().aa().intersect().rect(rect)
                                       .difference().path(bigPath).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex-diff+rrect-int", kDeviceBounds)
                              .actual().aa().intersect().rrect(rrect)
                                       .difference().path(bigPath).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // Diff small shape + intersect big path -> both
    run_test_case(r, TestCase::Build("convex-int+rect-diff", kDeviceBounds)
                              .actual().aa().intersect().path(bigPath).difference().rect(rect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex-int+rrect-diff", kDeviceBounds)
                              .actual().aa().intersect().path(bigPath).difference().rrect(rrect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that rects/rrects in different coordinate spaces can be consolidated when one is fully
// contained by the other.
DEF_TEST(GrClipStack_NonAxisAlignedContains, r) {
    SkMatrix lm1 = SkMatrix::RotateDeg(45.f);
    SkRect bigR = {-20.f, -20.f, 20.f, 20.f};
    SkRRect bigRR = SkRRect::MakeRectXY(bigR, 1.f, 1.f);

    SkMatrix lm2 = SkMatrix::RotateDeg(-45.f);
    SkRect smR = {-10.f, -10.f, 10.f, 10.f};
    SkRRect smRR = SkRRect::MakeRectXY(smR, 1.f, 1.f);

    // I+I should select the smaller 2nd shape (r2 or rr2)
    run_test_case(r, TestCase::Build("rect-rect-ii", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-rrect-ii", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rect-rrect-ii", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-rect-ii", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // D+D should select the larger shape (r1 or rr1)
    run_test_case(r, TestCase::Build("rect-rect-dd", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expect().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-rrect-dd", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expect().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rect-rrect-dd", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expect().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                         .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-rect-dd", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expect().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // D(1)+I(2) should result in empty
    run_test_case(r, TestCase::Build("rectD-rectI", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrectD-rrectI", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("rectD-rrectI", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrectD-rectI", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kDifference)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // I(1)+D(2) should result in both shapes
    run_test_case(r, TestCase::Build("rectI+rectD", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrectI+rrectD", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrectI+rectD", kDeviceBounds)
                              .actual().rrect(bigRR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(smR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("rectI+rrectD", kDeviceBounds)
                              .actual().rect(bigR, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rrect(smRR, lm2, GrAA::kYes, SkClipOp::kDifference)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that shapes with mixed AA state that contain each other can still be consolidated,
// unless they are too close to the edge and non-AA snapping can't be predicted
DEF_TEST(GrClipStack_MixedAAContains, r) {
    SkMatrix lm1 = SkMatrix::RotateDeg(45.f);
    SkRect r1 = {-20.f, -20.f, 20.f, 20.f};

    SkMatrix lm2 = SkMatrix::RotateDeg(-45.f);
    SkRect r2Safe = {-10.f, -10.f, 10.f, 10.f};
    SkRect r2Unsafe = {-19.5f, -19.5f, 19.5f, 19.5f};

    // Non-AA sufficiently inside AA element can discard the outer AA element
    run_test_case(r, TestCase::Build("mixed-outeraa-combine", kDeviceBounds)
                              .actual().rect(r1, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(r2Safe, lm2, GrAA::kNo, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(r2Safe, lm2, GrAA::kNo, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    // Vice versa
    run_test_case(r, TestCase::Build("mixed-inneraa-combine", kDeviceBounds)
                              .actual().rect(r1, lm1, GrAA::kNo, SkClipOp::kIntersect)
                                       .rect(r2Safe, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expect().rect(r2Safe, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // Non-AA too close to AA edges keeps both
    run_test_case(r, TestCase::Build("mixed-outeraa-nocombine", kDeviceBounds)
                              .actual().rect(r1, lm1, GrAA::kYes, SkClipOp::kIntersect)
                                       .rect(r2Unsafe, lm2, GrAA::kNo, SkClipOp::kIntersect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
    run_test_case(r, TestCase::Build("mixed-inneraa-nocombine", kDeviceBounds)
                              .actual().rect(r1, lm1, GrAA::kNo, SkClipOp::kIntersect)
                                       .rect(r2Unsafe, lm2, GrAA::kYes, SkClipOp::kIntersect)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

// Tests that a shape that contains the device bounds updates the clip state directly
DEF_TEST(GrClipStack_ShapeContainsDevice, r) {
    SkRect rect = SkRect::Make(kDeviceBounds).makeOutset(10.f, 10.f);
    SkRRect rrect = SkRRect::MakeRectXY(rect, 10.f, 10.f);
    SkPath convex = make_octagon(rect, 10.f, 10.f);

    // Intersect -> no-op
    run_test_case(r, TestCase::Build("rect-intersect", kDeviceBounds)
                              .actual().intersect().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-intersect", kDeviceBounds)
                              .actual().intersect().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex-intersect", kDeviceBounds)
                              .actual().intersect().path(convex).finishElements()
                              .state(GrClipStack::ClipState::kWideOpen)
                              .finishTest());

    // Difference -> empty
    run_test_case(r, TestCase::Build("rect-difference", kDeviceBounds)
                              .actual().difference().rect(rect).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("rrect-difference", kDeviceBounds)
                              .actual().difference().rrect(rrect).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
    run_test_case(r, TestCase::Build("convex-difference", kDeviceBounds)
                              .actual().difference().path(convex).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());
}

// Tests that shapes that do not overlap make for an empty clip (when intersecting), pick just the
// intersecting op (when mixed), or are all kept (when diff'ing).
DEF_TEST(GrClipStack_DisjointShapes, r) {
    SkRect rt = {10.f, 10.f, 20.f, 20.f};
    SkRRect rr = SkRRect::MakeOval(rt.makeOffset({20.f, 0.f}));
    SkPath p = make_octagon(rt.makeOffset({0.f, 20.f}));

    // I+I
    run_test_case(r, TestCase::Build("iii", kDeviceBounds)
                              .actual().aa().intersect().rect(rt).rrect(rr).path(p).finishElements()
                              .state(GrClipStack::ClipState::kEmpty)
                              .finishTest());

    // D+D
    run_test_case(r, TestCase::Build("ddd", kDeviceBounds)
                              .actual().nonAA().difference().rect(rt).rrect(rr).path(p)
                                       .finishElements()
                              .expectActual()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());

    // I+D from rect
    run_test_case(r, TestCase::Build("idd", kDeviceBounds)
                              .actual().aa().intersect().rect(rt)
                                       .nonAA().difference().rrect(rr).path(p)
                                       .finishElements()
                              .expect().aa().intersect().rect(rt).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRect)
                              .finishTest());

    // I+D from rrect
    run_test_case(r, TestCase::Build("did", kDeviceBounds)
                              .actual().aa().intersect().rrect(rr)
                                       .nonAA().difference().rect(rt).path(p)
                                       .finishElements()
                              .expect().aa().intersect().rrect(rr).finishElements()
                              .state(GrClipStack::ClipState::kDeviceRRect)
                              .finishTest());

    // I+D from path
    run_test_case(r, TestCase::Build("ddi", kDeviceBounds)
                              .actual().aa().intersect().path(p)
                                       .nonAA().difference().rect(rt).rrect(rr)
                                       .finishElements()
                              .expect().aa().intersect().path(p).finishElements()
                              .state(GrClipStack::ClipState::kComplex)
                              .finishTest());
}

DEF_TEST(GrClipStack_ComplexClip, r) {
    static constexpr float kN = 10.f;
    static constexpr float kR = kN / 3.f;

    // 4 rectangles that overlap by kN x 2kN (horiz), 2kN x kN (vert), or kN x kN (diagonal)
    static const SkRect kTL = {0.f, 0.f, 2.f * kN, 2.f * kN};
    static const SkRect kTR = {kN,  0.f, 3.f * kN, 2.f * kN};
    static const SkRect kBL = {0.f, kN,  2.f * kN, 3.f * kN};
    static const SkRect kBR = {kN,  kN,  3.f * kN, 3.f * kN};

    enum ShapeType { kRect, kRRect, kConvex };

    SkRect rects[] = { kTL, kTR, kBL, kBR };
    for (ShapeType type : { kRect, kRRect, kConvex }) {
        for (int opBits = 6; opBits < 16; ++opBits) {
            SkString name;
            name.appendf("complex-%d-%d", (int) type, opBits);

            SkRect expectedRectIntersection = SkRect::Make(kDeviceBounds);
            SkRRect expectedRRectIntersection = SkRRect::MakeRect(expectedRectIntersection);

            auto b = TestCase::Build(name.c_str(), kDeviceBounds);
            for (int i = 0; i < 4; ++i) {
                SkClipOp op = (opBits & (1 << i)) ? SkClipOp::kIntersect : SkClipOp::kDifference;
                switch(type) {
                    case kRect: {
                        SkRect r = rects[i];
                        if (op == SkClipOp::kDifference) {
                            // Shrink the rect for difference ops, otherwise in the rect testcase
                            // any difference op would remove the intersection of the other ops
                            // given how the rects are defined, and that's just not interesting.
                            r.inset(kR, kR);
                        }
                        b.actual().rect(r, GrAA::kYes, op);
                        if (op == SkClipOp::kIntersect) {
                            SkAssertResult(expectedRectIntersection.intersect(r));
                        } else {
                            b.expect().rect(r, GrAA::kYes, SkClipOp::kDifference);
                        }
                        break; }
                    case kRRect: {
                        SkRRect rrect = SkRRect::MakeRectXY(rects[i], kR, kR);
                        b.actual().rrect(rrect, GrAA::kYes, op);
                        if (op == SkClipOp::kIntersect) {
                            expectedRRectIntersection = SkRRectPriv::ConservativeIntersect(
                                    expectedRRectIntersection, rrect);
                            SkASSERT(!expectedRRectIntersection.isEmpty());
                        } else {
                            b.expect().rrect(rrect, GrAA::kYes, SkClipOp::kDifference);
                        }
                        break; }
                    case kConvex:
                        b.actual().path(make_octagon(rects[i], kR, kR), GrAA::kYes, op);
                        // NOTE: We don't set any expectations here, since convex just calls
                        // expectActual() at the end.
                        break;
                }
            }

            // The expectations differ depending on the shape type
            GrClipStack::ClipState state = GrClipStack::ClipState::kComplex;
            if (type == kConvex) {
                // The simplest case is when the paths cannot be combined together, so we expect
                // the actual elements to be unmodified (both intersect and difference).
                b.expectActual();
            } else if (opBits) {
                // All intersection ops were pre-computed into expectedR[R]ectIntersection
                // - difference ops already added in the for loop
                if (type == kRect) {
                    SkASSERT(expectedRectIntersection != SkRect::Make(kDeviceBounds) &&
                             !expectedRectIntersection.isEmpty());
                    b.expect().rect(expectedRectIntersection, GrAA::kYes, SkClipOp::kIntersect);
                    if (opBits == 0xf) {
                        state = GrClipStack::ClipState::kDeviceRect;
                    }
                } else {
                    SkASSERT(expectedRRectIntersection !=
                                    SkRRect::MakeRect(SkRect::Make(kDeviceBounds)) &&
                             !expectedRRectIntersection.isEmpty());
                    b.expect().rrect(expectedRRectIntersection, GrAA::kYes, SkClipOp::kIntersect);
                    if (opBits == 0xf) {
                        state = GrClipStack::ClipState::kDeviceRRect;
                    }
                }
            }

            run_test_case(r, b.state(state).finishTest());
        }
    }
}

// ///////////////////////////////////////////////////////////////////////////////
// // These tests do not use the TestCase infrastructure and manipulate a
// // GrClipStack directly.

// Tests that replaceClip() works as expected across save/restores
DEF_TEST(GrClipStack_ReplaceClip, r) {
    GrClipStack cs(kDeviceBounds, nullptr, false);

    SkRRect rrect = SkRRect::MakeRectXY({15.f, 12.25f, 40.3f, 23.5f}, 4.f, 6.f);
    cs.clipRRect(SkMatrix::I(), rrect, GrAA::kYes, SkClipOp::kIntersect);

    SkIRect replace = {50, 25, 75, 40}; // Is disjoint from the rrect element
    cs.save();
    cs.replaceClip(replace);

    REPORTER_ASSERT(r, cs.clipState() == GrClipStack::ClipState::kDeviceRect,
                    "Clip did not become a device rect");
    REPORTER_ASSERT(r, cs.getConservativeBounds() == replace, "Unexpected replaced clip bounds");
    const GrClipStack::Element& replaceElement = *cs.begin();
    REPORTER_ASSERT(r, replaceElement.fShape.rect() == SkRect::Make(replace) &&
                       replaceElement.fAA == GrAA::kNo &&
                       replaceElement.fOp == SkClipOp::kIntersect &&
                       replaceElement.fLocalToDevice == SkMatrix::I(),
                    "Unexpected replace element state");

    // Restore should undo the replaced clip and bring back the rrect
    cs.restore();
    REPORTER_ASSERT(r, cs.clipState() == GrClipStack::ClipState::kDeviceRRect,
                    "Unexpected state after restore, not kDeviceRRect");
    const GrClipStack::Element& rrectElem = *cs.begin();
    REPORTER_ASSERT(r, rrectElem.fShape.rrect() == rrect &&
                       rrectElem.fAA == GrAA::kYes &&
                       rrectElem.fOp == SkClipOp::kIntersect &&
                       rrectElem.fLocalToDevice == SkMatrix::I(),
                    "RRect element state not restored properly after replace clip undone");
}

// Try to overflow the number of allowed window rects (see skbug.com/10989)
DEF_TEST(GrClipStack_DiffRects, r) {
    GrMockOptions options;
    options.fMaxWindowRectangles = 8;

    SkSimpleMatrixProvider matrixProvider = SkMatrix::I();
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&options);
    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc = skgpu::v1::SurfaceDrawContext::Make(
            context.get(), GrColorType::kRGBA_8888, SkColorSpace::MakeSRGB(),
            SkBackingFit::kExact, kDeviceBounds.size(), SkSurfaceProps());

    GrClipStack cs(kDeviceBounds, &matrixProvider, false);

    cs.save();
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            cs.clipRect(SkMatrix::I(), SkRect::MakeXYWH(10*x+1, 10*y+1, 8, 8),
                        GrAA::kNo, SkClipOp::kDifference);
        }
    }

    GrAppliedClip out(kDeviceBounds.size());
    SkRect drawBounds = SkRect::Make(kDeviceBounds);
    GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                     &out, &drawBounds);

    REPORTER_ASSERT(r, effect == GrClip::Effect::kClipped);
    REPORTER_ASSERT(r, out.windowRectsState().numWindows() == 8);

    cs.restore();
}

// Tests that when a stack is forced to always be AA, non-AA elements become AA
DEF_TEST(GrClipStack_ForceAA, r) {
    GrClipStack cs(kDeviceBounds, nullptr, true);

    // AA will remain AA
    SkRect aaRect = {0.25f, 12.43f, 25.2f, 23.f};
    cs.clipRect(SkMatrix::I(), aaRect, GrAA::kYes, SkClipOp::kIntersect);

    // Non-AA will become AA
    SkPath nonAAPath = make_octagon({2.f, 10.f, 16.f, 20.f});
    cs.clipPath(SkMatrix::I(), nonAAPath, GrAA::kNo, SkClipOp::kIntersect);

    // Non-AA rects remain non-AA so they can be applied as a scissor
    SkRect nonAARect = {4.5f, 5.f, 17.25f, 18.23f};
    cs.clipRect(SkMatrix::I(), nonAARect, GrAA::kNo, SkClipOp::kIntersect);

    // The stack reports elements newest first, but the non-AA rect op was combined in place with
    // the first aa rect, so we should see nonAAPath as AA, and then the intersection of rects.
    auto elements = cs.begin();

    const GrClipStack::Element& nonAARectElement = *elements;
    REPORTER_ASSERT(r, nonAARectElement.fShape.isRect(), "Expected rect element");
    REPORTER_ASSERT(r, nonAARectElement.fAA == GrAA::kNo,
                    "Axis-aligned non-AA rect ignores forceAA");
    REPORTER_ASSERT(r, nonAARectElement.fShape.rect() == nonAARect,
                    "Mixed AA rects should not combine");

    ++elements;
    const GrClipStack::Element& aaPathElement = *elements;
    REPORTER_ASSERT(r, aaPathElement.fShape.isPath(), "Expected path element");
    REPORTER_ASSERT(r, aaPathElement.fShape.path() == nonAAPath, "Wrong path element");
    REPORTER_ASSERT(r, aaPathElement.fAA == GrAA::kYes, "Path element not promoted to AA");

    ++elements;
    const GrClipStack::Element& aaRectElement = *elements;
    REPORTER_ASSERT(r, aaRectElement.fShape.isRect(), "Expected rect element");
    REPORTER_ASSERT(r, aaRectElement.fShape.rect() == aaRect,
                    "Mixed AA rects should not combine");
    REPORTER_ASSERT(r, aaRectElement.fAA == GrAA::kYes, "Rect element stays AA");

    ++elements;
    REPORTER_ASSERT(r, !(elements != cs.end()), "Expected only three clip elements");
}

// Tests preApply works as expected for device rects, rrects, and reports clipped-out, etc. as
// expected.
DEF_TEST(GrClipStack_PreApply, r) {
    GrClipStack cs(kDeviceBounds, nullptr, false);

    // Offscreen is kClippedOut
    GrClip::PreClipResult result = cs.preApply({-10.f, -10.f, -1.f, -1.f}, GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClippedOut,
                    "Offscreen draw is kClippedOut");

    // Intersecting screen with wide-open clip is kUnclipped
    result = cs.preApply({-10.f, -10.f, 10.f, 10.f}, GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kUnclipped,
                    "Wide open screen intersection is still kUnclipped");

    // Empty clip is clipped out
    cs.save();
    cs.clipRect(SkMatrix::I(), SkRect::MakeEmpty(), GrAA::kNo, SkClipOp::kIntersect);
    result = cs.preApply({0.f, 0.f, 20.f, 20.f}, GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClippedOut,
                    "Empty clip stack preApplies as kClippedOut");
    cs.restore();

    // Contained inside clip is kUnclipped (using rrect for the outer clip element since paths
    // don't support an inner bounds and anything complex is otherwise skipped in preApply).
    SkRect rect = {10.f, 10.f, 40.f, 40.f};
    SkRRect bigRRect = SkRRect::MakeRectXY(rect.makeOutset(5.f, 5.f), 5.f, 5.f);
    cs.save();
    cs.clipRRect(SkMatrix::I(), bigRRect, GrAA::kYes, SkClipOp::kIntersect);
    result = cs.preApply(rect, GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kUnclipped,
                    "Draw contained within clip is kUnclipped");

    // Disjoint from clip (but still on screen) is kClippedOut
    result = cs.preApply({50.f, 50.f, 60.f, 60.f}, GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClippedOut,
                    "Draw not intersecting clip is kClippedOut");
    cs.restore();

    // Intersecting clip is kClipped for complex shape
    cs.save();
    SkPath path = make_octagon(rect.makeOutset(5.f, 5.f), 5.f, 5.f);
    cs.clipPath(SkMatrix::I(), path, GrAA::kYes, SkClipOp::kIntersect);
    result = cs.preApply(path.getBounds(), GrAA::kNo);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClipped && !result.fIsRRect,
                    "Draw with complex clip is kClipped, but is not an rrect");
    cs.restore();

    // Intersecting clip is kDeviceRect for axis-aligned rect clip
    cs.save();
    cs.clipRect(SkMatrix::I(), rect, GrAA::kYes, SkClipOp::kIntersect);
    result = cs.preApply(rect.makeOffset(2.f, 2.f), GrAA::kNo);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClipped &&
                       result.fAA == GrAA::kYes &&
                       result.fIsRRect &&
                       result.fRRect == SkRRect::MakeRect(rect),
                    "kDeviceRect clip stack should be reported by preApply");
    cs.restore();

    // Intersecting clip is kDeviceRRect for axis-aligned rrect clip
    cs.save();
    SkRRect clipRRect = SkRRect::MakeRectXY(rect, 5.f, 5.f);
    cs.clipRRect(SkMatrix::I(), clipRRect, GrAA::kYes, SkClipOp::kIntersect);
    result = cs.preApply(rect.makeOffset(2.f, 2.f), GrAA::kNo);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClipped &&
                       result.fAA == GrAA::kYes &&
                       result.fIsRRect &&
                       result.fRRect == clipRRect,
                    "kDeviceRRect clip stack should be reported by preApply");
    cs.restore();
}

// Tests the clip shader entry point
DEF_TEST(GrClipStack_Shader, r) {
    sk_sp<SkShader> shader = SkShaders::Color({0.f, 0.f, 0.f, 0.5f}, nullptr);

    SkSimpleMatrixProvider matrixProvider = SkMatrix::I();
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(nullptr);
    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc = skgpu::v1::SurfaceDrawContext::Make(
            context.get(), GrColorType::kRGBA_8888, SkColorSpace::MakeSRGB(),
            SkBackingFit::kExact, kDeviceBounds.size(), SkSurfaceProps());

    GrClipStack cs(kDeviceBounds, &matrixProvider, false);
    cs.save();
    cs.clipShader(shader);

    REPORTER_ASSERT(r, cs.clipState() == GrClipStack::ClipState::kComplex,
                    "A clip shader should be reported as a complex clip");

    GrAppliedClip out(kDeviceBounds.size());
    SkRect drawBounds = {10.f, 11.f, 16.f, 32.f};
    GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                     &out, &drawBounds);

    REPORTER_ASSERT(r, effect == GrClip::Effect::kClipped,
                    "apply() should return kClipped for a clip shader");
    REPORTER_ASSERT(r, out.hasCoverageFragmentProcessor(),
                    "apply() should have converted clip shader to a coverage FP");

    GrAppliedClip out2(kDeviceBounds.size());
    drawBounds = {-15.f, -10.f, -1.f, 10.f}; // offscreen
    effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage, &out2,
                      &drawBounds);
    REPORTER_ASSERT(r, effect == GrClip::Effect::kClippedOut,
                    "apply() should still discard offscreen draws with a clip shader");

    cs.restore();
    REPORTER_ASSERT(r, cs.clipState() == GrClipStack::ClipState::kWideOpen,
                    "restore() should get rid of the clip shader");


    // Adding a clip shader on top of a device rect clip should prevent preApply from reporting
    // it as a device rect
    cs.clipRect(SkMatrix::I(), {10, 15, 30, 30}, GrAA::kNo, SkClipOp::kIntersect);
    SkASSERT(cs.clipState() == GrClipStack::ClipState::kDeviceRect); // test precondition
    cs.clipShader(shader);
    GrClip::PreClipResult result = cs.preApply(SkRect::Make(kDeviceBounds), GrAA::kYes);
    REPORTER_ASSERT(r, result.fEffect == GrClip::Effect::kClipped && !result.fIsRRect,
                    "A clip shader should not produce a device rect from preApply");
}

// Tests apply() under simple circumstances, that don't require actual rendering of masks, or
// atlases. This lets us define the test regularly instead of a GPU-only test.
// - This is not exhaustive and is challenging to unit test, so apply() is predominantly tested by
//   the GMs instead.
DEF_TEST(GrClipStack_SimpleApply, r) {
    SkSimpleMatrixProvider matrixProvider = SkMatrix::I();
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(nullptr);
    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc = skgpu::v1::SurfaceDrawContext::Make(
            context.get(), GrColorType::kRGBA_8888, SkColorSpace::MakeSRGB(),
            SkBackingFit::kExact, kDeviceBounds.size(), SkSurfaceProps());

    GrClipStack cs(kDeviceBounds, &matrixProvider, false);

    // Offscreen draw is kClippedOut
    {
        SkRect drawBounds = {-15.f, -15.f, -1.f, -1.f};

        GrAppliedClip out(kDeviceBounds.size());
        GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                         &out, &drawBounds);
        REPORTER_ASSERT(r, effect == GrClip::Effect::kClippedOut, "Offscreen draw is clipped out");
    }

    // Draw contained in clip is kUnclipped
    {
        SkRect drawBounds = {15.4f, 16.3f, 26.f, 32.f};
        cs.save();
        cs.clipPath(SkMatrix::I(), make_octagon(drawBounds.makeOutset(5.f, 5.f), 5.f, 5.f),
                    GrAA::kYes, SkClipOp::kIntersect);

        GrAppliedClip out(kDeviceBounds.size());
        GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                         &out, &drawBounds);
        REPORTER_ASSERT(r, effect == GrClip::Effect::kUnclipped, "Draw inside clip is unclipped");
        cs.restore();
    }

    // Draw bounds are cropped to device space before checking contains
    {
        SkRect clipRect = {kDeviceBounds.fRight - 20.f, 10.f, kDeviceBounds.fRight, 20.f};
        SkRect drawRect = clipRect.makeOffset(10.f, 0.f);

        cs.save();
        cs.clipRect(SkMatrix::I(), clipRect, GrAA::kNo, SkClipOp::kIntersect);

        GrAppliedClip out(kDeviceBounds.size());
        GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                         &out, &drawRect);
        REPORTER_ASSERT(r, SkRect::Make(kDeviceBounds).contains(drawRect),
                        "Draw rect should be clipped to device rect");
        REPORTER_ASSERT(r, effect == GrClip::Effect::kUnclipped,
                        "After device clipping, this should be detected as contained within clip");
        cs.restore();
    }

    // Non-AA device rect intersect is just a scissor
    {
        SkRect clipRect = {15.3f, 17.23f, 30.2f, 50.8f};
        SkRect drawRect = clipRect.makeOutset(10.f, 10.f);
        SkIRect expectedScissor = clipRect.round();

        cs.save();
        cs.clipRect(SkMatrix::I(), clipRect, GrAA::kNo, SkClipOp::kIntersect);

        GrAppliedClip out(kDeviceBounds.size());
        GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                         &out, &drawRect);
        REPORTER_ASSERT(r, effect == GrClip::Effect::kClipped, "Draw should be clipped by rect");
        REPORTER_ASSERT(r, !out.hasCoverageFragmentProcessor(), "Clip should not use coverage FPs");
        REPORTER_ASSERT(r, !out.hardClip().hasStencilClip(), "Clip should not need stencil");
        REPORTER_ASSERT(r, !out.hardClip().windowRectsState().enabled(),
                        "Clip should not need window rects");
        REPORTER_ASSERT(r, out.scissorState().enabled() &&
                           out.scissorState().rect() == expectedScissor,
                        "Clip has unexpected scissor rectangle");
        cs.restore();
    }

    // Analytic coverage FPs
    auto testHasCoverageFP = [&](SkRect drawBounds) {
        GrAppliedClip out(kDeviceBounds.size());
        GrClip::Effect effect = cs.apply(context.get(), sdc.get(), NoOp::Get(), GrAAType::kCoverage,
                                         &out, &drawBounds);
        REPORTER_ASSERT(r, effect == GrClip::Effect::kClipped, "Draw should be clipped");
        REPORTER_ASSERT(r, out.scissorState().enabled(), "Coverage FPs should still set scissor");
        REPORTER_ASSERT(r, out.hasCoverageFragmentProcessor(), "Clip should use coverage FP");
    };

    // Axis-aligned rect can be an analytic FP
    {
        cs.save();
        cs.clipRect(SkMatrix::I(), {10.2f, 8.342f, 63.f, 23.3f}, GrAA::kYes,
                    SkClipOp::kDifference);
        testHasCoverageFP({9.f, 10.f, 30.f, 18.f});
        cs.restore();
    }

    // Axis-aligned round rect can be an analytic FP
    {
        SkRect rect = {4.f, 8.f, 20.f, 20.f};
        cs.save();
        cs.clipRRect(SkMatrix::I(), SkRRect::MakeRectXY(rect, 3.f, 3.f), GrAA::kYes,
                     SkClipOp::kIntersect);
        testHasCoverageFP(rect.makeOffset(2.f, 2.f));
        cs.restore();
    }

    // Transformed rect can be an analytic FP
    {
        SkRect rect = {14.f, 8.f, 30.f, 22.34f};
        SkMatrix rot = SkMatrix::RotateDeg(34.f);
        cs.save();
        cs.clipRect(rot, rect, GrAA::kNo, SkClipOp::kIntersect);
        testHasCoverageFP(rot.mapRect(rect));
        cs.restore();
    }

    // Convex polygons can be an analytic FP
    {
        SkRect rect = {15.f, 15.f, 45.f, 45.f};
        cs.save();
        cs.clipPath(SkMatrix::I(), make_octagon(rect), GrAA::kYes, SkClipOp::kIntersect);
        testHasCoverageFP(rect.makeOutset(2.f, 2.f));
        cs.restore();
    }
}

// Must disable tessellation in order to trigger SW mask generation when the clip stack is applied.
static void disable_tessellation_atlas(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kNone;
    options->fAvoidStencilBuffers = true;
}

DEF_GPUTEST_FOR_CONTEXTS(GrClipStack_SWMask,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         r, ctxInfo, disable_tessellation_atlas) {
    GrDirectContext* context = ctxInfo.directContext();
    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc = skgpu::v1::SurfaceDrawContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact, kDeviceBounds.size(),
            SkSurfaceProps());

    SkSimpleMatrixProvider matrixProvider = SkMatrix::I();
    std::unique_ptr<GrClipStack> cs(new GrClipStack(kDeviceBounds, &matrixProvider, false));

    auto addMaskRequiringClip = [&](SkScalar x, SkScalar y, SkScalar radius) {
        SkPath path;
        path.addCircle(x, y, radius);
        path.addCircle(x + radius / 2.f, y + radius / 2.f, radius);
        path.setFillType(SkPathFillType::kEvenOdd);

        // Use AA so that clip application does not route through the stencil buffer
        cs->clipPath(SkMatrix::I(), path, GrAA::kYes, SkClipOp::kIntersect);
    };

    auto drawRect = [&](SkRect drawBounds) {
        GrPaint paint;
        paint.setColor4f({1.f, 1.f, 1.f, 1.f});
        sdc->drawRect(cs.get(), std::move(paint), GrAA::kYes, SkMatrix::I(), drawBounds);
    };

    auto generateMask = [&](SkRect drawBounds) {
        GrUniqueKey priorKey = cs->testingOnly_getLastSWMaskKey();
        drawRect(drawBounds);
        GrUniqueKey newKey = cs->testingOnly_getLastSWMaskKey();
        REPORTER_ASSERT(r, priorKey != newKey, "Did not generate a new SW mask key as expected");
        return newKey;
    };

    auto verifyKeys = [&](const std::vector<GrUniqueKey>& expectedKeys,
                          const std::vector<GrUniqueKey>& releasedKeys) {
        context->flush();
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

#ifdef SK_DEBUG
        // The proxy providers key count fluctuates based on proxy lifetime, but we want to
        // verify the resource count, and that requires using key tags that are debug-only.
        SkASSERT(expectedKeys.size() > 0 || releasedKeys.size() > 0);
        const char* tag = expectedKeys.size() > 0 ? expectedKeys[0].tag() : releasedKeys[0].tag();
        GrResourceCache* cache = context->priv().getResourceCache();
        int numProxies = cache->countUniqueKeysWithTag(tag);
        REPORTER_ASSERT(r, (int) expectedKeys.size() == numProxies,
                        "Unexpected proxy count, got %d, not %d",
                        numProxies, (int) expectedKeys.size());
#endif

        for (const auto& key : expectedKeys) {
            auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
            REPORTER_ASSERT(r, SkToBool(proxy), "Unable to find resource for expected mask key");
        }
        for (const auto& key : releasedKeys) {
            auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
            REPORTER_ASSERT(r, !SkToBool(proxy), "SW mask not released as expected");
        }
    };

    // Creates a mask for a complex clip
    cs->save();
    addMaskRequiringClip(5.f, 5.f, 20.f);
    GrUniqueKey keyADepth1 = generateMask({0.f, 0.f, 20.f, 20.f});
    GrUniqueKey keyBDepth1 = generateMask({10.f, 10.f, 30.f, 30.f});
    verifyKeys({keyADepth1, keyBDepth1}, {});

    // Creates a new mask for a new save record, but doesn't delete the old records
    cs->save();
    addMaskRequiringClip(6.f, 6.f, 15.f);
    GrUniqueKey keyADepth2 = generateMask({0.f, 0.f, 20.f, 20.f});
    GrUniqueKey keyBDepth2 = generateMask({10.f, 10.f, 30.f, 30.f});
    verifyKeys({keyADepth1, keyBDepth1, keyADepth2, keyBDepth2}, {});

    // Release after modifying the current record (even if we don't draw anything)
    addMaskRequiringClip(4.f, 4.f, 15.f);
    GrUniqueKey keyCDepth2 = generateMask({4.f, 4.f, 16.f, 20.f});
    verifyKeys({keyADepth1, keyBDepth1, keyCDepth2}, {keyADepth2, keyBDepth2});

    // Release after restoring an older record
    cs->restore();
    verifyKeys({keyADepth1, keyBDepth1}, {keyCDepth2});

    // Drawing finds the old masks at depth 1 still w/o making new ones
    drawRect({0.f, 0.f, 20.f, 20.f});
    drawRect({10.f, 10.f, 30.f, 30.f});
    verifyKeys({keyADepth1, keyBDepth1}, {});

    // Drawing something contained within a previous mask also does not make a new one
    drawRect({5.f, 5.f, 15.f, 15.f});
    verifyKeys({keyADepth1, keyBDepth1}, {});

    // Release on destruction
    cs = nullptr;
    verifyKeys({}, {keyADepth1, keyBDepth1});
}
