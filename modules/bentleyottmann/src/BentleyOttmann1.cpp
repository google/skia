// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BentleyOttmann1.h"

#include "modules/bentleyottmann/include/EventQueue.h"

#include <optional>
#include <vector>

namespace bentleyottmann {

class SweepLine : public SweepLineInterface {
public:
    void handleDeletions(Point eventPoint, const DeletionSegmentSet& removing) override {}

    void
    handleInsertionsAndCheckForNewCrossings(Point eventPoint, const InsertionSegmentSet& inserting,
                                            EventQueueInterface* queue) override {}
};

std::optional<std::vector<Crossing>> bentley_ottmann_1(SkSpan<const Segment> segments)  {
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
