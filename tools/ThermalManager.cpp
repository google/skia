/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ThermalManager.h"

#include "SkOSFile.h"

#include <stdio.h>

#ifndef SK_BUILD_FOR_WIN32
    #include <unistd.h>
#endif

#ifdef THERMAL_MANAGER_SUPPORTED

/*
 * ThermalManager is completely dependent on sysfs to monitor thermal temperatures.  In sysfs
 * thermal management is controlled by a number of thermal zones.  They are laid out as follows:
 * /sys/class/thermal/thermal_zoneN where N is the number of the thermal zone starting at 0.
 *
 * Inside each thermal_zone folder is a file called 'temp,' which has the current temperature
 * reading from the sensor in that zone, as well as 0 or more files called 'trip_point_N_temp.'
 *
 * When the reading in temp is greater than one of the numbers in the trip_point files, then the
 * kernel will take some kind of action.  This is all documented online.
 *
 * In any case, the goal of this class is to sleep right before a trip point is about to be
 * triggered, thus naturally cooling the system and preventing thermal throttling.
 */

ThermalManager::ThermalManager(int32_t threshold, uint32_t sleepIntervalMs, uint32_t timeoutMs)
    : fSleepIntervalMs(sleepIntervalMs)
    , fTimeoutMs(timeoutMs) {
    static const char* kThermalZonePath = "/sys/class/thermal/";
    SkOSFile::Iter it(kThermalZonePath);
    SkString path;
    while (it.next(&path, true)) {
        if (!path.contains("thermal_zone")) {
            continue;
        }

        SkString fullPath(kThermalZonePath);
        fullPath.append(path);
        SkOSFile::Iter thermalZoneIt(fullPath.c_str());

        SkString filename;
        while (thermalZoneIt.next(&filename)) {
            if (!(filename.contains("trip_point") && filename.contains("temp"))) {
                continue;
            }

            fTripPoints.push_back(TripPoint(fullPath, filename, threshold));
        }
    }
}

bool ThermalManager::coolOffIfNecessary() {
    uint32_t i = 0, totalTimeSleptMs = 0;
    while (i < (uint32_t)fTripPoints.count() && totalTimeSleptMs < fTimeoutMs) {
        if (fTripPoints[i].willTrip()) {
            sleep(fSleepIntervalMs);
            totalTimeSleptMs += fSleepIntervalMs;
        } else {
            i++;
        }
    }

    return totalTimeSleptMs < fTimeoutMs;
}

int32_t ThermalManager::OpenFileAndReadInt32(const char* path) {
    FILE* tempFile = fopen(path, "r");
    SkASSERT(tempFile);
    int32_t value;
    int ret = fscanf(tempFile, "%d", &value);
    if (!ret) {
        SkDebugf("Could not read temperature\n");
        SkASSERT(false);
    }

    fclose(tempFile);
    return value;
}

ThermalManager::TripPoint::TripPoint(SkString thermalZoneRoot, SkString pointName,
                                     int32_t threshold)
    : fThermalZoneRoot(thermalZoneRoot)
    , fPointName(pointName) {
    SkString fullPath(thermalZoneRoot);
    fullPath.appendf("/%s", pointName.c_str());
    fPoint = OpenFileAndReadInt32(fullPath.c_str());
    fBase = GetTemp(fThermalZoneRoot);
    fThreshold = threshold;
    fDisabled = fBase + fThreshold >= fPoint;  // We disable any trip point which start off
                                               // triggered
}

bool ThermalManager::TripPoint::willTrip() {
    int32_t currentTemp = GetTemp(fThermalZoneRoot);
    bool wouldTrip = !fDisabled && currentTemp + fThreshold >= fPoint;

    if (wouldTrip) {
        SkDebugf("%s/%s would trip {%d,%d,%d,%d}\n", fThermalZoneRoot.c_str(),
                 fPointName.c_str(), fBase, currentTemp, fPoint, fThreshold);
    }
    return wouldTrip;
}

#endif
