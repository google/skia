/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkTSort.h"
#include "tests/Test.h"

static bool anderson_darling_test(double p[32]) {
    // Min and max Anderson-Darling values allowable for k=32
    const double kADMin32 = 0.202;        // p-value of ~0.1
    const double kADMax32 = 3.89;         // p-value of ~0.99

    // sort p values
    SkTQSort<double>(p, p + 31);

    // and compute Anderson-Darling statistic to ensure these are uniform
    double s = 0.0;
    for(int k = 0; k < 32; k++) {
        double v = p[k]*(1.0 - p[31-k]);
        if (v < 1.0e-30) {
           v = 1.0e-30;
        }
        s += (2.0*(k+1)-1.0)*log(v);
    }
    double a2 = -32.0 - 0.03125*s;

    return (kADMin32 < a2 && a2 < kADMax32);
}

static bool chi_square_test(int bins[256], int e) {
    // Min and max chisquare values allowable
    const double kChiSqMin256 = 206.3179;        // probability of chance = 0.99 with k=256
    const double kChiSqMax256 = 311.5603;        // probability of chance = 0.01 with k=256

    // compute chi-square
    double chi2 = 0.0;
    for (int j = 0; j < 256; ++j) {
        double delta = bins[j] - e;
        chi2 += delta*delta/e;
    }

    return (kChiSqMin256 < chi2 && chi2 < kChiSqMax256);
}

// Approximation to the normal distribution CDF
// From Waissi and Rossin, 1996
static double normal_cdf(double z) {
    double t = ((-0.0004406*z*z* + 0.0418198)*z*z + 0.9)*z;
    t *= -1.77245385091;  // -sqrt(PI)
    double p = 1.0/(1.0 + exp(t));

    return p;
}

static void test_random_byte(skiatest::Reporter* reporter, int shift) {
    int bins[256];
    memset(bins, 0, sizeof(int)*256);

    SkRandom rand;
    for (int i = 0; i < 256*10000; ++i) {
        bins[(rand.nextU() >> shift) & 0xff]++;
    }

    REPORTER_ASSERT(reporter, chi_square_test(bins, 10000));
}

static void test_random_float(skiatest::Reporter* reporter) {
    int bins[256];
    memset(bins, 0, sizeof(int)*256);

    SkRandom rand;
    for (int i = 0; i < 256*10000; ++i) {
        float f = rand.nextF();
        REPORTER_ASSERT(reporter, 0.0f <= f && f < 1.0f);
        bins[(int)(f*256.f)]++;
    }
    REPORTER_ASSERT(reporter, chi_square_test(bins, 10000));

    double p[32];
    for (int j = 0; j < 32; ++j) {
        float f = rand.nextF();
        REPORTER_ASSERT(reporter, 0.0f <= f && f < 1.0f);
        p[j] = f;
    }
    REPORTER_ASSERT(reporter, anderson_darling_test(p));
}

// This is a test taken from tuftests by Marsaglia and Tsang. The idea here is that
// we are using the random bit generated from a single shift position to generate
// "strings" of 16 bits in length, shifting the string and adding a new bit with each
// iteration. We track the numbers generated. The ones that we don't generate will
// have a normal distribution with mean ~24108 and standard deviation ~127. By
// creating a z-score (# of deviations from the mean) for one iteration of this step
// we can determine its probability.
//
// The original test used 26 bit strings, but is somewhat slow. This version uses 16
// bits which is less rigorous but much faster to generate.
static double test_single_gorilla(skiatest::Reporter* reporter, int shift) {
    const int kWordWidth = 16;
    const double kMean = 24108.0;
    const double kStandardDeviation = 127.0;
    const int kN = (1 << kWordWidth);
    const int kNumEntries = kN >> 5;  // dividing by 32
    unsigned int entries[kNumEntries];

    SkRandom rand;
    memset(entries, 0, sizeof(unsigned int)*kNumEntries);
    // pre-seed our string value
    int value = 0;
    for (int i = 0; i < kWordWidth-1; ++i) {
        value <<= 1;
        unsigned int rnd = rand.nextU();
        value |= ((rnd >> shift) & 0x1);
    }

    // now make some strings and track them
    for (int i = 0; i < kN; ++i) {
        value = SkLeftShift(value, 1);
        unsigned int rnd = rand.nextU();
        value |= ((rnd >> shift) & 0x1);

        int index = value & (kNumEntries-1);
        SkASSERT(index < kNumEntries);
        int entry_shift = (value >> (kWordWidth-5)) & 0x1f;
        entries[index] |= (0x1 << entry_shift);
    }

    // count entries
    int total = 0;
    for (int i = 0; i < kNumEntries; ++i) {
        unsigned int entry = entries[i];
        while (entry) {
            total += (entry & 0x1);
            entry >>= 1;
        }
    }

    // convert counts to normal distribution z-score
    double z = ((kN-total)-kMean)/kStandardDeviation;

    // compute probability from normal distibution CDF
    double p = normal_cdf(z);

    REPORTER_ASSERT(reporter, 0.01 < p && p < 0.99);
    return p;
}

static void test_gorilla(skiatest::Reporter* reporter) {

    double p[32];
    for (int bit_position = 0; bit_position < 32; ++bit_position) {
        p[bit_position] = test_single_gorilla(reporter, bit_position);
    }

    REPORTER_ASSERT(reporter, anderson_darling_test(p));
}

static void test_range(skiatest::Reporter* reporter) {
    SkRandom rand;

    // just to make sure we don't crash in this case
    (void) rand.nextRangeU(0, 0xffffffff);

    // check a case to see if it's uniform
    int bins[256];
    memset(bins, 0, sizeof(int)*256);
    for (int i = 0; i < 256*10000; ++i) {
        unsigned int u = rand.nextRangeU(17, 17+255);
        REPORTER_ASSERT(reporter, 17 <= u && u <= 17+255);
        bins[u - 17]++;
    }

    REPORTER_ASSERT(reporter, chi_square_test(bins, 10000));
}

DEF_TEST(Random, reporter) {
    // check uniform distributions of each byte in 32-bit word
    test_random_byte(reporter, 0);
    test_random_byte(reporter, 8);
    test_random_byte(reporter, 16);
    test_random_byte(reporter, 24);

    test_random_float(reporter);

    test_gorilla(reporter);

    test_range(reporter);
}
