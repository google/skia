/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitterTrace_DEFINED
#define SkBlitterTrace_DEFINED

#include <inttypes.h>
#include <unordered_map>
#include "src/utils/SkCycles.h"

class SkBlitterTrace {
/*
 * This class collects information for RasterPipeLine vs SkVM
 * performance comparison.
 * How to get the comparison table:
 * 1. Run nanobench for SkVM:
 *    []/Release/nanobench
 *    --csv --config 8888 --skvm --compare --loops 100 --samples 1
 *    --match $(ls skps | grep --invert-match svg ) 2>&1 | tee VM.data
 * 2. Run nanobench for RasterPipeLine:
 *    []/Release/nanobench
 *    --csv --config 8888 --forceRasterPipeline --compare --loops 100
 *    --samples 1 --match $(ls skps | grep --invert-match svg )
 *    2>&1 | tee RP.data
 * 3. Extract the information side-by-side:
 *    awk 'BEGIN {OFS=","; fileNum = 0} ($2 ~ /MB/) && fileNum == 0
 *    {vmvmcycles[$3] = $6; vmvmscan[$3] = $8; vmvmpixels[$3] = $10;
 *    vmvminterp[$3] = $11;  vmrpcycle[$3] = $14; vmrpscan[$3] = $16;
 *    vmrppixels[$3] = $18} ($2 ~ /MB/) && fileNum == 1  {print $3,
 *    vmvmcycles[$3], vmvmscan[$3], vmvmpixels[$3], vmvminterp[$3], $6, $8,
 *    $10, $11, $14, $16, $18} ENDFILE {fileNum += 1}'
 *    VM.data RP.data > compare.csv
 * 4. Open the compare.csv table in Google Spreadsheets.
 *     You will get columns [A:P]. Add 4 more columns with formulas:
 *     Q: =B/M-1
 *     R: =N-C
 *     S: =O-D
 *     T: =2*(S<>0)+(R<>0)
 *     To be honest R, S, T columns are here for checking only (they all
 *     supposed to have zero values in them)
 *     Column Q shows the actual performance difference. Negative value means
 *     that wins SkVM, positive - RasterPipeLine.
 */
public:
    SkBlitterTrace(const char* header, bool traceSteps = false)
        : fHeader(header), fTraceSteps(traceSteps) { }

    SkBlitterTrace& operator= (const SkBlitterTrace&) = default;

    void addTrace(const char* name, uint64_t cycles, uint64_t scanLines, uint64_t pixels) {
        fCycles += cycles;
        fScanlines += scanLines;
        fPixels += pixels;
        if (fTraceSteps) {
            printIncrements(name, cycles, scanLines, pixels);
        }
    }

    void reset() {
        fCycles = 0ul;
        fScanlines = 0ul;
        fPixels = 0ul;
    }

    void printIncrements(const char* name,
                         uint64_t cycles,
                         uint64_t scanLines,
                         uint64_t pixels) const {
        SkDebugf("%s %s: cycles=%" PRIu64 "+%" PRIu64
                       " scanlines=%" PRIu64 "+%" PRIu64 " pixels=%" PRIu64,
                 fHeader, name,
                 fCycles - cycles, cycles,
                 fScanlines - scanLines, scanLines,
                 fPixels);
        SkDebugf("\n");
    }

    void printCounts(const char* name) const {
        SkDebugf("%s cycles: %" PRIu64 " "
                 "   scanlines: %" PRIu64 " pixels: %" PRIu64,
                 fHeader,
                 fCycles,
                 fScanlines,
                 fPixels);
        SkDebugf(" ");
    }

    void turnTrace(bool value) { fTraceTime = value; }
    uint64_t getCycles() const { return fCycles; }
    uint64_t getScanlines() const { return fScanlines; }
    uint64_t getPixels() const { return fPixels; }

    class Step {
    public:
        Step(SkBlitterTrace* trace,
             const char* name,
             uint64_t scanlines,
             uint64_t pixels)
            : fTrace(trace)
            , fName(name)
            , fScanlines(scanlines)
            , fPixels(pixels) {
            fStartTime = SkCycles::Now();
        }
        void add(uint64_t scanlines, uint64_t pixels) {
            fScanlines += scanlines;
            fPixels += pixels;
        }
        ~Step() {
            if (fTrace == nullptr || !fTrace->fTraceTime) {
                return;
            }
            auto endTime = SkCycles::Now() - fStartTime;
            fTrace->addTrace(/*name=*/fName,
                             /*cycles=*/endTime,
                             /*scanlines=*/fScanlines,
                             /*pixels=*/fPixels);
        }
    private:
        SkBlitterTrace* fTrace = nullptr;
        const char* fName = "";
        uint64_t fStartTime = 0ul;
        uint64_t fScanlines = 0ul;
        uint64_t fPixels = 0ul;
    };

private:
    const char* fHeader = "";
    bool fTraceSteps = false;
    bool fTraceTime = false;
    uint64_t fCycles = 0ul;
    uint64_t fScanlines = 0ul;
    uint64_t fPixels = 0ul;
};

#endif
