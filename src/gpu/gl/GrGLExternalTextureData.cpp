/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "gl/GrGLTypes.h"

GrGLExternalTextureData::GrGLExternalTextureData(const GrGLTextureInfo& info,
                                                 sk_sp<GrSemaphore> semaphore,
                                                 GrContext* context)
        : fInfo(info)
        , fSemaphore(std::move(semaphore)) {
    SkASSERT(fSemaphore->unique());
    context->resourceProvider()->releaseOwnershipOfSemaphore(fSemaphore);
}

void GrGLExternalTextureData::attachToContext(GrContext* context) {
    context->resourceProvider()->takeOwnershipOfSemaphore(fSemaphore);
    // Ideally we don't want to get the Gpu off of the context here. However, eventually this
    // semaphore will live on a GrTexture object and the wait will be called from there. At that
    // point we can use the GrGpu already stored directly on the GrTexture.
    context->getGpu()->waitSemaphore(fSemaphore);
}
