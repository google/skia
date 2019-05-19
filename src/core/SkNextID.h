/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNextID_DEFINED
#define SkNextID_DEFINED

#include "include/core/SkTypes.h"

class SkNextID {
public:
    /**
     *  Shared between SkPixelRef's generationID and SkImage's uniqueID
     */
    static uint32_t ImageID();
};

#endif
