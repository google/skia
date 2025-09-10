/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGInvalidationController.h"

#include "include/core/SkRect.h"

namespace sksg {

InvalidationController::InvalidationController() : fBounds(SkRect::MakeEmpty()) {}

void InvalidationController::inval(const SkRect& r, const SkMatrix& ctm) {
    if (r.isEmpty()) {
        return;
    }

    SkRect rect = ctm.mapRect(r);

    fRects.push_back(rect);
    fBounds.join(rect);
}

void InvalidationController::reset() {
    fRects.clear();
    fBounds.setEmpty();
}

} // namespace sksg
