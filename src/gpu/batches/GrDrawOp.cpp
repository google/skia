/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOp.h"

GrDrawOp::GrDrawOp(uint32_t classID)
        : INHERITED(classID)
        , fRenderTargetUniqueID(GrGpuResource::UniqueID::InvalidID()) {
}
