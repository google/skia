/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGInvalidationController.h"

#include "SkRect.h"

namespace sksg {

InvalidationController::InvalidationController() {}

void InvalidationController::inval(const SkRect& r) {
    fRects.push(r);
}

} // namespace sksg
