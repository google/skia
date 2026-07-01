/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/SkAlign.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <memory>

template <typename T>
static T* allocate_and_align(std::unique_ptr<char[]>& storage, size_t sizeInBytes) {
    // Align to 64 bytes (critical for AVX-512 in SkRasterPipeline).
    // The initial allocation might not be 64-byte aligned, so we allocate extra padding
    // and find the next address in our buffer that is a multiple of 64.
    storage.reset(new char[sizeInBytes + 64]);
    return reinterpret_cast<T*>(
            SkAlignTo(reinterpret_cast<uintptr_t>(storage.get()), (uintptr_t)64));
}

class SkRPDivBench : public Benchmark {
public:
    enum class Type { kInt, kUint };

    SkRPDivBench(Type type, int slots) : fType(type), fSlots(slots) {
        fName.printf("skrp_div_%s_%d", fType == Type::kInt ? "int" : "uint", fSlots);

        fData = allocate_and_align<int32_t>(fStorage, 1024 * sizeof(int32_t));

        // Initialize the data we are dividing by so it's non-zero (avoiding any special casing).
        for (int i = 0; i < 1024; i++) {
            fData[i] = i + 1;
        }
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        SkArenaAlloc alloc(1024);
        SkRasterPipeline p(&alloc);

        SkRasterPipelineOp op;
        if (fType == Type::kInt) {
            switch (fSlots) {
                case 1: op = SkRasterPipelineOp::div_int; break;
                case 4: op = SkRasterPipelineOp::div_4_ints; break;
                default: SkUNREACHABLE;
            }
        } else {
            switch (fSlots) {
                case 1: op = SkRasterPipelineOp::div_uint; break;
                case 4: op = SkRasterPipelineOp::div_4_uints; break;
                default: SkUNREACHABLE;
            }
        }
        p.append(op, fData);

        for (int i = 0; i < loops; i++) {
            p.run(0, 0, 128, 1);
        }
    }

private:
    Type fType;
    int fSlots;
    SkString fName;
    std::unique_ptr<char[]> fStorage;
    int32_t* fData = nullptr;
};

DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kInt, 1);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kInt, 4);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kUint, 1);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kUint, 4);)
