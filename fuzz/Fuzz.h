/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Fuzz_DEFINED
#define Fuzz_DEFINED

#include "SkData.h"
#include "SkTRegistry.h"
#include "SkTypes.h"

#include <cmath>

class Fuzz : SkNoncopyable {
public:
    explicit Fuzz(sk_sp<SkData>);

    // Returns the total number of "random" bytes available.
    size_t size();
    // Returns if there are no bytes remaining for fuzzing.
    bool exhausted();

    template <typename T>
    T next();

    // nextRange returns values only in [min, max].
    template <typename T>
    T nextRange(T min, T max);

    void signalBug();  // Tell afl-fuzz these inputs found a bug.

private:
    template <typename T>
    T nextT();

    sk_sp<SkData> fBytes;
    size_t fNextByte;
};

// UBSAN reminds us that bool can only legally hold 0 or 1.
template <>
inline bool Fuzz::next<bool>() {
  return (this->next<uint8_t>() & 1) == 1;
}

template <typename T>
T Fuzz::next() {
    if ((fNextByte + sizeof(T)) > fBytes->size()) {
        T n = 0;
        memcpy(&n, fBytes->bytes() + fNextByte, fBytes->size() - fNextByte);
        fNextByte = fBytes->size();
        return n;
    }
    T n;
    memcpy(&n, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
    return n;
}

template <>
inline float Fuzz::nextRange(float min, float max) {
    if (min > max) {
        SkDebugf("Check mins and maxes (%f, %f)\n", min, max);
        this->signalBug();
    }
    float f = this->next<float>();
    if (!std::isnormal(f) && f != 0.0f) {
        // Don't deal with infinity or other strange floats.
        return max;
    }
    return min + std::fmod(std::abs(f), (max - min + 1));
}

template <typename T>
T Fuzz::nextRange(T min, T max) {
    if (min > max) {
        SkDebugf("Check mins and maxes (%d, %d)\n", min, max);
        this->signalBug();
    }
    T n = this->next<T>();
    T range = max - min + 1;
    if (0 == range) {
        return n;
    } else {
        n = abs(n);
        if (n < 0) {
          // abs(INT_MIN) = INT_MIN, so we check this to avoid accidental negatives.
          return min;
        }
        return min + n % range;
    }
}

struct Fuzzable {
    const char* name;
    void (*fn)(Fuzz*);
};

#define DEF_FUZZ(name, f)                                        \
    static void fuzz_##name(Fuzz*);                              \
    SkTRegistry<Fuzzable> register_##name({#name, fuzz_##name}); \
    static void fuzz_##name(Fuzz* f)

#endif//Fuzz_DEFINED
