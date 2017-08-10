/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathPriv.h"
#include "SkRecords.h"

namespace SkRecords {
    PreCachedPath::PreCachedPath(const SkPath& path) : SkPath(path) {
        this->updateBoundsCache();
        (void)this->getGenerationID();
#if 0  // Disabled to see if we ever really race on this.  It costs time, chromium:496982.
        SkPathPriv::FirstDirection junk;
        (void)SkPathPriv::CheapComputeFirstDirection(*this, &junk);
#endif
    }

    TypedMatrix::TypedMatrix(const SkMatrix& matrix) : SkMatrix(matrix) {
        (void)this->getType();
    }
}
