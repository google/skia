// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BentleyOttmann1.h"

#include "modules/bentleyottmann/include/EventQueue.h"
#include "modules/bentleyottmann/include/Segment.h"
#include "modules/bentleyottmann/include/SweepLine.h"

#include <optional>
#include <utility>
#include <vector>

namespace bentleyottmann {

std::optional<std::vector<Crossing>> bentley_ottmann_1(SkSpan<const Segment> segments) {
    if (auto possibleEQ = EventQueue::Make(segments)) {
        EventQueue eventQueue = std::move(possibleEQ.value());
        SweepLine sweepLine;
        while(eventQueue.hasMoreEvents()) {
            eventQueue.handleNextEventPoint(&sweepLine);
        }
        return eventQueue.crossings();
    }
    return std::nullopt;
}
}  // namespace bentleyottmann
