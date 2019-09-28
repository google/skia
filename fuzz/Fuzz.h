/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Fuzz_DEFINED
#define Fuzz_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRegion.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "tools/Registry.h"

#include <limits>
#include <cmath>
#include <signal.h>
#include <limits>

class Fuzz : SkNoncopyable {
public:
    explicit Fuzz(sk_sp<SkData> bytes) : fBytes(bytes), fNextByte(0) {}

    // Returns the total number of "random" bytes available.
    size_t size() { return fBytes->size(); }
    // Returns if there are no bytes remaining for fuzzing.
    bool exhausted() {
        return fBytes->size() == fNextByte;
    }

    size_t remaining() {
        return fBytes->size() - fNextByte;
    }

    void deplete() {
        fNextByte = fBytes->size();
    }

    // next() loads fuzzed bytes into the variable passed in by pointer.
    // We use this approach instead of T next() because different compilers
    // evaluate function parameters in different orders. If fuzz->next()
    // returned 5 and then 7, foo(fuzz->next(), fuzz->next()) would be
    // foo(5, 7) when compiled on GCC and foo(7, 5) when compiled on Clang.
    // By requiring params to be passed in, we avoid the temptation to call
    // next() in a way that does not consume fuzzed bytes in a single
    // platform-independent order.
    template <typename T>
    void next(T* t) { this->nextBytes(t, sizeof(T)); }

    // This is a convenient way to initialize more than one argument at a time.
    template <typename Arg, typename... Args>
    void next(Arg* first, Args... rest);

    // nextRange returns values only in [min, max].
    template <typename T, typename Min, typename Max>
    void nextRange(T*, Min, Max);

    // nextEnum is a wrapper around nextRange for enums.
    template <typename T>
    void nextEnum(T* ptr, T max);

    // nextN loads n * sizeof(T) bytes into ptr
    template <typename T>
    void nextN(T* ptr, int n);

    void signalBug(){
        // Tell the fuzzer that these inputs found a bug.
        SkDebugf("Signal bug\n");
        raise(SIGSEGV);
    }

    // Specialized versions for when true random doesn't quite make sense
    void next(bool* b);
    void next(SkImageFilter::CropRect* cropRect);
    void next(SkRegion* region);

    void nextRange(float* f, float min, float max);

private:
    template <typename T>
    T nextT();

    sk_sp<SkData> fBytes;
    size_t fNextByte;
    friend void fuzz__MakeEncoderCorpus(Fuzz*);

    void nextBytes(void* ptr, size_t size);
};

template <typename Arg, typename... Args>
inline void Fuzz::next(Arg* first, Args... rest) {
   this->next(first);
   this->next(rest...);
}

template <typename T, typename Min, typename Max>
inline void Fuzz::nextRange(T* value, Min min, Max max) {
    this->next(value);
    if (*value < (T)min) { *value = (T)min; }
    if (*value > (T)max) { *value = (T)max; }
}

template <typename T>
inline void Fuzz::nextEnum(T* value, T max) {
    // This works around the fact that UBSAN will assert if we put an invalid
    // value into an enum. We might see issues with enums being represented
    // on Windows differently than Linux, but that's not a thing we can fix here.
    using U = typename std::underlying_type<T>::type;
    U v;
    this->next(&v);
    if (v < (U)0) { *value = (T)0; return;}
    if (v > (U)max) { *value = (T)max; return;}
    *value = (T)v;
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

// Not static so that we can link these into oss-fuzz harnesses if we like.
#define DEF_FUZZ(name, f)                                               \
    void fuzz_##name(Fuzz*);                                            \
    sk_tools::Registry<Fuzzable> register_##name({#name, fuzz_##name}); \
    void fuzz_##name(Fuzz* f)

#endif//Fuzz_DEFINED
