/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SkColorSpacePrintf(...)

inline bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}
