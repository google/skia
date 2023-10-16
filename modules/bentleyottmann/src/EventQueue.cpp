// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/EventQueue.h"

namespace bentleyottmann {

// -- EventQueue -----------------------------------------------------------------------------------
std::optional<EventQueue> EventQueue::Make(SkSpan<const Segment> segments) {
    Queue queue;

    int32_t left   = Point::Largest().x,
            top    = Point::Largest().y,
            right  = Point::Smallest().x,
            bottom = Point::Smallest().y;

    for(const Segment& s : segments) {
        auto [l, t, r, b] = s.bounds();
        left   = std::min(l, left);
        top    = std::min(t, top);
        right  = std::max(r, right);
        bottom = std::max(b, bottom);

        queue.insert(Event{s.upper(), Upper{s}});
    }

    // If min max difference is too large, fail.
    if (Point::DifferenceTooBig(Point{left, top}, Point{right, bottom})) {
        return std::nullopt;
    }

    return EventQueue{std::move(queue)};
}

EventQueue::EventQueue(EventQueue::Queue&& queue) : fQueue{std::move(queue)} { }

void EventQueue::add(const Event& event) {
    // New events must be up stream from the current event.
    SkASSERT(fLastEventPoint < event.where);

    fQueue.insert(event);
}

void EventQueue::addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) {
    this->add({crossingPoint, Cross{s0, s1}});
    fCrossings.push_back({s0, s1, crossingPoint});
}

bool EventQueue::hasMoreEvents() const {
    return !fQueue.empty();
}

template<class... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

void EventQueue::handleNextEventPoint(SweepLineInterface* handler) {
    SkASSERT(!fQueue.empty());

    // Clear temp segment buffers.
    fDeletionSet.clear();
    fInsertionSet.clear();

    // An events that are Lower points.
    bool hasLower = false;

    // Set up the visitors for the different event types.
    auto handleLower = [&hasLower](const Lower& l) {
        hasLower = true;
    };

    // Crossing Segments must be deleted and re-inserted in the sweep line.
    auto handleCross = [this](const Cross& c) {
        fDeletionSet.insert({c.s0, c.s1});
        fInsertionSet.insert({c.s0, c.s1});
    };

    // Upper events are added to the sweep line, and a lower event is added to the event queue.
    auto handleUpper = [this](const Upper& u) {
        fInsertionSet.insert(u.s);
        // Add the delete event for the inserted segment. Make sure we are not adding more events
        // on this eventPoint.
        SkASSERT(u.s.lower() != u.s.upper());
        this->add(Event{u.s.lower(), Lower{}});
    };

    Visitor visitor{handleLower, handleCross, handleUpper};

    const Point eventPoint = fQueue.begin()->where;

    // We must make forward progress.
    SkASSERT(fLastEventPoint < eventPoint);
    fLastEventPoint = eventPoint;

    // Accumulate changes for all events with the same event point.
    auto cursor = fQueue.begin();
    const auto queueEnd = fQueue.end();
    for (; cursor != queueEnd && cursor->where == eventPoint;
         ++cursor) {
        const Event event = *cursor;
        std::visit(visitor, event.type);
    }

    // Remove all accumulated events with the same event point.
    fQueue.erase(fQueue.begin(), cursor);

    if (hasLower || !fDeletionSet.empty()) {
        // There are segments to delete.
        handler->handleDeletions(eventPoint, fDeletionSet);
    }

    if (hasLower || !fDeletionSet.empty() || !fInsertionSet.empty()) {
        // If there are insertions then insert them. If there are no insertions, but there were
        // deletions we need to check for new crossings.
        handler->handleInsertionsAndCheckForNewCrossings(eventPoint, fInsertionSet, this);
    }
}

std::vector<Crossing> EventQueue::crossings() {
    return std::vector<Crossing>{fCrossings.begin(), fCrossings.end()};
}

bool OrderBySlope::operator()(const bentleyottmann::Segment& s0,
                              const bentleyottmann::Segment& s1) const {
    return compareSlopes(s0, s1) < 0;
}
}  // namespace bentleyottmann
