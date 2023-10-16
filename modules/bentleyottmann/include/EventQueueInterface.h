// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef EventQueueInterface_DEFINED
#define EventQueueInterface_DEFINED

#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <set>

// The EventQueueInterface and the SweepLineInterface allow the EventQueue and the SweepLine
// to be tested independently of each other. This allows very specific scenarios to be setup and
// tested in isolation.

namespace bentleyottmann {
// -- EventQueueInterface --------------------------------------------------------------------------
// An EventQueueInterface implementation must be able to add crossing events into the event queue.
class EventQueueInterface {
public:
    EventQueueInterface() = default;
    EventQueueInterface(const EventQueueInterface&) = default;
    EventQueueInterface(EventQueueInterface&&) = default;
    EventQueueInterface& operator=(const EventQueueInterface&) = default;
    EventQueueInterface& operator=(EventQueueInterface&&) = default;
    virtual ~EventQueueInterface() = default;

    virtual void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) = 0;
};

using DeletionSegmentSet = std::set<Segment>;
struct OrderBySlope {
    bool operator()(const Segment& s0, const Segment& s1) const;
};
// The set of insertion segments is ordered by slope. Since all the lines pass through the same
// point, then the slope of each line must be ordered from smallest to largest to keep the
// segment order correct in the sweep line.
using InsertionSegmentSet = std::set<Segment, OrderBySlope>;

// The EventQueue uses an object of SweepLineInterface to find new crossings when manipulating
// the sweep line.
class SweepLineInterface {
public:
    virtual ~SweepLineInterface() = default;

    // These are the segments to remove from the sweep line.
    virtual void handleDeletions(Point eventPoint, const DeletionSegmentSet& removing) = 0;

    // Insert inserting into the sweep line. Check the inserting segments against the existing
    // sweep line segments and report any crossings using the addCrossing from the
    // EventQueueInterface.
    virtual void handleInsertionsAndCheckForNewCrossings(
            Point eventPoint, const InsertionSegmentSet& inserting, EventQueueInterface* queue) = 0;
};
}  // namespace bentleyottmann
#endif  // EventQueueInterface_DEFINED
