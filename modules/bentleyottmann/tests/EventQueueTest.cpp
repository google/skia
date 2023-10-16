// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/EventQueue.h"
#include "tests/Test.h"

namespace bentleyottmann {
class EventQueueTestingPeer {
public:
    static Event NextEvent(EventQueue* eq) {
        SkASSERT(eq->hasMoreEvents());

        auto firstElement = eq->fQueue.begin();

        // Extract event at the beginning of the queue.
        Event event = *firstElement;

        // Remove the beginning element from the queue.
        eq->fQueue.erase(firstElement);

        return event;
    }
};
}  // namespace bentleyottmann

using namespace bentleyottmann;

DEF_TEST(BO_EventQueueOrdering, reporter) {
    { // Check that event types are ordered correctly.
        EventQueue::Queue q;

        // Insert the events in reverse order.
        Point eventPoint = {100, 100};
        Segment s = {{100, 100}, {200, 200}};
        q.insert(Event{eventPoint, Upper{s}});
        Segment s0 = {{50, 50}, {150, 150}},
                s1 = {{150, 50}, {50, 150}};
        q.insert(Event{eventPoint, Cross{s0, s1}});
        q.insert(Event{eventPoint, Lower{}});

        // Make sure that the events are in the right order.
        auto cursor = q.begin();
        REPORTER_ASSERT(reporter, std::holds_alternative<Lower>(cursor->type));
        ++cursor;
        REPORTER_ASSERT(reporter, std::holds_alternative<Cross>(cursor->type));
        ++cursor;
        REPORTER_ASSERT(reporter, std::holds_alternative<Upper>(cursor->type));
    }
}

DEF_TEST(BO_EventQueueBasic, reporter) {
    {
        EventQueue::Queue q;
        EventQueue eq{std::move(q)};
        REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
    }
    {
        EventQueue::Queue q;
        Point eventPoint = {100, 100};
        q.insert({eventPoint, Lower{} });
        EventQueue eq{std::move(q)};
        {
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            Event e = EventQueueTestingPeer::NextEvent(&eq);
            REPORTER_ASSERT(reporter, e.where == eventPoint);
            REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
        }
    }
    { // Check that Lower events are de-duplicated.
        EventQueue::Queue q;
        Point eventPoint = {100, 100};
        q.insert({eventPoint, Lower{}});
        q.insert({eventPoint, Lower{}});
        EventQueue eq{std::move(q)};
        {
            // There should be only one lower because of queue de-duplication
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            auto [p, _] = EventQueueTestingPeer::NextEvent(&eq);
            REPORTER_ASSERT(reporter, p == eventPoint);
            REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
        }
    }
    { // Check that Lower distinct Lower events are distinct.
        EventQueue::Queue q;
        Point eventPoint1 = {100, 100};
        Point eventPoint2 = {100, 101};

        q.insert({eventPoint1, Lower{}});
        q.insert({eventPoint2, Lower{}});
        EventQueue eq{std::move(q)};
        {
            // There should be only one lower because of queue de-duplication
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            auto [p, _] = EventQueueTestingPeer::NextEvent(&eq);
            REPORTER_ASSERT(reporter, p == eventPoint1);
        }
        {
            // There should be only one lower because of queue de-duplication
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            auto [p, _] = EventQueueTestingPeer::NextEvent(&eq);
            REPORTER_ASSERT(reporter, p == eventPoint2);
            REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
        }
    }
    { // Check that non-Lower events are separate.
        EventQueue::Queue q;
        Segment s0 {{0, 0}, {100, 100}};
        Segment s1 {{0, 0}, {-100, 100}};
        q.insert({Point{0, 0}, Upper{s0}});
        q.insert({Point{0, 0}, Upper{s1}});
        EventQueue eq{std::move(q)};
        {
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            Event e = EventQueueTestingPeer::NextEvent(&eq);
            Point upperPt = Point{0, 0};
            REPORTER_ASSERT(reporter, e.where == upperPt);
            REPORTER_ASSERT(reporter, e.type.index() == 2);
            Upper upper = std::get<Upper>(e.type);
            REPORTER_ASSERT(reporter, !(upper < Upper{s1}) && !(Upper{s1} < upper));
            Event e2 = EventQueueTestingPeer::NextEvent(&eq);
            REPORTER_ASSERT(reporter, e2.where == upperPt);
            REPORTER_ASSERT(reporter, e2.type.index() == 2);
            Upper upper2 = std::get<Upper>(e2.type);
            REPORTER_ASSERT(reporter, !(upper2 < Upper{s0}) && !(Upper{s0} < upper2));
            REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
        }
    }
}

