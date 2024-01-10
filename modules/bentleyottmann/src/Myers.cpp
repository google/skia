// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "modules/bentleyottmann/include/Int96.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace myers {

// -- Point ----------------------------------------------------------------------------------------
Point operator-(const Point& p0, const Point& p1) {
    return {p0.x - p1.x, p0.y - p1.y};
}

std::tuple<int64_t, int64_t> point_to_s64(Point p) {
    return std::make_tuple(SkToS64(p.x), SkToS64(p.y));
}

// -- Segment --------------------------------------------------------------------------------------
const Point& Segment::upper() const {
    return fUpper;
}

const Point& Segment::lower() const {
    return fLower;
}

std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [left, right] = std::minmax(fUpper.x, fLower.x);
    return std::make_tuple(left, fUpper.y, right, fLower.y);
}

bool Segment::isHorizontal() const {
    return fUpper.y == fLower.y;
}

bool Segment::isVertical() const {
    return fUpper.x == fLower.x;
}

// Return:
//    | d0x   d1x |
//    | d0y   d1y |
int64_t cross(Point d0, Point d1) {
    const auto [d0x, d0y] = point_to_s64(d0);
    const auto [d1x, d1y] = point_to_s64(d1);
    return d0x * d1y - d1x * d0y;
}

// compare_slopes returns a comparison of the slope of s0 to the slope of s1 where
//    slope(s) = dx / dy
// instead of the regular dy / dx, and neither s0 nor s1 are horizontal.
//
// The slope for non-horizontal segments monotonically increases from the smallest along the
// negative x-axis increasing counterclockwise to the largest along the positive x-axis.
int64_t compare_slopes(const Segment& s0, const Segment& s1) {
    // Handle cases involving horizontal segments.
    if (s0.isHorizontal() || s1.isHorizontal()) {
        if (s0.isHorizontal() && s1.isHorizontal()) {
            // slope(s0) == slope(s1)
            return 0;
        }
        if (s0.isHorizontal()) {
            // slope(s0) > slope(s1)
            return 1;
        } else {
            // slope(s0) < slope(s1)
            return -1;
        }
    }

    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const Point d0 = l0 - u0;
    const Point d1 = l1 - u1;

    // Since horizontal lines are handled separately and because of the ordering of points for
    // a segment, then d0y and d1y should always be positive.
    SkASSERT(d0.y > 0 && d1.y > 0);

    //     * slope(s0) = d0.x / d0.y
    //     * slope(s1) = d1.x / d1.y
    // If we want to find d0.x / d0.y < d1.x / d1.y, then
    //    d0x * d1y < d1x * d0y
    //    d0x * d1y - d1x * d0y < 0.
    //
    // We know that d0.y and d1.y are both positive, therefore, we can do a cross multiply without
    // worrying about changing the relation.
    // We can define ==, and > in a similar way:
    //    * <  - cross_of_slopes(s0, s1) < 0
    //    * == - cross_of_slopes(s0, s1) == 0
    //    * >  - cross_of_slopes(s0, s1) > 0
    return cross(d0, d1);
}

// Returns true of slope(s0) < slope(s1). See compare_slopes above for more information.
bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1) {
    return compare_slopes(s0, s1) < 0;
}

