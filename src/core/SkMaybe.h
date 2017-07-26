/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaybe_DEFINED
#define SkMaybe_DEFINED

#include "SkTypes.h"

// SkMaybe<T> is a T if possible, or maybe not.
// This is primarily targeted at making math that might over or underflow painlessly safe.

template <typename T>
class SkMaybe;

// Just to get started here's just one specialization of SkMaybe<uint32_t>.
// It's possible some of this will generalize as we add more specializations.

static inline SkMaybe<uint32_t> sk_maybe_add(uint32_t x, uint32_t y);
static inline SkMaybe<uint32_t> sk_maybe_mul(uint32_t x, uint32_t y);

template <>
class SkMaybe<uint32_t> {
public:
    SkMaybe()                          = default;
    SkMaybe(const SkMaybe&)            = default;
    SkMaybe(SkMaybe&&)                 = default;
    SkMaybe& operator=(const SkMaybe&) = default;
    SkMaybe& operator=(SkMaybe&&)      = default;

    /*implicit*/
    SkMaybe(uint32_t value) : fValue(value), fIsNaN(false) {}

    static SkMaybe NaN() {
        SkMaybe v;
        v.fIsNaN = true;
        return v;
    }

    bool isNaN() const { return fIsNaN; }

    bool get(uint32_t* value) const {
        *value = fValue;
        return !fIsNaN;
    }

    SkMaybe operator+(SkMaybe o) {
        uint32_t x,y;
        if (this->get(&x) && o.get(&y)) {
            return sk_maybe_add(x,y);
        }
        return SkMaybe<uint32_t>::NaN();
    }

    SkMaybe operator*(SkMaybe o) {
        uint32_t x,y;
        if (this->get(&x) && o.get(&y)) {
            return sk_maybe_mul(x,y);
        }
        return SkMaybe<uint32_t>::NaN();
    }

private:
    uint32_t fValue = 0;
    bool     fIsNaN = false;
};

static inline SkMaybe<uint32_t> sk_maybe_add(uint32_t x, uint32_t y) {
#if defined(__GNUC__)
    uint32_t result;
    if (!__builtin_add_overflow(x, y, &result)) {
        return result;
    }
#else
    uint64_t X = x, Y = y,
             R = X+Y;
    if (R <= ~uint32_t(0)) {
        return R;
    }
#endif
    return SkMaybe<uint32_t>::NaN();
}

static inline SkMaybe<uint32_t> sk_maybe_mul(uint32_t x, uint32_t y) {
#if defined(__GNUC__)
    uint32_t result;
    if (!__builtin_mul_overflow(x, y, &result)) {
        return result;
    }
#else
    uint64_t X = x, Y = y,
             R = X*Y;
    if (R <= ~uint32_t(0)) {
        return R;
    }
#endif
    return SkMaybe<uint32_t>::NaN();
}

#endif//SkMaybe_DEFINED
