
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TimerData_DEFINED
#define TimerData_DEFINED

#include "SkString.h"
#include "SkTemplates.h"


class BenchTimer;

class TimerData {
public:
    /**
     * Constructs a TimerData to hold at most maxNumTimings sets of elapsed timer values.
     **/
    explicit TimerData(int maxNumTimings);

    /**
     * Collect times from the BenchTimer for an iteration. It will fail if called more often than
     * indicated in the constructor.
     *
     * @param BenchTimer Must not be null.
     */
    bool appendTimes(BenchTimer*);

    enum Result {
        kMin_Result,
        kAvg_Result,
        kPerIter_Result
    };

    enum TimerFlags {
        kWall_Flag              = 0x1,
        kTruncatedWall_Flag     = 0x2,
        kCpu_Flag               = 0x4,
        kTruncatedCpu_Flag      = 0x8,
        kGpu_Flag               = 0x10
    };

    SkString getResult(const char* doubleFormat,
                       Result result,
                       const char* configName,
                       uint32_t timerFlags,
                       int itersPerTiming = 1);
private:
    int fMaxNumTimings;
    int fCurrTiming;

    SkAutoTArray<double> fWallTimes;
    SkAutoTArray<double> fTruncatedWallTimes;
    SkAutoTArray<double> fCpuTimes;
    SkAutoTArray<double> fTruncatedCpuTimes;
    SkAutoTArray<double> fGpuTimes;
};

#endif // TimerData_DEFINED