// compare_point_to_segment the relation between a point p and a segment s in the following way:
//     * p < s if the cross product is negative.
//     * p == s if the cross product is zero.
//     * p > s if the cross product is positive.
int64_t compare_point_to_segment(Point p, const Segment& s) {
    const auto [u, l] = s;

    // The segment must span p vertically.
    SkASSERT(u.y <= p.y && p.y <= l.y);

    // Check horizontal extents.
    {
        const auto [left, right] = std::minmax(u.x, l.x);
        if (p.x < left) {
            return -1;
        }

        if (right < p.x) {
            return 1;
        }
    }

    // If s is horizontal, then p is on the interval [u.x, l.x].
    if (s.isHorizontal()) {
        return 0;
    }

    // The point p < s when:
    //     p.x < u.x + (l.x - u.x)(p.y - u.y) / (l.y - u.y),
    //     p.x - u.x < (l.x - u.x)(p.y - u.y) / (l.y - u.y),
    //     (p.x - u.x)(l.y - u.y) < (l.x - u.x)(p.y - u.y),
    //     (p.x - u.x)(l.y - u.y) - (l.x - u.x)(p.y - u.y) < 0,
    //     (p - u) x (l - u) < 0,
    //     dUtoP x dS < 0.
    // The other relations can be implemented in a similar way.
    const Point dUToP = p - u;
    const Point dS = l - u;

    SkASSERT(dS.y > 0);
    return cross(dUToP, dS);
}

// segment_less_than_upper_to_insert is used with std::lower_bound to find the place to insert the
// segment to_insert in a vector. The signature of this function is crafted to work with
// lower_bound.
bool segment_less_than_upper_to_insert(const Segment& segment, const Segment& to_insert) {
    const int64_t compare = compare_point_to_segment(to_insert.upper(), segment);

    // compare > 0 when segment < to_insert.upper().
    return (compare > 0) || ((compare == 0) && slope_s0_less_than_slope_s1(segment, to_insert));
}

// Return true if s0(y) < s1(y) else if s0(y) == s1(y) then slope(s0) < slope(s1)
bool s0_less_than_s1_at_y(const Segment& s0, const Segment& s1, int32_t y) {
    // Neither s0 nor s1 are horizontal because this is used during the sorting phase
    SkASSERT(!s0.isHorizontal() && !s1.isHorizontal());

    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const auto [left0, right0] = std::minmax(u0.x, l0.x);
    const auto [left1, right1] = std::minmax(u1.x, l1.x);

    if (right0 < left1) {
        return true;
    } else if (right1 < left0) {
        return false;
    }

    const Point d0 = l0 - u0;
    const Point d1 = l1 - u1;

    // Since horizontal lines are handled separately and the ordering of points for the segment,
    // then there should always be positive Dy.
    SkASSERT(d0.y > 0 && d1.y > 0);

    namespace bo = bentleyottmann;
    using Int96 = bo::Int96;

    // Defining s0(y) and s1(y),
    //    s0(y) = u0.x + (y - u0.y) * d0.x / d0.y
    //    s1(y) = u1.x + (y - u1.y) * d1.x / d1.y
    // Find the following
    //    s0(y) < s1(y)
    // Substituting s0(y) and s1(y)
    //    u0.x + (y - u0.y) * d0.x / d0.y < u1.x + (y - u1.y) * d1.x / d1.y
    // Factoring out the denominator.
    //    (u0.x * d0.y + (y - u0.y) * d0.x) / d0.y < (u1.x * d1.y + (y - u1.y) * d1.x) / d1.y
    // Cross-multiplying the denominators. The sign will not switch because d0.y and d1.y are
    // always positive.
    //    d1.y * (u0.x * d0.y + (y - u0.y) * d0.x) < d0.y * (u1.x * d1.y + (y - u1.y) * d1.x)
    // If these are equal, then we use the slope to break the tie.
    //    d0.x / d0.y < d1.x / d1.y
    // Cross multiplying leaves.
    //    d0.x * d1.y < d1.x * d0.y
    const Int96 lhs = bo::multiply(d1.y, u0.x * SkToS64(d0.y) + (y - u0.y) * SkToS64(d0.x));
    const Int96 rhs = bo::multiply(d0.y, u1.x * SkToS64(d1.y) + (y - u1.y) * SkToS64(d1.x));

    return lhs < rhs || ((lhs == rhs) && slope_s0_less_than_slope_s1(s0, s1));
}

// -- Event ----------------------------------------------------------------------------------------
// Events are horizontal lines at a given y where segments are added, or they contain one or
// more horizontal lines, or segments end.
struct Event {
    const int32_t y;

