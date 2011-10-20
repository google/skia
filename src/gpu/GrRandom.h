
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrRandom_DEFINED
#define GrRandom_DEFINED

class GrRandom {
public:
    GrRandom() : fSeed(0) {}
    GrRandom(uint32_t seed) : fSeed(seed) {}

    uint32_t seed() const { return fSeed; }

    uint32_t nextU() {
        fSeed = fSeed * kMUL + kADD;
        return fSeed;
    }

    int32_t nextS() { return (int32_t)this->nextU(); }

    /**
     *  Returns value [0...1) as a float
     */
    float nextF() {
        // const is 1 / (2^32 - 1)
        return (float)(this->nextU() * 2.32830644e-10);
    }

    /**
     *  Returns value [min...max) as a float
     */
    float nextF(float min, float max) {
        return min + this->nextF() * (max - min);
    }

private:
    /*
     *  These constants taken from "Numerical Recipes in C", reprinted 1999
     */
    enum {
        kMUL = 1664525,
        kADD = 1013904223
    };
    uint32_t    fSeed;
};

#endif

