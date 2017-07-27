/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSafeMath_DEFINED
#define SkSafeMath_DEFINED

#include <stddef.h>

// SkSafeMath is a little helper that lets you do your normal math,
// and then check once at the end for any overflow issues.

class SkSafeMath {
public:
    SkSafeMath() { this->reset(); }
    void reset() { fOK = true; }

    bool ok() const { return fOK; }
    explicit operator bool() const { return fOK; }

    size_t mul(size_t, size_t);
    size_t add(size_t, size_t);

private:
    bool fOK;
};

#endif//SkSafeMath_DEFINED
