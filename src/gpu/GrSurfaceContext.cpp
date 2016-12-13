/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceContext.h"

#include "../private/GrAuditTrail.h"


// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrContext* context,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : fContext(context)
    , fAuditTrail(auditTrail)
#ifdef SK_DEBUG
    , fSingleOwner(singleOwner)
#endif
{
}

bool GrSurfaceContext::writePixels(int left, int top, int width, int height,
                                   GrPixelConfig config, const void* buffer, size_t rowBytes,
                                   uint32_t pixelOpsFlags) {
#if 0
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (nullptr == context) {
        return false;
    }
    return context->writeSurfacePixels(this, left, top, width, height, config, buffer,
                                       rowBytes, pixelOpsFlags);
#endif
    return false;
}

bool GrSurfaceContext::readPixels(int left, int top, int width, int height,
                                  GrPixelConfig config, void* buffer, size_t rowBytes,
                                  uint32_t pixelOpsFlags) {
#if 0
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (nullptr == context) {
        return false;
    }
    return context->readSurfacePixels(this, left, top, width, height, config, buffer,
                                      rowBytes, pixelOpsFlags);
#endif
    return false;
}