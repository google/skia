/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRegionBatch_DEFINED
#define GrRegionBatch_DEFINED

#include "GrColor.h"

class GrDrawBatch;
class SkMatrix;
class SkRegion;

namespace GrRegionBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRegion& region);

};

#endif
