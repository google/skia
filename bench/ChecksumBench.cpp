/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkChecksum.h"
#include "SkMD5.h"
#include "SkRandom.h"
#include "SkTemplates.h"

enum ChecksumType {
    kMD5_ChecksumType,
    kMurmur3_ChecksumType,
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
            case kMurmur3_ChecksumType: return "compute_murmur3";

            default: SK_ABORT("Invalid Type"); return "";
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        switch (fType) {
            case kMD5_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    SkMD5 md5;
                    md5.write(fData, sizeof(fData));
                    SkMD5::Digest digest;
                    md5.finish(digest);
                }
            } break;
            case kMurmur3_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    volatile uint32_t result = SkChecksum::Murmur3(fData, sizeof(fData));
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
DEF_BENCH( return new ComputeChecksumBench(kMurmur3_ChecksumType); )
