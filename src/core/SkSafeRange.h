/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSafeRange_DEFINED
#define SkSafeRange_DEFINED

#include "include/core/SkTypes.h"

#include <cstdint>

// SkSafeRange always check that a series of operations are in-range.
// This check is sticky, so that if any one operation fails, the object will remember that and
// return false from ok().

class SkSafeRange {
public:
    explicit operator bool() const { return fOK; }

    bool ok() const { return fOK; }

    // checks 0 <= value <= max.
    // On success, returns value
    // On failure, returns 0 and sets ok() to false
    template <typename T> T checkLE(uint64_t value, T max) {
        SkASSERT(static_cast<int64_t>(max) >= 0);
        if (value > static_cast<uint64_t>(max)) {
            fOK = false;
            value = 0;
        }
        return static_cast<T>(value);
    }

    int checkGE(int value, int min) {
        if (value < min) {
            fOK = false;
            value = min;
        }
        return value;
    }

private:
    bool fOK = true;
};

#endif
