/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <cstddef>
#include <limits>
#include <memory>

#include "SkMask.h"

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
        return sizeof(size_t) == sizeof(uint64_t) ? mul64(x, y) : mul32(x, y);
    }

    size_t add(size_t x, size_t y) {
        size_t result = x + y;
        fOK &= result >= x;
        return result;
    }

private:
    uint32_t mul32(uint32_t x, uint32_t y) {
        uint64_t bx = x;
        uint64_t by = y;
        uint64_t result = bx * by;
        fOK &= result >> 32 == 0;
        return result;
    }

    uint64_t mul64(uint64_t x, uint64_t y) {
        if (x <= std::numeric_limits<uint64_t>::max() >> 32
            && y <= std::numeric_limits<uint64_t>::max() >> 32) {
            return x * y;
        } else {
            auto hi = [](uint64_t x) { return x >> 32; };
            auto lo = [](uint64_t x) { return x & 0xFFFFFFFF; };

            uint64_t lx_ly = lo(x) * lo(y);
            uint64_t hx_ly = hi(x) * lo(y);
            uint64_t lx_hy = lo(x) * hi(y);
            uint64_t hx_hy = hi(x) * hi(y);
            uint64_t result = 0;
            result = this->add(lx_ly, (hx_ly << 32));
            result = this->add(result, (lx_hy << 32));
            fOK &= (hx_hy + (hx_ly >> 32) + (lx_hy >> 32)) == 0;

            #if defined(SK_DEBUG) && defined(__clang__) && defined(__x86_64__)
                auto double_check = (unsigned __int128)x * y;
                SkASSERT(result == (double_check & 0xFFFFFFFFFFFFFFFF));
                SkASSERT(!fOK || (double_check >> 64 == 0));
            #endif

            return result;
        }
    }
    bool fOK = true;
};

class SkSafeSize {
public:
    template <typename V, typename = typename std::enable_if<
        std::is_convertible<V, size_t>::value>::type>
    explicit constexpr SkSafeSize(V v) : size_{CheckValue(v)} {}

    size_t size() { return size_; }

    SkSafeSize operator+(const SkSafeSize& r) const {
        size_t possibleAnswer = this->size_ + r.size_;
        if (possibleAnswer >= size_) {
            return SkSafeSize{possibleAnswer};
        }
        return Overflow();
    }

    SkSafeSize operator*(const SkSafeSize& r) const;

    template <typename T>
    SkSafeSize roundUp() {
        size_t mask = alignof(T);
        if (size_ + mask >= size_) {
            return SkSafeSize{(size_ + mask) & ~mask};
        }
        return Overflow();
    }

    bool ok() const { return size_ != kMaxSize; }
    explicit operator bool() const { return ok(); }
    template <typename T>
    operator T() const {return static_cast<T>(size_);}
    operator int() const {return static_cast<int>(size_);}
/*
    operator size_t() const {return size_;}
    operator int32_t() const {
        if (size_ > std::numeric_limits<int32_t>::max()) {
            return std::numeric_limits<int32_t>::max();
        }
        return static_cast<int32_t>(size_);
    }
    operator long() const {
        return static_cast<long>(size_);
    }
    */

    template<typename T>
    T* makeArrayFromSize() {
        return new T[size_];
    }

    template<typename T>
    T* makeArrayFromSizeInitialized() {
        return new T[size_]();
    }

    template<typename T>
    std::unique_ptr<T[]> makeUniquePtrFromSize() {
        return std::unique_ptr<T[]>(makeArrayFromSize<T[]>());
    }

private:
    static constexpr size_t kMaxSize = std::numeric_limits<size_t>::max() >> 1;
    template <typename V>
    static size_t CheckValue(V v) {
        if (SkTFitsIn<size_t>(v)) {
            return static_cast<size_t>(v);
        }
        return kMaxSize;
    }
    static SkSafeSize Overflow() { return SkSafeSize{kMaxSize}; }
    size_t size_{0};
};

inline SkSafeSize SkSafeWidth(const SkMask& mask) {
    return SkSafeSize{mask.fBounds.width()};
}

inline SkSafeSize SkSafeHeight(const SkMask& mask) {
    return SkSafeSize{mask.fBounds.height()};
}

inline SkSafeSize SkSafeRowBytes(const SkMask& mask) {
    return SkSafeSize{mask.fRowBytes};
}

#endif//SkSafeMath_DEFINED
