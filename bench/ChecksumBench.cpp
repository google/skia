/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOpts.h"

enum ChecksumType {
    kMD5_ChecksumType,
    kHash_ChecksumType,
};

class ComputeChecksumBench : public Benchmark {
    enum {
        U32COUNT  = 256,
        SIZE      = U32COUNT * 4,
    };
    uint32_t    fData[U32COUNT];
    ChecksumType fType;

public:
    ComputeChecksumBench(ChecksumType type) : fType(type) {
        SkRandom rand;
        for (int i = 0; i < U32COUNT; ++i) {
            fData[i] = rand.nextU();
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        switch (fType) {
            case kMD5_ChecksumType: return "compute_md5";
            case kHash_ChecksumType: return "compute_hash";

            default: SK_ABORT("Invalid Type"); return "";
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        switch (fType) {
            case kMD5_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    SkMD5 md5;
                    md5.write(fData, sizeof(fData));
                    (void)md5.finish();
                }
            } break;
            case kHash_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    volatile uint32_t result = SkOpts::hash(fData, sizeof(fData));
                    sk_ignore_unused_variable(result);
                }
            }break;
        }

    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ComputeChecksumBench(kMD5_ChecksumType); )
DEF_BENCH( return new ComputeChecksumBench(kHash_ChecksumType); )
