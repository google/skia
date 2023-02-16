/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkEncoder.h"

#include "include/private/base/SkAssert.h"

bool SkEncoder::encodeRows(int numRows) {
    SkASSERT(numRows > 0 && fCurrRow < fSrc.height());
    if (numRows <= 0 || fCurrRow >= fSrc.height()) {
        return false;
    }

    if (fCurrRow + numRows > fSrc.height()) {
        numRows = fSrc.height() - fCurrRow;
    }

    if (!this->onEncodeRows(numRows)) {
        // If we fail, short circuit any future calls.
        fCurrRow = fSrc.height();
        return false;
    }

    return true;
}
