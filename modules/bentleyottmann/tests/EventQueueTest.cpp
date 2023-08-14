// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/EventQueue.h"
#include "tests/Test.h"

using namespace bentleyottmann;

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
            Event e = eq.nextEvent();
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
            auto [p, _] = eq.nextEvent();
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
            auto [p, _] = eq.nextEvent();
            REPORTER_ASSERT(reporter, p == eventPoint1);
        }
        {
            // There should be only one lower because of queue de-duplication
            REPORTER_ASSERT(reporter, eq.hasMoreEvents());
            auto [p, _] = eq.nextEvent();
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
            Event e = eq.nextEvent();
            Point upperPt = Point{0, 0};
            REPORTER_ASSERT(reporter, e.where == upperPt);
            REPORTER_ASSERT(reporter, e.type.index() == 2);
            Upper upper = std::get<Upper>(e.type);
            REPORTER_ASSERT(reporter, !(upper < Upper{s1}) && !(Upper{s1} < upper));
            Event e2 = eq.nextEvent();
            REPORTER_ASSERT(reporter, e2.where == upperPt);
            REPORTER_ASSERT(reporter, e2.type.index() == 2);
            Upper upper2 = std::get<Upper>(e2.type);
            REPORTER_ASSERT(reporter, !(upper2 < Upper{s0}) && !(Upper{s0} < upper2));
            REPORTER_ASSERT(reporter, !eq.hasMoreEvents());
        }

    }
}

