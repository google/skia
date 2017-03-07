/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceContext.h"
#include "SkColorSpace_Base.h"

#include "../private/GrAuditTrail.h"


// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrContext* context,
                                   GrDrawingManager* drawingMgr,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : fContext(context)
    , fColorSpace(std::move(colorSpace))
    , fAuditTrail(auditTrail)
#ifdef SK_DEBUG
    , fSingleOwner(singleOwner)
#endif
    , fDrawingManager(drawingMgr) {
}

bool GrSurfaceContext::isGammaCorrect() const {
    return fColorSpace && !as_CSB(fColorSpace)->nonLinearBlending();
}
