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
    SkASSERT(!fQueue.empty() && fQueue.begin()->where < event.where);
    auto [_, success] = fQueue.insert(event);
    SkASSERT_RELEASE(success);
}

bool EventQueue::hasMoreEvents() const {
    return !fQueue.empty();
}

Event EventQueue::nextEvent() {
    SkASSERT(this->hasMoreEvents());

    auto firstElement = fQueue.begin();

    // Extract event at the beginning of the queue.
    Event event = *firstElement;

    // Remove the beginning element from the queue.
    fQueue.erase(firstElement);

    return event;
}
}  // namespace bentleyottmann
