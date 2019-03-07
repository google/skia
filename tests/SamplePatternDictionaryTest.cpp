/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "SkPoint.h"
#include "SkRandom.h"
#include "Test.h"
#include <vector>

#if SK_SUPPORT_GPU

#include "GrSamplePatternDictionary.h"

static SkTArray<SkPoint> make_sample_pattern(const std::vector<SkPoint>& sampleLocations) {
    return SkTArray<SkPoint>(sampleLocations.data(), sampleLocations.size());
}

static SkTArray<SkPoint> make_random_sample_pattern(SkRandom* rand) {
    SkTArray<SkPoint> pattern;
    int count = rand->nextULessThan(20) + 1;
    pattern.reset(count);
    for (int i = 0; i < count; ++i) {
        pattern[i] = SkPoint::Make(rand->nextF(), rand->nextF());
    }
    return pattern;
}

// This test ensures that the sample pattern dictionary caches and retrieves patterns correctly.
DEF_TEST(SamplePatternDictionary, reporter) {
    SkTArray<SkTArray<SkPoint>> testPatterns;
    testPatterns.push_back() = make_sample_pattern({ // Intel on mac, msaa8, offscreen.
        {0.562500, 0.312500},
        {0.437500, 0.687500},
        {0.812500, 0.562500},
        {0.312500, 0.187500},
        {0.187500, 0.812500},
        {0.062500, 0.437500},
        {0.687500, 0.937500},
        {0.937500, 0.062500}
    });

    testPatterns.push_back() = make_sample_pattern({ // Intel on mac, msaa8, on-screen.
        {0.562500, 0.687500},
        {0.437500, 0.312500},
        {0.812500, 0.437500},
        {0.312500, 0.812500},
        {0.187500, 0.187500},
        {0.062500, 0.562500},
        {0.687500, 0.062500},
        {0.937500, 0.937500}
    });

    testPatterns.push_back() = make_sample_pattern({ // NVIDIA, msaa16.
        {0.062500, 0.000000},
        {0.250000, 0.125000},
        {0.187500, 0.375000},
        {0.437500, 0.312500},
        {0.500000, 0.062500},
        {0.687500, 0.187500},
        {0.750000, 0.437500},
        {0.937500, 0.250000},
        {0.000000, 0.500000},
        {0.312500, 0.625000},
        {0.125000, 0.750000},
        {0.375000, 0.875000},
        {0.562500, 0.562500},
        {0.812500, 0.687500},
        {0.625000, 0.812500},
        {0.875000, 0.937500}
    });

    testPatterns.push_back() = make_sample_pattern({ // NVIDIA, mixed samples, 16:1.
        {0.250000, 0.125000},
        {0.625000, 0.812500},
        {0.500000, 0.062500},
        {0.812500, 0.687500},
        {0.187500, 0.375000},
        {0.875000, 0.937500},
        {0.125000, 0.750000},
        {0.750000, 0.437500},
        {0.937500, 0.250000},
        {0.312500, 0.625000},
        {0.437500, 0.312500},
        {0.000000, 0.500000},
        {0.375000, 0.875000},
        {0.687500, 0.187500},
        {0.062500, 0.000000},
        {0.562500, 0.562500}
    });

    SkRandom rand;
    for (int i = 0; i < 23; ++i) {
        testPatterns.push_back(make_random_sample_pattern(&rand));
    }

    // Duplicate the initial 4 patterns, with slight differences.
    testPatterns.push_back(testPatterns[0]);
    testPatterns.back().back().fX += 0.001f;

    testPatterns.push_back(testPatterns[1]);
    testPatterns.back().back().fY -= 0.002f;

    testPatterns.push_back(testPatterns[2]);
    testPatterns.back().push_back(SkPoint::Make(.5f, .5f));

    testPatterns.push_back(testPatterns[3]);
    testPatterns.back().pop_back();

    for (int i = 0; i < 13; ++i) {
        testPatterns.push_back(make_random_sample_pattern(&rand));
    }

    GrSamplePatternDictionary dict;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < testPatterns.count(); ++j) {
            for (int k = 0; k < 3; ++k) {
                const SkTArray<SkPoint>& pattern = testPatterns[testPatterns.count() - j - 1];
                REPORTER_ASSERT(reporter, j == dict.findOrAssignSamplePatternKey(pattern));
            }
        }
    }
    for (int j = 0; j < testPatterns.count(); ++j) {
        const SkTArray<SkPoint>& pattern = testPatterns[testPatterns.count() - j - 1];
        REPORTER_ASSERT(reporter, dict.retrieveSampleLocations(j) == pattern);
    }
}

#endif
