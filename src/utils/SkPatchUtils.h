/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPatchUtils_DEFINED
#define SkPatchUtils_DEFINED

#include "SkPatch.h"
#include "SkMatrix.h"

class SK_API SkPatchUtils {

public:
    /**
     * Method that calculates a level of detail (number of subdivisions) for a patch in both axis. 
     */
    static SkISize GetLevelOfDetail(const SkPatch& patch, const SkMatrix* matrix);
};

#endif
