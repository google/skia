/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNullCanvas.h"

#include "SkCanvas.h"
#include "SkNWayCanvas.h"
#include "SkMakeUnique.h"

std::unique_ptr<SkCanvas> SkMakeNullCanvas() {
    // An N-Way canvas forwards calls to N canvas's. When N == 0 it's
    // effectively a null canvas.
    return std::unique_ptr<SkCanvas>(new SkNWayCanvas(0, 0));
}