    // The set of segments that begin at y.
    SkSpan<const Segment> begin;

    // The set of segments that are horizontal on y.
    SkSpan<const Segment> horizontal;

    // The set of segments that end on y.
    SkSpan<const Segment> end;
};

// -- EventQueue -----------------------------------------------------------------------------------
// The EventQueue produces Events. Events are never added to the queue after initial creation.
class EventQueue {
    class Iterator {
    public:
        using value_type = Event;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        Iterator(const EventQueue& eventQueue, size_t index)
            : fEventQueue{eventQueue}
            , fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fEventQueue, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        value_type operator*() { return fEventQueue[fIndex]; }
        friend difference_type operator-(Iterator lhs, Iterator rhs) {
            return lhs.fIndex - rhs.fIndex;
        }

    private:
        const EventQueue& fEventQueue;
        size_t fIndex = 0;
    };

    // Events are stored using CompactEvent, Events are only passed back from nextEvent. start,
    // endOfBegin, etc. are all indexes into fSegmentStorage. The beginning segments for the event
    // are from start to endOfBegin, the horizontal segments are from endOfBegin to endOfStart, and
    // the end segments are from endOfHorizontal to endOfEnd.
    class CompactEvent {
    public:
        const int32_t y;
        const int32_t start;
        const int32_t endOfBegin;
        const int32_t endOfHorizontal;
        const int32_t endOfEnd;
    };

public:
    // Given a list of segments make an EventQueue, and populate its queue with events.
    static EventQueue Make(const SkSpan<const Segment> segments) {
        SkASSERT(!segments.empty());
        SkASSERT(segments.size() < INT32_MAX);

        enum EventType {
            kBegin = 0,
            kHorizontal = 1,
            kEnd = 2
        };

        // A vector of SetupTuple when ordered will produce the events, and all the different
        // sets of segments (beginning, etc.).
        struct SetupTuple {
            // The main ordering of events.
            int32_t yOrdering;

            // Group together like event types together.
            EventType type;

            // Break ties if yOrdering is the same.
            int32_t xTieBreaker;

            // We want to sort the segments, but they are not part of the key.
            Segment originalSegment;

            bool operator==(const SetupTuple& r) const {
                return
                    std::tie(this->yOrdering, this->type, this->xTieBreaker, this->originalSegment)
                        == std::tie(r.yOrdering, r.type, r.xTieBreaker, r.originalSegment);
            }
        };

        std::vector<SetupTuple> eventOrdering;
        for (const auto& s : segments) {

            // Exclude zero length segments.
            if (s.upper() == s.lower()) {
                continue;
            }

            if (s.isHorizontal()) {
                // Tag for the horizontal set.
                eventOrdering.push_back(SetupTuple{s.upper().y, kHorizontal, -s.upper().x, s});
            } else {
                // Tag for the beginning and ending sets.
                eventOrdering.push_back(SetupTuple{s.upper().y, kBegin, -s.upper().x, s});
                eventOrdering.push_back(SetupTuple{s.lower().y, kEnd, -s.lower().x, s});
            }
        }

        // Order the tuples by y, then by set type, then by x value.
        auto eventLess = [](const SetupTuple& l, const SetupTuple& r) {
            return std::tie(l.yOrdering, l.type, l.xTieBreaker) <
                   std::tie(r.yOrdering, r.type, r.xTieBreaker);
        };

        // Sort the events.
        std::sort(eventOrdering.begin(), eventOrdering.end(), eventLess);

        // Remove duplicate segments.
        auto eraseFrom = std::unique(eventOrdering.begin(), eventOrdering.end());
        eventOrdering.erase(eraseFrom, eventOrdering.end());

        std::vector<CompactEvent> events;
        std::vector<Segment> segmentStorage;
        segmentStorage.reserve(eventOrdering.size());

        int32_t currentY = eventOrdering.front().yOrdering;
        int32_t start = 0,
                endOfBegin = 0,
                endOfHorizontal = 0,
                endOfEnd = 0;
        for (const auto& [y, type, _, s] : eventOrdering) {
            // If this is a new y then create the compact event.
            if (currentY != y) {
                events.push_back(CompactEvent{currentY,
                                              start,
                                              endOfBegin,
                                              endOfHorizontal,
                                              endOfEnd});
                start = endOfBegin = endOfHorizontal = endOfEnd = segmentStorage.size();
                currentY = y;
            }

            segmentStorage.push_back(s);

            // Increment the various set indices.
            const size_t segmentCount = segmentStorage.size();
            switch (type) {
                case kBegin: endOfBegin = segmentCount;
                    [[fallthrough]];
                case kHorizontal: endOfHorizontal = segmentCount;
                    [[fallthrough]];
                case kEnd: endOfEnd = segmentCount;
            }
        }

        // Store the last event.
        events.push_back(CompactEvent{currentY,
                                      start,
                                      endOfBegin,
                                      endOfHorizontal,
                                      endOfEnd});

        return EventQueue{std::move(events), std::move(segmentStorage)};
    }

