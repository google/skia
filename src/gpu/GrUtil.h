/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrUtil_DEFINED
#define GrUtil_DEFINED

#include "include/core/SkTypes.h"

enum GrIntelGpuFamily {
    kUnknown_IntelGpuFamily,

    // 6th gen
    kSandyBridge_IntelGpuFamily,

    // 7th gen
    kIvyBridge_IntelGpuFamily,
    kValleyView_IntelGpuFamily, // aka BayTrail
    kHaswell_IntelGpuFamily,

    // 8th gen
    kCherryView_IntelGpuFamily, // aka Braswell
    kBroadwell_IntelGpuFamily,

    // 9th gen
    kApolloLake_IntelGpuFamily,
    kSkyLake_IntelGpuFamily,
    kGeminiLake_IntelGpuFamily,
    kKabyLake_IntelGpuFamily,
    kCoffeeLake_IntelGpuFamily,

    // 11th gen
    kIceLake_IntelGpuFamily,
};

GrIntelGpuFamily GrGetIntelGpuFamily(uint32_t deviceID) {
    // https://en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units
    uint32_t maskedID = deviceID & 0xFF00;
    switch (maskedID) {
        case 0x0100:
            switch (deviceID & 0xFFF0) {
                case 0x0100:
                case 0x0110:
                case 0x0120:
                    return kSandyBridge_IntelGpuFamily;
                case 0x0150:
                    if (deviceID == 0x0155 || deviceID == 0x0157) {
                        return kValleyView_IntelGpuFamily;
                    }
                    if (deviceID == 0x0152 || deviceID == 0x015A) {
                        return kIvyBridge_IntelGpuFamily;
                    }
                    break;
                case 0x0160:
                    return kIvyBridge_IntelGpuFamily;
                default:
                    break;
            }
            break;
        case 0x0F00:
            return kValleyView_IntelGpuFamily;
        case 0x0400:
        case 0x0A00:
        case 0x0D00:
            return kHaswell_IntelGpuFamily;
        case 0x2200:
            return kCherryView_IntelGpuFamily;
        case 0x1600:
            return kBroadwell_IntelGpuFamily;
        case 0x5A00:
            return kApolloLake_IntelGpuFamily;
        case 0x1900:
            return kSkyLake_IntelGpuFamily;
        case 0x3100:
            return kGeminiLake_IntelGpuFamily;
        case 0x5900:
            return kKabyLake_IntelGpuFamily;
        case 0x3E00:
            return kCoffeeLake_IntelGpuFamily;
        case 0x8A00:
            return kIceLake_IntelGpuFamily;
        default:
            break;
    }
    return kUnknown_IntelGpuFamily;
}

#endif
