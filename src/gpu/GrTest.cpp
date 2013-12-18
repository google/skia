
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrGpu.h"
#include "GrResourceCache.h"

void GrTestTarget::init(GrContext* ctx, GrDrawTarget* target) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawTarget.reset(SkRef(target));

    SkNEW_IN_TLAZY(&fASR, GrDrawTarget::AutoStateRestore, (target, GrDrawTarget::kReset_ASRInit));
    SkNEW_IN_TLAZY(&fACR, GrDrawTarget::AutoClipRestore, (target));
    SkNEW_IN_TLAZY(&fAGP, GrDrawTarget::AutoGeometryPush, (target));
}

void GrContext::getTestTarget(GrTestTarget* tar) {
    this->flush();
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    tar->init(this, fGpu);
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::setMaxTextureSizeOverride(int maxTextureSizeOverride) {
    fMaxTextureSizeOverride = maxTextureSizeOverride;
}

void GrContext::purgeAllUnlockedResources() {
    fTextureCache->purgeAllUnlocked();
}
