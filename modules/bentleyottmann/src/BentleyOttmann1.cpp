// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BentleyOttmann1.h"

#include "modules/bentleyottmann/include/EventQueue.h"

#include <optional>
#include <vector>

namespace bentleyottmann {

class SweepLine {
public:
    void handleEvent(const Event& event, EventQueue* eventQueue) {}
};

std::optional<std::vector<Point>> bentley_ottmann_1(SkSpan<const Segment> segments) {
    if (auto possibleEQ = EventQueue::Make(segments)) {
        EventQueue eventQueue = std::move(possibleEQ.value());
        SweepLine sweepLine;
        while(eventQueue.hasMoreEvents()) {
            Event event = eventQueue.nextEvent();
            sweepLine.handleEvent(event, &eventQueue);
        }

    }
    return std::nullopt;
}
}  // namespace bentleyottmann
