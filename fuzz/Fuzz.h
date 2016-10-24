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
    bool next(T* n);

    // UBSAN reminds us that bool can only legally hold 0 or 1.
    bool next(bool* b) {
        uint8_t byte;
        if (!this->next(&byte)) {
            return false;
        }
        *b = (byte & 1) == 1;
        return true;
    }

    // The nextFoo methods are deprecated.
    // TODO(kjlubick): replace existing uses with next() and remove these.
    bool nextBool();
    uint8_t  nextB();
    uint32_t nextU();
    // This can be nan, +- infinity, 0, anything.
    float    nextF();
    // Returns a float between [0..1) as a IEEE float
    float    nextF1();

    // Return the next fuzzed value [min, max) as an unsigned 32bit integer.
    uint32_t nextRangeU(uint32_t min, uint32_t max);
    /**
     *  Returns next fuzzed value [min...max) as a float.
     *  Will not be Infinity or NaN.
     */
    float    nextRangeF(float    min, float    max);

    void signalBug   ();  // Tell afl-fuzz these inputs found a bug.
    void signalBoring();  // Tell afl-fuzz these inputs are not worth testing.

private:
    template <typename T>
    T nextT();

    sk_sp<SkData> fBytes;
    int fNextByte;
};

template <typename T>
bool Fuzz::next(T* n) {
    if (fNextByte + sizeof(T) > fBytes->size()) {
        return false;
    }

    memcpy(n, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
    return true;
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
