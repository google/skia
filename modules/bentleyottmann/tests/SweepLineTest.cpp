// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/EventQueueInterface.h"
#include "modules/bentleyottmann/include/Point.h"

#include "tests/Test.h"

using namespace bentleyottmann;

namespace bentleyottmann {
struct SweepLineTestingPeer {
    SweepLineTestingPeer(SweepLine* sl) : fSL{sl} {}
    void verifySweepLine(int32_t y) const {
        fSL->verify(y);
    }
    void insertSegment(int i, const Segment& s) {
        auto& v = fSL->fSweepLine;
        v.insert(v.begin() + i, s);
    }
    size_t size() const {
        return fSL->fSweepLine.size();
    }

    SweepLine* const fSL;
};
}  // namespace bentleyottmann

using TP = SweepLineTestingPeer;

DEF_TEST(BO_SweepLineInsert, reporter) {
    {
        SweepLine sweepLine;
        TP tp{&sweepLine};
        tp.verifySweepLine(0);
    }
    { // Handle insert
        SweepLine sweepLine;
        TP tp{&sweepLine};

        class FailIfEventQueueAdd : public EventQueueInterface {
        public:
            void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) override {
                SK_ABORT("There should be no crossings.");
            }
        } eventQueue;
        InsertionSegmentSet insertions;
        Segment s = {{0, -100}, {0, 100}};

        insertions.insert(s);

        tp.verifySweepLine(-99);

        sweepLine.handleInsertionsAndCheckForNewCrossings(
                Point{0, -100}, insertions, &eventQueue);
        REPORTER_ASSERT(reporter, tp.size() == 3);

        tp.verifySweepLine(-99);
    }

    { // Handle 3 segments where removing middle segment introduces crossing
        SweepLine sweepLine;
        TP tp{&sweepLine};

        Point p0 = {-100, -100},
              p1 = { 100,  100},
              p2 = { 100, -100},
              p3 = {-100,  100},
              p4 = {   0, -100},
              p5 = {   0,  -50};
        Segment s0 = {p0, p1},
                s1 = {p2, p3},
                s2 = {p4, p5};

        class CollectCrossings : public EventQueueInterface {
        public:
            void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) override {
                fCrossing.push_back({s0, s1, crossingPoint});
            }
            std::vector<Crossing> fCrossing;
        } eventQueue;

        { // Simulate handling Upper s0
            InsertionSegmentSet insertions;
            insertions.insert(s0);
            tp.verifySweepLine(-99);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p0, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 3);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 0);
            tp.verifySweepLine(-99);
        }
        { // Simulate handling Upper s2
            InsertionSegmentSet insertions;
            insertions.insert(s2);
            tp.verifySweepLine(-99);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p4, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 4);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 0);
            tp.verifySweepLine(-99);
        }
        { // Simulate handling Upper s1
            InsertionSegmentSet insertions;
            insertions.insert(s1);
            tp.verifySweepLine(-99);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p2, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 5);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 0);
            tp.verifySweepLine(-99);
        }
        { // Simulate handling Lower s2 which introduces a crossing
            DeletionSegmentSet deletions;  // empty set because this will be a lower event
            InsertionSegmentSet insertions;
            tp.verifySweepLine(-51);
            sweepLine.handleDeletions(p5, deletions);
            REPORTER_ASSERT(reporter, tp.size() == 4);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p5, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 1);
            tp.verifySweepLine(-51);
        }
        { // Handle crossing
            DeletionSegmentSet deletions{s0, s1};  // empty set because this will be a lower event
            InsertionSegmentSet insertions{s0, s1};
            tp.verifySweepLine(-1);  // Check above the crossing
            sweepLine.handleDeletions({0,0}, deletions);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    {0,0}, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 4);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 1);
            tp.verifySweepLine(1);  // Make sure things are correct after deletion
        }
        { // Handle deletion s1
            DeletionSegmentSet deletions{};  // empty set because this will be a lower event
            InsertionSegmentSet insertions{};
            tp.verifySweepLine(99);  // Check above the crossing
            sweepLine.handleDeletions(p3, deletions);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p3, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 3);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 1);
            tp.verifySweepLine(99);  // Make sure sentinels are correct.
        }
        { // Handle deletion s0
            DeletionSegmentSet deletions{};  // empty set because this will be a lower event
            InsertionSegmentSet insertions{};
            tp.verifySweepLine(99);  // Check above the crossing
            sweepLine.handleDeletions(p1, deletions);
            sweepLine.handleInsertionsAndCheckForNewCrossings(
                    p1, insertions, &eventQueue);
            REPORTER_ASSERT(reporter, tp.size() == 2);
            REPORTER_ASSERT(reporter, eventQueue.fCrossing.size() == 1);
            tp.verifySweepLine(99);  // Make sure sentinels are correct.
        }
    }
}
