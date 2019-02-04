/*
 * Copyright 2016 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScan.h"

void SkScan::DAAFillPath(const SkPath& path, SkBlitter* blitter, const SkIRect& ir,
                         const SkIRect& clipBounds, bool forceRLE, SkDAARecord* record) {
    SkDEBUGFAIL("DAA Disabled");
    return;
}
