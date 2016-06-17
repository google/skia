/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ThermalManager_DEFINED
#define ThermalManager_DEFINED

#include "../private/SkTArray.h"
#include "SkString.h"

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
#    define THERMAL_MANAGER_SUPPORTED
#endif

#ifdef THERMAL_MANAGER_SUPPORTED

/*
 * This simple class monitors the thermal part of sysfs to ensure we don't trigger thermal events
 */

class ThermalManager {
public:
    ThermalManager(int32_t threshold, uint32_t sleepIntervalMs, uint32_t timeoutMs);

    bool coolOffIfNecessary();

private:
    static int32_t OpenFileAndReadInt32(const char* path);

    // current temperature can be read from /thermalZonePath/temp
    static int32_t GetTemp(SkString thermalZonePath) {
        SkString temperatureFilePath(thermalZonePath);
        temperatureFilePath.appendf("/temp");
        return OpenFileAndReadInt32(temperatureFilePath.c_str());
    }

    struct TripPoint {
        TripPoint(SkString thermalZoneRoot, SkString pointName, int32_t threshold);

        bool willTrip();

        SkString fThermalZoneRoot;
        SkString fPointName;
        int32_t fBase;
        int32_t fPoint;
        int32_t fThreshold;

        // Certain trip points seem to start tripped.  For example, I have seen trip points of 0 or
        // negative numbers.
        bool fDisabled;
    };

    SkTArray<TripPoint> fTripPoints;
    uint32_t fSleepIntervalMs;
    uint32_t fTimeoutMs;
};
#endif
#endif
