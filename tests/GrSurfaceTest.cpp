/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "GrTexture.h"
#include "GrSurfacePriv.h"
#include "SkTypes.h"
#include "Test.h"

DEF_GPUTEST(GrSurface, reporter, factory) {
    GrContext* context = factory->get(GrContextFactory::kNull_GLContextType);
    if (context) {
        GrTextureDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = 256;
        desc.fHeight = 256;
        desc.fSampleCnt = 0;
        GrSurface* texRT1 = context->createUncachedTexture(desc, NULL, 0);
        GrSurface* texRT2 = context->createUncachedTexture(desc, NULL, 0);
        desc.fFlags = kNone_GrTextureFlags;
        GrSurface* tex1 = context->createUncachedTexture(desc, NULL, 0);

        REPORTER_ASSERT(reporter, texRT1->surfacePriv().isSameAs(texRT1));
        REPORTER_ASSERT(reporter, texRT1->surfacePriv().isSameAs(texRT1->asRenderTarget()));
        REPORTER_ASSERT(reporter, texRT1->asRenderTarget()->surfacePriv().isSameAs(texRT1));
        REPORTER_ASSERT(reporter, !texRT2->surfacePriv().isSameAs(texRT1));
        REPORTER_ASSERT(reporter, !texRT2->asRenderTarget()->surfacePriv().isSameAs(texRT1));
        REPORTER_ASSERT(reporter, !texRT2->surfacePriv().isSameAs(texRT1->asRenderTarget()));
        REPORTER_ASSERT(reporter, !texRT2->surfacePriv().isSameAs(tex1));
        REPORTER_ASSERT(reporter, !texRT2->asRenderTarget()->surfacePriv().isSameAs(tex1));

        GrBackendTextureDesc backendDesc;
        backendDesc.fConfig = kSkia8888_GrPixelConfig;
        backendDesc.fFlags = kRenderTarget_GrBackendTextureFlag;
        backendDesc.fWidth = 256;
        backendDesc.fHeight = 256;
        backendDesc.fSampleCnt = 0;
        backendDesc.fTextureHandle = 5;
        GrSurface* externalTexRT = context->wrapBackendTexture(backendDesc);
        REPORTER_ASSERT(reporter, externalTexRT->surfacePriv().isSameAs(externalTexRT));
        REPORTER_ASSERT(reporter,
                        externalTexRT->surfacePriv().isSameAs(externalTexRT->asRenderTarget()));
        REPORTER_ASSERT(reporter,
                        externalTexRT->asRenderTarget()->surfacePriv().isSameAs(externalTexRT));
        REPORTER_ASSERT(reporter, !externalTexRT->surfacePriv().isSameAs(texRT1));
        REPORTER_ASSERT(reporter, !externalTexRT->asRenderTarget()->surfacePriv().isSameAs(texRT1));

        texRT1->unref();
        texRT2->unref();
        tex1->unref();
        externalTexRT->unref();
    }
}

#endif
