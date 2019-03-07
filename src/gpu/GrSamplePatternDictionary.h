/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSamplePatternDictionary_DEFINED
#define GrSamplePatternDictionary_DEFINED

#include "SkPoint.h"
#include "SkTArray.h"
#include <map>

/**
 * A bidirectional dictionary mapping between sample patterns (i.e., a list of sample locations) and
 * unique keys. Since we expect that most render targets will draw from the same small pool of
 * sample patterns, we favor sample pattern keys over actual arrays of points.
 */
class GrSamplePatternDictionary {
public:
    static constexpr int kInvalidSamplePatternKey = -1;

    int findOrAssignSamplePatternKey(const SkTArray<SkPoint>& sampleLocations);

    const SkTArray<SkPoint>& retrieveSampleLocations(int samplePatternKey) const {
        return *fSampleLocationsArray[samplePatternKey];
    }

private:
    struct LessThan {
        bool operator()(const SkTArray<SkPoint>&, const SkTArray<SkPoint>&) const;
    };

    std::map<SkTArray<SkPoint>, int, LessThan> fSamplePatternKeyMap;
    SkTArray<const SkTArray<SkPoint>*> fSampleLocationsArray;
};

#endif
