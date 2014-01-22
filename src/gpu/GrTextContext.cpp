/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTextContext.h"

GrTextContext::GrTextContext(GrContext* context, const GrPaint& paint) : fPaint(paint) {
    fContext = context;

    const GrClipData* clipData = context->getClip();

    SkRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     context->getRenderTarget()->width(),
                                     context->getRenderTarget()->height(),
                                     &devConservativeBound);

    devConservativeBound.roundOut(&fClipRect);

    fDrawTarget = fContext->getTextTarget();
}
