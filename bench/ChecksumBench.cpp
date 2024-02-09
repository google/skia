/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkMD5.h"

#include <memory>

enum ChecksumType {
    kMD5_ChecksumType,
    kWyhash_ChecksumType,
};

class ComputeChecksumBench : public Benchmark {
public:
    // All instances of this benchmark compute a checksum over 64K (or slightly less, if the
    // block size doesn't divide evenly).
    constexpr static size_t kBufferSize = 64 * 1024;

    ChecksumType fType;
    size_t fBlockSize;
    SkString fName;
    std::unique_ptr<uint8_t[]> fData;

    ComputeChecksumBench(ChecksumType type, size_t blockSize) : fType(type), fBlockSize(blockSize) {
        SkASSERT(blockSize <= kBufferSize);

        switch (fType) {
            case kMD5_ChecksumType: fName = "compute_md5"; break;
            case kWyhash_ChecksumType: fName = "compute_wyhash"; break;
        }
        fName.appendf("_%d", static_cast<int>(fBlockSize));
    }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    const char* onGetName() override { return fName.c_str(); }

    void onPreDraw(SkCanvas*) override {
        fData.reset(new uint8_t[kBufferSize]);

        SkRandom rand;
        for (size_t i = 0; i < kBufferSize; ++i) {
            fData[i] = rand.nextBits(8);
        }
    }

    void onPostDraw(SkCanvas*) override {
        fData.reset();
    }

    void onDraw(int loops, SkCanvas*) override {
        volatile uint32_t result = 0;
        const size_t blockCount = kBufferSize / fBlockSize;
        const uint8_t* bufEnd = fData.get() + (blockCount * fBlockSize);
        for (int i = 0; i < loops; i++) {
            for (const uint8_t* buf = fData.get(); buf < bufEnd; buf += fBlockSize) {
                switch (fType) {
                    case kMD5_ChecksumType: {
                        SkMD5 md5;
                        md5.write(buf, fBlockSize);
                        (void)md5.finish();
                        break;
                    }
                    case kWyhash_ChecksumType:
                        result = SkChecksum::Hash32(buf, fBlockSize);
                        break;
                }
            }
        }
        sk_ignore_unused_variable(result);
    }
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ComputeChecksumBench(kMD5_ChecksumType, 1024); )

#define DEF_CHECKSUM_BENCH(T) \
    DEF_BENCH( return new ComputeChecksumBench(T, 4); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 8); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 15); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 16); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 31); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 32); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 96); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 1024); )

DEF_CHECKSUM_BENCH(kWyhash_ChecksumType)