    Event operator[](size_t i) const {
        SkASSERT(i < fEvents.size());
        auto& [y, start, endOfBegin, endOfHorizontal, endOfEnd] = fEvents[i];
        SkSpan<const Segment> begin{&fSegmentStorage[start], endOfBegin - start};
        SkSpan<const Segment>
            horizontal{&fSegmentStorage[endOfBegin], endOfHorizontal - endOfBegin};
        SkSpan<const Segment> end{&fSegmentStorage[endOfHorizontal], endOfEnd - endOfHorizontal};
        return Event{y, begin, horizontal, end};
    }

    Iterator begin() const {
        return Iterator{*this, 0};
    }

    Iterator end() const {
        return Iterator{*this, fEvents.size()};
    }

    size_t size() const {
        return fEvents.size();
    }

    bool empty() const {
        return fEvents.empty();
    }

private:
    EventQueue(std::vector<CompactEvent>&& events, std::vector<Segment>&& segmentStorage)
            : fEvents{std::move(events)}
            , fSegmentStorage{std::move(segmentStorage)} {}

    const std::vector<CompactEvent> fEvents;
    const std::vector<Segment> fSegmentStorage;
};

// -- CrossingAccumulator --------------------------------------------------------------------------
// Collect all the crossings, and reject endpoint-to-endpoint crossings as those intersections
// are already represented in the data.
class CrossingAccumulator {
public:
    void recordCrossing(const Segment& s0, const Segment& s1) {
        // Endpoints with no possible interior overlap.
        if (s0.upper() == s1.lower() || s0.lower() == s1.upper()) {
            return;
        }

        // Segments don't overlap if they are not collinear.
        if ((s0.upper() == s1.upper() || s0.lower() == s1.lower()) && compare_slopes(s0, s1) != 0) {
            return;
        }

        fCrossings.emplace_back(s0, s1);
    }

    std::vector<Crossing> finishAndReleaseCrossings() {
        return std::move(fCrossings);
    }

private:
    std::vector<Crossing> fCrossings;
};

class SweepLine {
    static constexpr Segment kLeftStatusSentinel{{INT32_MIN, INT32_MIN}, {INT32_MIN, INT32_MAX}};
    static constexpr Segment kRightStatusSentinel{{INT32_MAX, INT32_MIN}, {INT32_MAX, INT32_MAX}};

public:
    SweepLine() {
        fStatus.push_back(kLeftStatusSentinel);
        fStatus.push_back(kRightStatusSentinel);
    }

    void handleEvent(Event e) {
        auto& [y, beginnings, horizontals, endings] = e;

        // Things could be out of order from last event.
        this->sortAndRecord(y);

        this->handleBeginnings(y, beginnings);
        this->handleHorizontals(y, horizontals);
        this->handleEndings(endings);
    }

