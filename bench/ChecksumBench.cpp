/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkChecksum.h"
#include "SkRandom.h"

class ComputeChecksumBench : public SkBenchmark {
    enum {
        U32COUNT  = 256,
        SIZE      = U32COUNT * 4,
        N         = SkBENCHLOOP(100000),
    };
    uint32_t    fData[U32COUNT];

public:
    ComputeChecksumBench(void* param) : INHERITED(param) {
        SkRandom rand;
        for (int i = 0; i < U32COUNT; ++i) {
            fData[i] = rand.nextU();
        }
    }

protected:
    virtual const char* onGetName() {
        return "compute_checksum";
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; i++) {
            volatile uint32_t result = SkChecksum::Compute(fData, sizeof(fData));
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new ComputeChecksumBench(p); }

static BenchRegistry gReg0(Fact0);
