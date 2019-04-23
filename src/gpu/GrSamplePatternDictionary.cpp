/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSamplePatternDictionary.h"

bool GrSamplePatternDictionary::LessThan::operator()(
        const SkTArray<SkPoint>& a, const SkTArray<SkPoint>& b) const {
    if (a.count() != b.count()) {
        return a.count() < b.count();
    }
    for (int i = 0; i < a.count(); ++i) {
        // This doesn't have geometric meaning. We just need to define an ordering for std::map.
        if (a[i].x() != b[i].x()) {
            return a[i].x() < b[i].x();
        }
        if (a[i].y() != b[i].y()) {
            return a[i].y() < b[i].y();
        }
    }
    return false;  // Both sample patterns are equal, therefore, "a < b" is false.
}

int GrSamplePatternDictionary::findOrAssignSamplePatternKey(
        const SkTArray<SkPoint>& sampleLocations) {
    if (std::numeric_limits<int>::max() == fSampleLocationsArray.count()) {
        return 0;
    }
    const auto& insertResult = fSamplePatternKeyMap.insert(
            {sampleLocations, fSampleLocationsArray.count()});
    if (insertResult.second) {
        // This means the "insert" call did not find the pattern in the key map already, and
        // therefore an actual insertion took place. (We don't expect to see many unique sample
        // patterns.)
        const SkTArray<SkPoint>& sampleLocations = insertResult.first->first;
        fSampleLocationsArray.push_back(&sampleLocations);
    }
    return insertResult.first->second;  // Return the new sample pattern key.
}
