/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Fuzz_DEFINED
#define Fuzz_DEFINED

#include "SkData.h"
#include "../tools/Registry.h"
#include "SkMalloc.h"
#include "SkTypes.h"

#include <cmath>

class Fuzz : SkNoncopyable {
public:
    explicit Fuzz(sk_sp<SkData>);

    // Returns the total number of "random" bytes available.
    size_t size();
    // Returns if there are no bytes remaining for fuzzing.
    bool exhausted();

    // next() loads fuzzed bytes into the variable passed in by pointer.
    // We use this approach instead of T next() because different compilers
    // evaluate function parameters in different orders. If fuzz->next()
    // returned 5 and then 7, foo(fuzz->next(), fuzz->next()) would be
    // foo(5, 7) when compiled on GCC and foo(7, 5) when compiled on Clang.
    // By requiring params to be passed in, we avoid the temptation to call
    // next() in a way that does not consume fuzzed bytes in a single
    // uplatform-independent order.
    template <typename T>
    void next(T* t);

    // This is a convenient way to initialize more than one argument at a time.
    template <typename Arg, typename... Args>
    void next(Arg* first, Args... rest);

    // nextRange returns values only in [min, max].
    template <typename T, typename Min, typename Max>
    void nextRange(T*, Min, Max);

    // nextN loads n * sizeof(T) bytes into ptr
    template <typename T>
    void nextN(T* ptr, int n);

    void signalBug();  // Tell afl-fuzz these inputs found a bug.

private:
    template <typename T>
    T nextT();

    sk_sp<SkData> fBytes;
    size_t fNextByte;
};

// UBSAN reminds us that bool can only legally hold 0 or 1.
template <>
inline void Fuzz::next(bool* b) {
  uint8_t n;
  this->next(&n);
  *b = (n & 1) == 1;
}

template <typename T>
inline void Fuzz::next(T* n) {
    if ((fNextByte + sizeof(T)) > fBytes->size()) {
        sk_bzero(n, sizeof(T));
        memcpy(n, fBytes->bytes() + fNextByte, fBytes->size() - fNextByte);
        fNextByte = fBytes->size();
        return;
    }
    memcpy(n, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
}

template <typename Arg, typename... Args>
inline void Fuzz::next(Arg* first, Args... rest) {
   this->next(first);
   this->next(rest...);
}

template <>
inline void Fuzz::nextRange(float* f, float min, float max) {
    this->next(f);
    if (!std::isnormal(*f) && *f != 0.0f) {
        // Don't deal with infinity or other strange floats.
        *f = max;
    }
    *f = min + std::fmod(std::abs(*f), (max - min + 1));
}

template <typename T, typename Min, typename Max>
inline void Fuzz::nextRange(T* n, Min min, Max max) {
    this->next<T>(n);
    if (min == max) {
        *n = min;
        return;
    }
    if (min > max) {
        // Avoid misuse of nextRange
        this->signalBug();
    }
    if (*n < 0) { // Handle negatives
        if (*n != std::numeric_limits<T>::lowest()) {
            *n *= -1;
        }
        else {
            *n = std::numeric_limits<T>::max();
        }
    }
    *n = min + (*n % ((size_t)max - min + 1));
}

template <typename T>
inline void Fuzz::nextN(T* ptr, int n) {
   for (int i = 0; i < n; i++) {
       this->next(ptr+i);
   }
}

struct Fuzzable {
    const char* name;
    void (*fn)(Fuzz*);
};

#define DEF_FUZZ(name, f)                                               \
    static void fuzz_##name(Fuzz*);                                     \
    sk_tools::Registry<Fuzzable> register_##name({#name, fuzz_##name}); \
    static void fuzz_##name(Fuzz* f)

#endif//Fuzz_DEFINED
