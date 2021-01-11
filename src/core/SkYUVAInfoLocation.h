/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVAInfoLocation_DEFINED
#define SkYUVAInfoLocation_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkYUVAInfo.h"

/**
 * The location of Y, U, V, or A values within the planes described by SkYUVAInfo.
 */
struct SkYUVAInfo::YUVALocation {
    /** The index of the plane where the Y, U, V, or A values are found. */
    int fPlane = -1;
    /** The channel in the plane that contains the Y, U, V, or A value. */
    SkColorChannel fChannel = SkColorChannel::kA;

    bool operator==(const YUVALocation& that) const {
        return fPlane == that.fPlane && fChannel == that.fChannel;
    }
    bool operator!=(const YUVALocation& that) const { return !(*this == that); }

    static bool AreValidLocations(const SkYUVAInfo::YUVALocations& locations,
                                  int* numPlanes = nullptr) {
        int tempNumPlanes;
        numPlanes = numPlanes ? numPlanes : &tempNumPlanes;
        // Note that 'numPlanes' is always filled in even if the indices are not valid.
        // This means it can always be used to process the backing resources (but be careful
        // of empty intervening slots).
        int maxSlotUsed = -1;
        bool used[SkYUVAInfo::kMaxPlanes] = {};
        bool valid = true;
        for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
            if (locations[i].fPlane < 0) {
                if (i != SkYUVAInfo::YUVAChannels::kA) {
                    valid = false;  // only the 'A' plane can be omitted
                }
            } else if (locations[i].fPlane >= SkYUVAInfo::kMaxPlanes) {
                valid = false;  // A maximum of four input textures is allowed
            } else {
                maxSlotUsed = std::max(locations[i].fPlane, maxSlotUsed);
                used[i] = true;
            }
        }

        // All the used slots should be packed starting at 0 with no gaps
        for (int i = 0; i <= maxSlotUsed; ++i) {
            if (!used[i]) {
                valid = false;
            }
        }

        *numPlanes = maxSlotUsed + 1;
        return valid;
    }
};

#endif
