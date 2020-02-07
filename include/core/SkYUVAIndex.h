/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVAIndex_DEFINED
#define SkYUVAIndex_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"

/** \struct SkYUVAIndex
    Describes from which image source and which channel to read each individual YUVA plane.

    SkYUVAIndex contains a index for which image source to read from and a enum for which channel
    to read from.
*/
struct SK_API SkYUVAIndex {
    bool operator==(const SkYUVAIndex& that) const {
        return this->fIndex == that.fIndex && this->fChannel == that.fChannel;
    }

    bool operator!=(const SkYUVAIndex& that) const {
        return !(*this == that);
    }

    // Index in the array of SkYUVAIndex
    // TODO: rename as Component
    enum Index {
        kY_Index = 0,
        kU_Index = 1,
        kV_Index = 2,
        kA_Index = 3,

        kLast_Index = kA_Index
    };
    static constexpr int kIndexCount = kLast_Index + 1;

    /** The index is a number between -1..3 which defines which image source to read from, where -1
     * means the image source doesn't exist. The assumption is we will always have image sources for
     * each of YUV planes, but optionally have image source for A plane. */
    int            fIndex;
    /** The channel describes from which channel to read the info from. Currently we only deal with
     * YUV and NV12 and channel info is ignored. */
    SkColorChannel fChannel;

    static bool AreValidIndices(const SkYUVAIndex yuvaIndices[4], int* numPlanes) {
        // Note that 'numPlanes' is always filled in even if the indices are not valid.
        // This means it can always be used to process the backing resources (but be careful
        // of empty intervening slots).
        int maxSlotUsed = -1;
        bool used[4] = { false, false, false, false };
        bool valid = true;
        for (int i = 0; i < 4; ++i) {
            if (yuvaIndices[i].fIndex < 0) {
                if (SkYUVAIndex::kA_Index != i) {
                    valid = false; // only the 'A' plane can be omitted
                }
            } else if (yuvaIndices[i].fIndex > 3) {
                valid = false; // A maximum of four input textures is allowed
            } else {
                maxSlotUsed = std::max(yuvaIndices[i].fIndex, maxSlotUsed);
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
