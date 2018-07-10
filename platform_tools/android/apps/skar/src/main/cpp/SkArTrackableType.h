/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArTrackableType_DEFINED
#define SkArTrackableType_DEFINED

/**
 * Enum used to describe the type of an SkArTrackable
 */

enum class SkArTrackableType {
    kBaseTrackable = 1,
    kPlane = 2,
    kPoint = 3,
    kAugmentedImage = 4,
    kInvalid = 0,
};

int ConvertSkArTrackableType(SkArTrackableType type) {
    switch(type) {
        case SkArTrackableType::kBaseTrackable:
            return 0x41520100;
        case SkArTrackableType::kPlane:
            return 0x41520101;
        case SkArTrackableType::kPoint:
            return 0x41520102;
        case SkArTrackableType::kAugmentedImage:
            return 0x41520104;
        case SkArTrackableType::kInvalid:
            return 0;
    }
}

#endif  // SkArTrackingState_DEFINED
