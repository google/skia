
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

TimerData::TimerData(int maxNumTimings)
: fMaxNumTimings(maxNumTimings)
, fCurrTiming(0)
, fWallTimes(maxNumTimings)
, fTruncatedWallTimes(maxNumTimings)
, fCpuTimes(maxNumTimings)
, fTruncatedCpuTimes(maxNumTimings)
, fGpuTimes(maxNumTimings){
}

bool TimerData::appendTimes(BenchTimer* timer) {
    SkASSERT(timer != NULL);
    if (fCurrTiming >= fMaxNumTimings) {
        return false;
    }

    fWallTimes[fCurrTiming] = timer->fWall;
    fTruncatedWallTimes[fCurrTiming] = timer->fTruncatedWall;
    fCpuTimes[fCurrTiming] = timer->fCpu;
    fTruncatedCpuTimes[fCurrTiming] = timer->fTruncatedCpu;
    fGpuTimes[fCurrTiming] = timer->fGpu;

    ++fCurrTiming;

    return true;
}

SkString TimerData::getResult(const char* doubleFormat,
                              Result result,
                              const char *configName,
                              uint32_t timerFlags,
                              int itersPerTiming) {
    SkASSERT(itersPerTiming >= 1);

    if (!fCurrTiming) {
        return SkString("");
    }

    int numTimings = fCurrTiming;

    SkString wallStr(" msecs = ");
    SkString truncWallStr(" Wmsecs = ");
    SkString cpuStr(" cmsecs = ");
    SkString truncCpuStr(" Cmsecs = ");
    SkString gpuStr(" gmsecs = ");

    double wallMin = std::numeric_limits<double>::max();
    double truncWallMin = std::numeric_limits<double>::max();
    double cpuMin = std::numeric_limits<double>::max();
    double truncCpuMin = std::numeric_limits<double>::max();
    double gpuMin = std::numeric_limits<double>::max();

    double wallSum = 0;
    double truncWallSum = 0;
    double cpuSum = 0;
    double truncCpuSum = 0;
    double gpuSum = 0;

    for (int i = 0; i < numTimings; ++i) {
        if (kPerIter_Result == result) {
            wallStr.appendf(doubleFormat, fWallTimes[i]);
            truncWallStr.appendf(doubleFormat, fTruncatedWallTimes[i]);
            cpuStr.appendf(doubleFormat, fCpuTimes[i]);
            truncCpuStr.appendf(doubleFormat, fTruncatedCpuTimes[i]);
            gpuStr.appendf(doubleFormat, fGpuTimes[i]);

            if (i != numTimings - 1) {
                static const char kSep[] = ", ";
                wallStr.append(kSep);
                truncWallStr.append(kSep);
                cpuStr.append(kSep);
                truncCpuStr.append(kSep);
                gpuStr.append(kSep);
            }
        } else if (kMin_Result == result) {
            wallMin = SkTMin(wallMin, fWallTimes[i]);
            truncWallMin = SkTMin(truncWallMin, fTruncatedWallTimes[i]);
            cpuMin = SkTMin(cpuMin, fCpuTimes[i]);
            truncCpuMin = SkTMin(truncCpuMin, fTruncatedCpuTimes[i]);
            gpuMin = SkTMin(gpuMin, fGpuTimes[i]);
        } else {
            SkASSERT(kAvg_Result == result);
            wallSum += fWallTimes[i];
            truncWallSum += fTruncatedWallTimes[i];
            cpuSum += fCpuTimes[i];
            truncCpuSum += fTruncatedCpuTimes[i];
        }

        // We always track the GPU sum because whether it is non-zero indicates if valid gpu times
        // were recorded at all.
        gpuSum += fGpuTimes[i];
    }

    if (kMin_Result == result) {
        wallStr.appendf(doubleFormat, wallMin / itersPerTiming);
        truncWallStr.appendf(doubleFormat, truncWallMin / itersPerTiming);
        cpuStr.appendf(doubleFormat, cpuMin / itersPerTiming);
        truncCpuStr.appendf(doubleFormat, truncCpuMin / itersPerTiming);
        gpuStr.appendf(doubleFormat, gpuMin / itersPerTiming);
    } else if (kAvg_Result == result) {
        int divisor = numTimings * itersPerTiming;
        wallStr.appendf(doubleFormat, wallSum / divisor);
        truncWallStr.appendf(doubleFormat, truncWallSum / divisor);
        cpuStr.appendf(doubleFormat, cpuSum / divisor);
        truncCpuStr.appendf(doubleFormat, truncCpuSum / divisor);
        gpuStr.appendf(doubleFormat, gpuSum / divisor);
    }

    SkString str;
    str.printf("  %4s:", configName);
    if (timerFlags & kWall_Flag) {
        str += wallStr;
    }
    if (timerFlags & kTruncatedWall_Flag) {
        str += truncWallStr;
    }
    if (timerFlags & kCpu_Flag) {
        str += cpuStr;
    }
    if (timerFlags & kTruncatedCpu_Flag) {
        str += truncCpuStr;
    }
    if ((timerFlags & kGpu_Flag) && gpuSum > 0) {
        str += gpuStr;
    }
    return str;
}
