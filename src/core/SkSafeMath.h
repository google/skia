/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSafeMath_DEFINED
#define SkSafeMath_DEFINED

// SkSafeMath is a little helper that lets you do your normal math,
// and then check once at the end for any overflow issues.

class SkSafeMath {
public:
    SkSafeMath() = default;

    bool ok() const { return fOK; }
    explicit operator bool() const { return fOK; }

    // ok() will always return true on some platforms.
    // In general you shouldn't need to call this; it's mostly for testing.
    static bool Unchecked() {
    #if defined(__clang__)
        return false;
    #else
        return true;
    #endif
    }

    template <typename X, typename Y>
    auto mul(X x, Y y) -> decltype(x*y) {
    #if defined(__clang__)
        decltype(x*y) result;
        if (__builtin_mul_overflow(x,y, &result)) {
            fOK = false;
        }
        return result;
    #else
        return x*y;
    #endif
    }

    template <typename X, typename Y>
    auto add(X x, Y y) -> decltype(x+y) {
    #if defined(__clang__)
        decltype(x+y) result;
        if (__builtin_add_overflow(x,y, &result)) {
            fOK = false;
        }
        return result;
    #else
        return x+y;
    #endif
    }

private:
    bool fOK = true;
};

#endif//SkSafeMath_DEFINED
