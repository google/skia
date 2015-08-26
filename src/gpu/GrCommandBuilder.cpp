/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCommandBuilder.h"

#include "GrInOrderCommandBuilder.h"
#include "GrReorderCommandBuilder.h"

GrCommandBuilder* GrCommandBuilder::Create(GrGpu* gpu, bool reorder) {
    if (reorder) {
        return new GrReorderCommandBuilder;
    } else {
        return new GrInOrderCommandBuilder;
    }
}
