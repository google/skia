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

#include "experimental/lowp-basic/QMath.h"

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


static int16_t bilerp_1(float tx, float ty, int16_t p00, int16_t p10, int16_t p01, int16_t p11) {
    const int logPixelScale = 7;
    const int16_t half = 1 << logPixelScale;
    I16 qtx = floor(tx * 65536.0f - 32768.0f + 0.5f);
    I16 qw = (p10 - p00) << logPixelScale;
    U16 qm = (p10 + p00) << logPixelScale;
    I16 top = (I16)((U16)(constrained_add(simulate_ssse3_mm_mulhrs_epi16(qtx, qw), qm) + 1) >> 1);

    qw = (p11 - p01) << logPixelScale;
    qm = (p11 + p01) << logPixelScale;
    I16 bottom =
            (I16)((U16)(constrained_add(simulate_ssse3_mm_mulhrs_epi16(qtx, qw), qm) + 1) >> 1);

    I16 qty = floor(ty * 65536.0f - 32768.0f + 0.5f);

    qw = bottom - top;
    qm = (U16)bottom + (U16)top;
    U16 scaledAnswer = constrained_add(simulate_ssse3_mm_mulhrs_epi16(qty, qw), qm);

    return (scaledAnswer[0] + half) >> (logPixelScale + 1);
}

template <typename Bilerp>
static Stats check_bilerp(Bilerp bilerp) {
    Stats stats;
    const int step = 1;
    auto interesting = {0, 1, 2, 3, 4, 5, 6, 7, 8, 60, 61, 62, 63, 64, 65, 66, 67, 68, 124, 125,
                        126, 127, 128, 129, 130, 131, 132, 188, 189, 190, 191, 192, 193, 194,
                        195, 196, 248, 249, 250, 251, 252, 253, 254, 255};
    for (float tx : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f - 1.0f/65536.0f})
    for (float ty : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f - 1.0f/65536.0f})
    for (int p00 : interesting)
    for (int p01 : interesting)
    for (int p10 : interesting)
    for (int p11 : interesting) {
        // Having this be double causes the proper rounding.
        double l = golden_bilerp2(tx, ty, p00, p10, p01, p11);
        int16_t golden = floor(l + 0.5);
        //l = golden_bilerp(tx, ty, p00, p10, p01, p11);
        //int16_t golden2 = floor(l + 0.5f);
        int16_t candidate = bilerp(tx, ty, p00, p10, p01, p11);
        stats.log(golden, candidate);
    }
    return stats;
}

int main() {
    Stats stats;

    printf("\nUsing trunc_bilerp...\n");
    stats = check_bilerp(bilerp_1);
    stats.print();

    printf("\nUsing full_res_bilerp...\n");
    stats = check_bilerp(full_res_bilerp);
    stats.print();

    printf("Done.\n");
    return 0;
}
