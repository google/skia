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

class Fuzz : SkNoncopyable {
public:
    explicit Fuzz(sk_sp<SkData>);

    // Returns the total number of "random" bytes available.
    size_t size();
    // Returns the total number of "random" bytes remaining for randomness.
    size_t remaining();

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
    int fNextByte;
};

// UBSAN reminds us that bool can only legally hold 0 or 1.
template <>
inline bool Fuzz::next<bool>() {
  return (this->next<uint8_t>() & 1) == 1;
}

template <typename T>
T Fuzz::next() {
    if (fNextByte + sizeof(T) > fBytes->size()) {
        return 0;
    }
    T n;
    memcpy(&n, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
    return n;
}

template <typename T>
T Fuzz::nextRange(T min, T max) {
    if (min > max) {
        SkDebugf("Check mins and maxes (%d, %d)\n", min, max);
        this->signalBug();
    }
    T n = this->next<T>();
    // we don't care about the distribution of these, the fuzzer will learn
    // what to put in there to make it interesting.
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
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