    std::vector<Crossing> finishAndReleaseCrossings() {
        // Only the sentinels should be left.
        SkASSERT(this->statusEmpty());
        return fCrossings.finishAndReleaseCrossings();
    }

private:
    using StatusLine = std::vector<Segment>;

    bool statusEmpty() const {
        return fStatus.size() == 2;
    }

    // Sort the status line, if items are swapped, then there is a crossing to record.
    void sortAndRecord(int32_t y) {
        // If there are only the sentinels or just 1 segment, then nothing to sort.
        if (fStatus.size() <= 3) {
            return;
        }

        // Skip the first and last sentinels.
        for (size_t i = 2; i < fStatus.size() - 1; ++i) {
            const Segment t = fStatus[i];
            size_t j = i;
            for (; j > 1 && s0_less_than_s1_at_y(t, fStatus[j - 1], y); --j) {
                // While t < the thing before it move it down.
                fCrossings.recordCrossing(t, fStatus[j-1]);
                fStatus[j] = fStatus[j-1];
            }
            fStatus[j] = t;
        }
    }

    // When inserting a starting point (either a beginning or a horizontal) check the segments to
    // the left and the right checking nearby segments for crossings.
    template <typename CrossingCheck>
    void checkCrossingsLeftAndRight(
            const Segment& segment, StatusLine::iterator insertionPoint, CrossingCheck check) {

        // Match to the left using the left sentinel to break the loop.
        for (auto cursor = std::make_reverse_iterator(insertionPoint); check(*cursor); ++cursor) {
            fCrossings.recordCrossing(segment, *cursor);
        }

        // Match to the right using the right sentinel to break the loop.
        for (auto cursor = insertionPoint; check(*cursor); ++cursor) {
            fCrossings.recordCrossing(segment, *cursor);
        }
    }

    // Add segments that start on y excluding horizontals.
    void handleBeginnings(int32_t y, SkSpan<const Segment> inserting) {
        for (const Segment& s : inserting) {
            auto insertionPoint =
                    std::lower_bound(fStatus.begin(), fStatus.end(), s,
                                     segment_less_than_upper_to_insert);

            // Checking intersections left and right checks if the point s.upper() lies on
            // the nearby segment.
            auto checkIntersect = [&](const Segment& toCheck) {
                return compare_point_to_segment(s.upper(), toCheck) == 0;
            };
            this->checkCrossingsLeftAndRight(s, insertionPoint, checkIntersect);

            fStatus.insert(insertionPoint, s);
        }
    }

    // Horizontals on y are handled by checking for crossings by adding them, and then immediately
    // removing them.
    void handleHorizontals(int32_t y, SkSpan<const Segment> horizontals) {
        for (const Segment& s : horizontals) {
            auto insertionPoint =
                    std::lower_bound(fStatus.begin(), fStatus.end(), s,
                                     segment_less_than_upper_to_insert);

            // Check if the nearby segment crosses the horizontal line.
            auto checkIntersection = [&](const Segment& toCheck) {
                return compare_point_to_segment(s.upper(), toCheck) <= 0 &&
                       compare_point_to_segment(s.lower(), toCheck) >= 0;
            };
            this->checkCrossingsLeftAndRight(s, insertionPoint, checkIntersection);

            fStatus.insert(insertionPoint, s);
        }

        this->handleEndings(horizontals);
    }

    // Remove all the segments ending on y.
    void handleEndings(SkSpan<const Segment> removing) {
        for (const Segment& s : removing) {
            auto removedPoint = std::remove(fStatus.begin(), fStatus.end(), s);
            SkASSERT(removedPoint != fStatus.end());
            fStatus.erase(removedPoint, fStatus.end());
        }
    }

    StatusLine fStatus;
    CrossingAccumulator fCrossings;
};

