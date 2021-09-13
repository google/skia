/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>

#include "QMath.h"

struct Stats {
    int64_t diff_8_bits = 0;
    int64_t max_diff = 0;
    int64_t min_diff = 0;
    int64_t total = 0;

    void log(int16_t golden, int16_t candidate) {
        int64_t diff = candidate - golden;
        max_diff = std::max(max_diff, diff);
        min_diff = std::min(min_diff, diff);
        diff_8_bits += candidate != golden;
        total++;
    }

    void print() const {
        printf("8-bit diff: %lld - %g%%\n", diff_8_bits, 100.0 * diff_8_bits / total);
        printf("differences min: %lld max: %lld\n", min_diff, max_diff);
        printf("total: %lld\n", total);
    }
};

// This has all kinds of rounding issues.
// TODO(herb): figure out rounding problems with this code.
static float golden_bilerp(float tx, float ty, int16_t p00, int16_t p10, int16_t p01, int16_t p11) {
    return (1.0f-tx) * (1.0f-ty) * p00
         + (1.0f-tx) * ty * p01
         + (1.0f-ty) * tx * p10
         + tx * ty * p11;
}

static double golden_bilerp2(
        float tx, float ty, int16_t p00, int16_t p10, int16_t p01, int16_t p11) {
    // Double is needed to avoid rounding of lower bits.
    double dtx(tx), dty(ty);

    double top = (1.0 - dtx) * p00 + dtx * p10;
    double bottom = (1.0 - dtx) * p01 + dtx * p11;

    return (1.0 - dty) * top + dty * bottom;
}

static int16_t full_res_bilerp(
        float tx, float ty, int16_t p00, int16_t p10, int16_t p01, int16_t p11) {
    int32_t ftx(floor(tx * 65536.0f + 0.5f));
    int64_t top = ftx * (p10 - p00) + 65536 * p00;
    int64_t bottom = ftx * (p11 - p01) + 65536 * p01;

    int64_t fty(floor(ty * 65536.0f + 0.5f));
    int64_t temp = fty * (bottom - top) + top * 65536LL;
    int64_t rounded = temp + (1LL << 31);
    return rounded >> 32;
}

template <typename Bilerp>
static Stats check_bilerp(Bilerp bilerp) {
    Stats stats;
    const int step = 1;
    for (float tx : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f - 1.0f/65536.0f})
    for (float ty : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f - 1.0f/65536.0f})
    for (int p00 = 0; p00 < 256; p00 += step)
    for (int p01 = 0; p01 < 256; p01 += step)
    for (int p10 = 0; p10 < 256; p10 += step)
    for (int p11 = 0; p11 < 256; p11 += step) {
        // Having this be double causes the proper rounding.
        double l = golden_bilerp2(tx, ty, p00, p10, p01, p11);
        int16_t golden = floor(l + 0.5);
        l = golden_bilerp(tx, ty, p00, p10, p01, p11);
        //int16_t golden2 = floor(l + 0.5f);
        int16_t candidate = bilerp(tx, ty, p00, p10, p01, p11);
        stats.log(golden, candidate);
    }
    return stats;
}

int main() {
    Stats stats;

    printf("\nUsing full_res_bilerp...\n");
    stats = check_bilerp(full_res_bilerp);
    stats.print();

    printf("Done.\n");
    return 0;
}
