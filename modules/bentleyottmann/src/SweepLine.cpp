// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "include/private/base/SkAssert.h"
#include "modules/bentleyottmann/include/EventQueueInterface.h"
#include "modules/bentleyottmann/include/Point.h"
#include <algorithm>
#include <iterator>
#include <limits>
#include <optional>
#include <set>


namespace bentleyottmann {

SweepLine::SweepLine() {
    static constexpr Segment LeftSentinel = {{-std::numeric_limits<int32_t>::max(),
                                                     -std::numeric_limits<int32_t>::max()},
                                             {-std::numeric_limits<int32_t>::max(),
                                                     std::numeric_limits<int32_t>::max()}};
    static constexpr Segment RightSentinel = {{std::numeric_limits<int32_t>::max(),
                                                      -std::numeric_limits<int32_t>::max()},
                                              {std::numeric_limits<int32_t>::max(),
                                                      std::numeric_limits<int32_t>::max()}};

    fSweepLine.reserve(8);
    fSweepLine.push_back(LeftSentinel);
    fSweepLine.push_back(RightSentinel);
}

void SweepLine::verify(int32_t y) const {
    for(auto cursor = fSweepLine.begin(); (cursor + 1) != fSweepLine.end(); ++cursor) {
        [[maybe_unused]] const Segment& left = *cursor;
        [[maybe_unused]] const Segment& right = *(cursor + 1);
        SkASSERT(less_than_at(left, right, y));
    }
}

void SweepLine::handleDeletions(Point eventPoint, const DeletionSegmentSet& removing) {
    std::vector<Segment>::iterator newEnd;
    if (removing.empty()) {
        // Remove ending segments
        auto toRemove = [eventPoint](Segment s) {
            return s.lower() == eventPoint;
        };
        newEnd = std::remove_if(fSweepLine.begin(), fSweepLine.end(), toRemove);
    } else {
        // Remove all ending and crossing segments.
        auto toRemove = [eventPoint, &removing](Segment s) {
            return s.lower() == eventPoint || removing.find(s) != removing.end();
        };
        newEnd = std::remove_if(fSweepLine.begin(), fSweepLine.end(), toRemove);
    }
    fSweepLine.erase(newEnd, fSweepLine.end());
}

void SweepLine::handleInsertionsAndCheckForNewCrossings(
        Point eventPoint, const InsertionSegmentSet& inserting, EventQueueInterface* queue) {
    // The SlopeSegmentSet makes sure that these segments are in the right order for insertion.
    auto comp = [](const Segment& s, Point p) {
        return !point_less_than_segment_in_x(p, s);
    };

    const auto rightOfInsertion = std::lower_bound(
            fSweepLine.begin(), fSweepLine.end(), eventPoint, comp);
    SkASSERT(rightOfInsertion != fSweepLine.begin());
    const auto leftOfInsertion = rightOfInsertion - 1;

    if (inserting.empty()) {
        // There were deletions, but no insertions, so check if the two segments at the insertion
        // point cross.
        if (auto crossingPoint = intersect(*leftOfInsertion, *rightOfInsertion)) {
            queue->addCrossing(crossingPoint.value(), *leftOfInsertion, *rightOfInsertion);
        }
    } else {
        // Check if the left most inserted segment crosses the segment immediately to the left of
        // the insertion cursor.
        if (auto crossingPoint = intersect(*leftOfInsertion, *inserting.begin())) {
            queue->addCrossing(crossingPoint.value(), *leftOfInsertion, *inserting.begin());
        }

        // Check if the right most inserted segment crosses the segment immediately to the right of
        // the insertion cursor.
        if (auto crossingPoint = intersect(*inserting.rbegin(), *rightOfInsertion)) {
            queue->addCrossing(crossingPoint.value(), *inserting.rbegin(), *rightOfInsertion);
        }

        // Insert the set in the sweep line.
        fSweepLine.insert(rightOfInsertion, inserting.begin(), inserting.end());
    }
}
}  // namespace bentleyottmann