std::vector<Crossing> myers_find_crossings(const SkSpan<const Segment> segments) {
    const EventQueue eventQueue = EventQueue::Make(segments);
    SweepLine sweepLine;

    for (const Event& event : eventQueue) {
        sweepLine.handleEvent(event);
    }

    return sweepLine.finishAndReleaseCrossings();
}

// This intersection algorithm is from "Robust Plane Sweep for Intersecting Segments" page 10.
bool s0_intersects_s1(const Segment& s0, const Segment& s1) {
    // Make sure that s0 upper is above s1 upper.
    if (s1.upper().y < s0.upper().y
        || ((s1.upper().y == s0.upper().y) && (s1.lower().y > s0.lower().y))) {

        // Swap to put in the right orientation.
        return s0_intersects_s1(s1, s0);
    }

    SkASSERT(s0.upper().y <= s1.upper().y);

    {  // If extents don't overlap then there is no intersection.
        auto [left0, top0, right0, bottom0] = s0.bounds();
        auto [left1, top1, right1, bottom1] = s1.bounds();
        if (right1 < left0 || right0 < left1 || bottom1 < top0 || bottom0 < top1) {
            return false;
        }
    }

    auto [u0, l0] = s0;
    auto [u1, l1] = s1;

    const Point D0 = l0 - u0,
                D1 = l1 - u1;

    // If the vector from u0 to l0 (named D0) and the vector from u0 to u1 have an angle of 0
    // between them, then u1 is on the segment u0 to l0 (named s0).
    const Point U0toU1 = (u1 - u0);
    const int64_t D0xU0toU1 = cross(D0, U0toU1);
    if (D0xU0toU1 == 0) {
        // u1 is on s0.
        return true;
    }

    if (l1.y <= l0.y) {
        // S1 is between the upper and lower points of S0.
        const Point U0toL1 = (l1 - u0);
        const int64_t D0xU0toL1 = cross(D0, U0toL1);
        if (D0xU0toL1 == 0) {
            // l1 is on s0.
            return true;
        }

        // If U1 and L1 are on opposite sides of D0 then the segments cross.
        return (D0xU0toU1 ^ D0xU0toL1) < 0;
    } else {
        // S1 extends past S0. It could be that S1 crosses the line of S0 (not the bound segment)
        // beyond the endpoints of S0. Make sure that it crosses on the segment and not beyond.
        const Point U1toL0 = (l0 - u1);
        const int64_t D1xU1toL0 = cross(D1, U1toL0);
        if (D1xU1toL0 == 0) {
            return true;
        }

        // For D1 to cross D0, then D1 must be on the same side of U1toL0 as D0. D0xU0toU1
        // describes the orientation of U0 compared to D0. The angle from D1 to U1toL0 must
        // have the same direction as the angle from U0toU1 to D0.
        return (D0xU0toU1 ^ D1xU1toL0) >= 0;
    }
}

std::vector<Crossing> brute_force_crossings(SkSpan<Segment> segments) {

    auto isNonZeroSegment = [](const Segment& segment) {
        return segment.upper() != segment.lower();
    };
    const auto zeroSegments = std::partition(segments.begin(), segments.end(), isNonZeroSegment);

    std::sort(segments.begin(), zeroSegments);

    const auto duplicateSegments = std::unique(segments.begin(), zeroSegments);

    SkSpan<const Segment> cleanSegments =
            SkSpan{segments.data(), std::distance(segments.begin(), duplicateSegments)};

    CrossingAccumulator crossings;
    if (cleanSegments.size() >= 2) {
        for (auto i = cleanSegments.begin(); i != std::prev(cleanSegments.end()); ++i) {
            for (auto j = std::next(i); j != cleanSegments.end(); ++j) {
                if (s0_intersects_s1(*i, *j)) {
                    crossings.recordCrossing(*i, *j);
                }
            }
        }
    }
    return crossings.finishAndReleaseCrossings();
}
}  // namespace myers
