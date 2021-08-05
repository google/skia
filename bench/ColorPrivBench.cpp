/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"

template <bool kFast, bool kScale>
class FourByteInterpBench : public Benchmark {
public:
    FourByteInterpBench() {
        fName.set("four_byte_interp");
        fName.append(kFast ? "_fast" : "_slow");
        fName.append(kScale ? "_255" : "_256");
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        // A handful of random srcs and dsts.
        SkRandom rand;
        for (int i = 0; i < kInputs; i++) {
            fSrcs[i] = SkPreMultiplyColor(rand.nextU());
            fDsts[i] = SkPreMultiplyColor(rand.nextU());
        }

        // We'll exhaustively test all scales instead of using random numbers.
        for (int i = 0; i <= 256; i++) {
            fScales[i] = i;
        }
        if (kScale) fScales[256] = 255;  // We'll just do 255 twice if we're limited to [0,255].
    }

    void onDraw(int loops, SkCanvas*) override {
        // We xor results of FourByteInterp into junk to make sure the function runs.
        SK_MAYBE_UNUSED volatile SkPMColor junk = 0;

        for (int loop = 0; loop < loops; loop++) {
            for (int i = 0; i < kInputs; i++) {
                for (size_t j = 0; j <= 256; j++) {
                    // Note: we really want to load src and dst here and not outside in the i-loop.
                    // If we put the loads there, a clever compiler will do the not-insignificant
                    // work in the FourByteInterps that depends only on src and dst outside this
                    // loop, so we'd only be benchmarking the back half of those functions that also
                    // depends on scale.  Even here, these must be volatile arrays to prevent that
                    // clever compiler from hoisting the loads out of the loop on its own.
                    const SkPMColor src = fSrcs[i];
                    const SkPMColor dst = fDsts[i];

                    const unsigned scale = fScales[j];

                    if (kFast && kScale) {
                        junk ^= SkFastFourByteInterp(src, dst, scale);
                    } else if (kFast) {
                        junk ^= SkFastFourByteInterp256(src, dst, scale);
                    } else if (kScale) {
                        junk ^= SkFourByteInterp(src, dst, scale);
                    } else {
                        junk ^= SkFourByteInterp256(src, dst, scale);
                    }
                }
            }
        }
    }

private:
    SkString fName;
    static const int kInputs = 10;  // Arbitrary.
    volatile unsigned fSrcs[kInputs];
    volatile unsigned fDsts[kInputs];
    unsigned fScales[257];  // We need space for [0, 256].
};

#define COMMA ,
DEF_BENCH(return (new FourByteInterpBench<true COMMA true>);)
DEF_BENCH(return (new FourByteInterpBench<true COMMA false>);)
DEF_BENCH(return (new FourByteInterpBench<false COMMA true>);)
DEF_BENCH(return (new FourByteInterpBench<false COMMA false>);)
#undef COMMA
