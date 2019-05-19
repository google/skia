/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGInvalidationController.h"

#include "include/core/SkRect.h"
#include "src/core/SkTLazy.h"

namespace sksg {

InvalidationController::InvalidationController() : fBounds(SkRect::MakeEmpty()) {}

void InvalidationController::inval(const SkRect& r, const SkMatrix& ctm) {
    if (r.isEmpty()) {
        return;
    }

    SkTCopyOnFirstWrite<SkRect> rect(r);

    if (!ctm.isIdentity()) {
        ctm.mapRect(rect.writable());
    }

    fRects.push_back(*rect);
    fBounds.join(*rect);
}

} // namespace sksg
