/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkChecksum.h"
#include "SkMD5.h"
#include "SkRandom.h"
#include "SkSHA1.h"
#include "SkTemplates.h"

enum ChecksumType {
    kChecksum_ChecksumType,
    kMD5_ChecksumType,
    kSHA1_ChecksumType,
    kMurmur3_ChecksumType,
};

class ComputeChecksumBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        switch (fType) {
            case kChecksum_ChecksumType: return "compute_checksum";
            case kMD5_ChecksumType: return "compute_md5";
            case kSHA1_ChecksumType: return "compute_sha1";
            case kMurmur3_ChecksumType: return "compute_murmur3";

            default: SK_CRASH(); return "";
        }
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        switch (fType) {
            case kChecksum_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    volatile uint32_t result = SkChecksum::Compute(fData, sizeof(fData));
                    sk_ignore_unused_variable(result);
                }
            } break;
            case kMD5_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    SkMD5 md5;
                    md5.update(reinterpret_cast<uint8_t*>(fData), sizeof(fData));
                    SkMD5::Digest digest;
                    md5.finish(digest);
                }
            } break;
            case kSHA1_ChecksumType: {
                for (int i = 0; i < loops; i++) {
                    SkSHA1 sha1;
                    sha1.update(reinterpret_cast<uint8_t*>(fData), sizeof(fData));
                    SkSHA1::Digest digest;
                    sha1.finish(digest);
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
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ComputeChecksumBench(kChecksum_ChecksumType); )
DEF_BENCH( return new ComputeChecksumBench(kMD5_ChecksumType); )
DEF_BENCH( return new ComputeChecksumBench(kSHA1_ChecksumType); )
DEF_BENCH( return new ComputeChecksumBench(kMurmur3_ChecksumType); )
