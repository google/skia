// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SweepLine_DEFINED
#define SweepLine_DEFINED

#include "modules/bentleyottmann/include/EventQueueInterface.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <cstdint>
#include <vector>

namespace bentleyottmann {
struct Point;

class SweepLine : public SweepLineInterface {
public:
    SweepLine();

    void handleDeletions(Point eventPoint, const DeletionSegmentSet& removing) override;

    void handleInsertionsAndCheckForNewCrossings(Point eventPoint,
                                                 const InsertionSegmentSet& inserting,
                                                 EventQueueInterface* queue) override;

private:
    friend struct SweepLineTestingPeer;

    void verify(int32_t y) const;

    std::vector<Segment> fSweepLine;
};
}  // namespace bentleyottmann
#endif  // SweepLine_DEFINED
