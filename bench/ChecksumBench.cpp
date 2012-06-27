/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkChecksum.h"
#include "SkString.h"

class ComputeChecksumBench : public SkBenchmark {
public:
    ComputeChecksumBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("compute_checksum_%s", name);
    }

    enum {
        DATA_SIZE = 1024,
        N         = SkBENCHLOOP(100000),
    };
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        uint64_t data[DATA_SIZE / sizeof(uint64_t)];
        computeChecksum(data, DATA_SIZE);
    }

    virtual void computeChecksum(const uint64_t*, size_t) = 0;

    SkString fName;
private:
    typedef SkBenchmark INHERITED;
};

/*
 *  Use SkComputeChecksum32 to compute a checksum on a datablock
 */
class ComputeChecksum32Bench : public ComputeChecksumBench {
public:
    ComputeChecksum32Bench(void* param)
        : INHERITED(param, "32") { }

protected:
    virtual void computeChecksum(const uint64_t* data, size_t len) {
        for (int i = 0; i < N; i++) {
            volatile uint32_t result = SkComputeChecksum32(reinterpret_cast<const uint32_t*>(data), len);
        }
    }

private:
    typedef ComputeChecksumBench INHERITED;
};

/*
 *  Use SkComputeChecksum64 to compute a checksum on a datablock
 */
class ComputeChecksum64Bench : public ComputeChecksumBench {
public:
    ComputeChecksum64Bench(void* param)
        : INHERITED(param, "64") { }

protected:
    virtual void computeChecksum(const uint64_t* data, size_t len) {
        for (int i = 0; i < N; i++) {
            volatile uint64_t result = SkComputeChecksum64(data, len);
        }
    }

private:
    typedef ComputeChecksumBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new ComputeChecksum32Bench(p); }
static SkBenchmark* Fact1(void* p) { return new ComputeChecksum64Bench(p); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
