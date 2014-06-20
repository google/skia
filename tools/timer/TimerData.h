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

#ifdef SK_BUILD_FOR_WIN
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif

#include "SkJSONCPP.h"

#ifdef SK_BUILD_FOR_WIN
    #pragma warning(pop)
#endif

class Timer;

class TimerData {
public:
    /**
     * Constructs a TimerData to hold at most maxNumTimings sets of elapsed timer values.
     **/
    explicit TimerData(int maxNumTimings);

    /**
     * Collect times from the Timer for an iteration. It will fail if called more often than
     * indicated in the constructor.
     *
     * @param Timer Must not be null.
     */
    bool appendTimes(Timer*);

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

    /**
     * Gets the timer data results as a string.
     * @param doubleFormat printf-style format for doubles (e.g. "%02d")
     * @param result the type of result desired
     * @param the name of the config being timed (prepended to results string)
     * @param timerFlags bitfield of TimerFlags values indicating which timers should be reported.
     * @param itersPerTiming the number of test/bench iterations that correspond to each
     *        appendTimes() call, 1 when appendTimes is called for each iteration.
     */
    SkString getResult(const char* doubleFormat,
                       Result result,
                       const char* configName,
                       uint32_t timerFlags,
                       int itersPerTiming = 1);
    Json::Value getJSON(uint32_t timerFlags,
                        Result result,
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