enum HasDeletions {
    kHasDeletions,   // The handleDeletions call should be called.
    kHasNoDeletions  // The handleDeletions call should not be called.
};

struct TestEventHandler : public SweepLineInterface {
    TestEventHandler(skiatest::Reporter*r,
                     Point eventPoint,
                     SkSpan<const Segment> deletions,
                     SkSpan<const Segment> insertions,
                     SkSpan<const Crossing> crossings,
                     HasDeletions hasDeletions = kHasDeletions)
        : fR(r)
        , fCandidateEventPoint{eventPoint}
        , fDeletions{deletions.begin(), deletions.end()}
        , fInsertions{insertions.begin(), insertions.end()}
        , fCrossings{crossings.begin(), crossings.end()}
        , fHasDeletions{hasDeletions} {}

    void handleDeletions(Point eventPoint,
                         const DeletionSegmentSet& removing) override {
        if (fHasDeletions == kHasNoDeletions) {
            REPORTER_ASSERT(fR, false, "There should be no deletions.");
            return;
        }

        REPORTER_ASSERT(fR, eventPoint == fCandidateEventPoint);

        REPORTER_ASSERT(fR, removing.size() == fDeletions.size());

        for (const Segment& s : fDeletions) {
            REPORTER_ASSERT(fR, removing.find(s) != removing.end());
        }
    }

    void
    handleInsertionsAndCheckForNewCrossings(Point eventPoint,
                                            const InsertionSegmentSet& inserting,
                                            EventQueueInterface* queue) override {
        REPORTER_ASSERT(fR, eventPoint == fCandidateEventPoint);

        REPORTER_ASSERT(fR, inserting.size() == fInsertions.size());

        for (const Segment& s : fInsertions) {
            REPORTER_ASSERT(fR, inserting.find(s) != inserting.end());
        }

        for (const Crossing& crossing : fCrossings) {
            auto [s0, s1, pt] = crossing;
            queue->addCrossing(pt, s0, s1);
        }
    }

    skiatest::Reporter* const fR;
    const Point fCandidateEventPoint;
    std::vector<Segment> fDeletions;
    std::vector<Segment> fInsertions;
    std::vector<Crossing> fCrossings;
    const HasDeletions fHasDeletions;
};

DEF_TEST(BO_EventQueueHandlerInterface, reporter) {
    { // Check that a Lower event is added while processing the Upper event.
        EventQueue::Queue q;
        static constexpr Point eventPoint = {100, 100};
        static constexpr Point endPoint = {200, 200};
        static constexpr Segment s = {eventPoint, endPoint};
        q.insert(Event{eventPoint, Upper{s}});
        EventQueue eq{std::move(q)};

        REPORTER_ASSERT(reporter, eq.hasMoreEvents());


        TestEventHandler eh1{reporter, eventPoint, {}, {s}, {}, kHasNoDeletions};
        eq.handleNextEventPoint(&eh1);

        TestEventHandler eh2{reporter, endPoint, {}, {}, {}};
        eq.handleNextEventPoint(&eh2);

        REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
    }

    { // Check an entire crossing event.
        EventQueue::Queue q;
        static constexpr Point b0 = {100, 100};
        static constexpr Point e0 = {200, 200};
        static constexpr Segment s0 = {b0, e0};
        static constexpr Point b1 = {200, 100};
        static constexpr Point e1 = {100, 200};
        static constexpr Segment s1 = {b1, e1};
        static constexpr Point crossingPoint = {150, 150};

        // Load crossing segments into the queue
        q.insert(Event{b0, Upper{s0}});
        q.insert(Event{b1, Upper{s1}});
        EventQueue eq{std::move(q)};

        REPORTER_ASSERT(reporter, eq.hasMoreEvents());

        TestEventHandler eh1{reporter, b0, {}, {s0}, {}, kHasNoDeletions};
        eq.handleNextEventPoint(&eh1);

        TestEventHandler eh2{reporter, b1, {}, {s1}, {{s0, s1, crossingPoint}}, kHasNoDeletions};
        eq.handleNextEventPoint(&eh2);

        TestEventHandler eh3{reporter, crossingPoint, {s0, s1}, {s0, s1}, {}};
        eq.handleNextEventPoint(&eh3);

        TestEventHandler eh4{reporter, e1, {}, {}, {}};
        eq.handleNextEventPoint(&eh4);

        TestEventHandler eh5{reporter, e0, {}, {}, {}};
        eq.handleNextEventPoint(&eh5);

        REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
    }
}

