
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TimerData.h"

#include "BenchTimer.h"
#include <limits>

using namespace std;

TimerData::TimerData(const SkString& perIterTimeFormat, const SkString& normalTimeFormat)
: fWallStr(" msecs = ")
, fTruncatedWallStr(" Wmsecs = ")
, fCpuStr(" cmsecs = ")
, fTruncatedCpuStr(" Cmsecs = ")
, fGpuStr(" gmsecs = ")
, fWallSum(0.0)
, fWallMin(numeric_limits<double>::max())
, fTruncatedWallSum(0.0)
, fTruncatedWallMin(numeric_limits<double>::max())
, fCpuSum(0.0)
, fCpuMin(numeric_limits<double>::max())
, fTruncatedCpuSum(0.0)
, fTruncatedCpuMin(numeric_limits<double>::max())
, fGpuSum(0.0)
, fGpuMin(numeric_limits<double>::max())
, fPerIterTimeFormat(perIterTimeFormat)
, fNormalTimeFormat(normalTimeFormat)
{}

static double Min(double a, double b) {
    return (a < b) ? a : b;
}

void TimerData::appendTimes(BenchTimer* timer, bool last) {
    SkASSERT(timer != NULL);
    SkString formatString(fPerIterTimeFormat);
    if (!last) {
        formatString.append(",");
    }
    const char* format = formatString.c_str();
    fWallStr.appendf(format, timer->fWall);
    fCpuStr.appendf(format, timer->fCpu);
    fTruncatedWallStr.appendf(format, timer->fTruncatedWall);
    fTruncatedCpuStr.appendf(format, timer->fTruncatedCpu);
    fGpuStr.appendf(format, timer->fGpu);

    // Store the minimum values. We do not need to special case the first time since we initialized
    // to max double.
    fWallMin = Min(fWallMin, timer->fWall);
    fCpuMin  = Min(fCpuMin,  timer->fCpu);
    fTruncatedWallMin = Min(fTruncatedWallMin, timer->fTruncatedWall);
    fTruncatedCpuMin  = Min(fTruncatedCpuMin,  timer->fTruncatedCpu);
    fGpuMin  = Min(fGpuMin,  timer->fGpu);

    // Tally the sum of each timer type.
    fWallSum += timer->fWall;
    fCpuSum += timer->fCpu;
    fTruncatedWallSum += timer->fTruncatedWall;
    fTruncatedCpuSum += timer->fTruncatedCpu;
    fGpuSum += timer->fGpu;

}

SkString TimerData::getResult(bool logPerIter, bool printMin, int repeatDraw,
                              const char *configName, bool showWallTime, bool showTruncatedWallTime,
                              bool showCpuTime, bool showTruncatedCpuTime, bool showGpuTime) {
    // output each repeat (no average) if logPerIter is set,
    // otherwise output only the average
    if (!logPerIter) {
        const char* format = fNormalTimeFormat.c_str();
        fWallStr.set(" msecs = ");
        fWallStr.appendf(format, printMin ? fWallMin : fWallSum / repeatDraw);
        fCpuStr.set(" cmsecs = ");
        fCpuStr.appendf(format, printMin ? fCpuMin : fCpuSum / repeatDraw);
        fTruncatedWallStr.set(" Wmsecs = ");
        fTruncatedWallStr.appendf(format,
                                  printMin ? fTruncatedWallMin : fTruncatedWallSum / repeatDraw);
        fTruncatedCpuStr.set(" Cmsecs = ");
        fTruncatedCpuStr.appendf(format,
                                 printMin ? fTruncatedCpuMin : fTruncatedCpuSum / repeatDraw);
        fGpuStr.set(" gmsecs = ");
        fGpuStr.appendf(format, printMin ? fGpuMin : fGpuSum / repeatDraw);
    }
    SkString str;
    str.printf("  %4s:", configName);
    if (showWallTime) {
        str += fWallStr;
    }
    if (showTruncatedWallTime) {
        str += fTruncatedWallStr;
    }
    if (showCpuTime) {
        str += fCpuStr;
    }
    if (showTruncatedCpuTime) {
        str += fTruncatedCpuStr;
    }
    if (showGpuTime && fGpuSum > 0) {
        str += fGpuStr;
    }
    return str;
}
