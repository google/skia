/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArTrackingState_DEFINED
#define SkArTrackingState_DEFINED

/**
 * Enum used to describe the tracking state of various AR objects (anchors, planes, points...)
 */

enum class SkArTrackingState {
    kStopped,
    kTracking,
    kPaused,
};

#endif  // SkArTrackingState_DEFINED
