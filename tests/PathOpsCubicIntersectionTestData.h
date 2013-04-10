/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsCubic.h"

extern const SkDCubic pointDegenerates[];
extern const SkDCubic notPointDegenerates[];
extern const SkDCubic tests[][2];
extern SkDCubic hexTests[][2];

extern const SkDCubic lines[];
extern const SkDCubic notLines[];
extern const SkDCubic modEpsilonLines[];
extern const SkDCubic lessEpsilonLines[];
extern const SkDCubic negEpsilonLines[];

extern const size_t pointDegenerates_count;
extern const size_t notPointDegenerates_count;
extern const size_t tests_count;
extern const size_t hexTests_count;
extern const size_t lines_count;
extern const size_t notLines_count;
extern const size_t modEpsilonLines_count;
extern const size_t lessEpsilonLines_count;
extern const size_t negEpsilonLines_count;
