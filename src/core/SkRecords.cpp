/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRecords.h"

namespace SkRecords {
    TypedMatrix::TypedMatrix(const SkMatrix& matrix) : SkMatrix(matrix) {
        (void)this->getType();
    }
}  // namespace SkRecords
