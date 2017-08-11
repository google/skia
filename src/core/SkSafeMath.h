/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSafeMath_DEFINED
#define SkSafeMath_DEFINED

// SkSafeMath always check that a series of operations do not overflow.
// This must be correct for all platforms, because this is a check for safety at runtime.

class SkSafeMath {
public:
    SkSafeMath() = default;

    bool ok() const { return fOK; }
    explicit operator bool() const { return fOK; }

    size_t mul(size_t x, size_t y) {
        if (sizeof(size_t) == sizeof(uint64_t)) {

            if (x <= std::numeric_limits<size_t>::max() >> 32
                && y <= std::numeric_limits<size_t>::max() >> 32) {
                return x * y;
            } else {
                size_t lx_ly = (x & 0xFFFFFFFF) * (y & 0xFFFFFFFF);
                size_t hx_ly = (x >> 32) * y;
                size_t lx_hy = x * (y >> 32);
                size_t hx_hy = (x >> 32) * (y >> 32);
                size_t result = this->add(lx_ly, (hx_ly << 32));
                result = this->add(result, (lx_hy << 32));
                fOK &= (hx_hy + (hx_ly >> 32) + (lx_hy >> 32)) == 0;
                return result;
            }
        } else {
            // assume sizeof(size_t) == sizeof(uint32_t)
            uint64_t bx = x;
            uint64_t by = y;
            uint64_t result = bx * by;
            fOK &= result >> 32 == 0;
            return result;
        }

    }

    size_t add(size_t x, size_t y) {
        size_t result = x + y;
        fOK &= result >= x;
        return result;
    }

private:
    bool fOK = true;
};

#endif//SkSafeMath_DEFINED
