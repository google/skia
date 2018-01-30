/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkanInvalidationController.h"

#include "SkRect.h"
#include "SkTLazy.h"

namespace skan {

InvalidationController::InvalidationController() : fBounds(SkRect::MakeEmpty()) {}

void InvalidationController::inval(const SkRect& r, const SkMatrix& ctm) {
    if (r.isEmpty()) {
        return;
    }

    SkTCopyOnFirstWrite<SkRect> rect(r);

    if (!ctm.isIdentity()) {
        ctm.mapRect(rect.writable());
    }

    fRects.push(*rect);
    fBounds.join(*rect);
}

} // namespace skan
