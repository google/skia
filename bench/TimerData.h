
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TimerData_DEFINED
#define TimerData_DEFINED

#include "SkString.h"

class BenchTimer;

class TimerData {
public:
    TimerData(const SkString& perIterTimeFormat, const SkString& normalTimeFormat);

    /**
     * Append the value from each timer in BenchTimer to our various strings, and update the
     * minimum and sum times.
     * @param BenchTimer Must not be null.
     * @param last True if this is the last set of times to add.
     */
    void appendTimes(BenchTimer*, bool last);
    SkString getResult(bool logPerIter, bool printMin, int repeatDraw, const char* configName,
                       bool showWallTime, bool showTruncatedWallTime, bool showCpuTime,
                       bool showTruncatedCpuTime, bool showGpuTime);
private:
    SkString fWallStr;
    SkString fTruncatedWallStr;
    SkString fCpuStr;
    SkString fTruncatedCpuStr;
    SkString fGpuStr;
    double fWallSum, fWallMin;
    double fTruncatedWallSum, fTruncatedWallMin;
    double fCpuSum, fCpuMin;
    double fTruncatedCpuSum, fTruncatedCpuMin;
    double fGpuSum, fGpuMin;

    SkString fPerIterTimeFormat;
    SkString fNormalTimeFormat;
};

#endif // TimerData_DEFINED
