// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef EventQueue_DEFINED
#define EventQueue_DEFINED

#include "include/core/SkSpan.h"
#include "modules/bentleyottmann/include/EventQueueInterface.h"
#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <optional>
#include <set>
#include <tuple>
#include <variant>
#include <vector>

namespace bentleyottmann {

struct Lower {
    // All Lowers are equal.
    friend bool operator< (const Lower&, const Lower&) {
        return false;
    }
};

struct Upper {
    Segment s;

    // Arbitrary comparison for queue uniqueness.
    friend bool operator< (const Upper& u0, const Upper& u1) {
        return std::tie(u0.s.p0, u0.s.p1) < std::tie(u1.s.p0, u1.s.p1);
    }
};

struct Cross {
    Segment s0;
    Segment s1;

    // Arbitrary comparison for queue uniqueness.
    friend bool operator< (const Cross& c0, const Cross& c1) {
        return std::tie(c0.s0.p0, c0.s0.p1, c0.s1.p0, c0.s1.p1) <
                    std::tie(c1.s0.p0, c1.s0.p1, c1.s1.p0, c1.s1.p1);
    }
};

using EventType = std::variant<Lower, Cross, Upper>;

struct Event {
    Point where;
    EventType type;

    friend bool operator< (const Event& e0, const Event& e1) {
        return std::tie(e0.where, e0.type) < std::tie(e1.where, e1.type);
    }
};

class EventQueue : public EventQueueInterface {
public:
    // Queue ordered by Event where the point is the most important followed by type and then
    // contents of the event. The ordering of the contents of the event is arbitrary but need to
    // enforce uniqueness of the events in the queue.
    using Queue = std::set<Event>;

    static std::optional<EventQueue> Make(SkSpan<const Segment> segments);

    EventQueue(Queue&& queue);
    EventQueue() = default;

    void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) override;

    bool hasMoreEvents() const;
    void handleNextEventPoint(SweepLineInterface* handler);
    std::vector<Crossing> crossings();

private:
    friend class EventQueueTestingPeer;
    Point fLastEventPoint = Point::Smallest();

    void add(const Event& e);

    DeletionSegmentSet fDeletionSet;
    InsertionSegmentSet fInsertionSet;
    Queue fQueue;
    std::vector<Crossing> fCrossings;
};
}  // namespace bentleyottmann
#endif  // EventQueue_DEFINED
