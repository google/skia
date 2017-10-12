/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTest.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkGpuDevice.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "Test.h"

// Test that the correct mip map states are on the GrTextures when wrapping GrBackendTextures in
// SkImages and SkSurfaces
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrWrappedMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
        for (auto isRT : {false, true}) {
            // CreateTestingOnlyBackendTexture currently doesn't support uploading data to mip maps
            // so we don't send any. However, we pretend there is data for the checks below which is
            // fine since we are never actually using these textures for any work on the gpu.
            GrBackendObject backendHandle = context->getGpu()->createTestingOnlyBackendTexture(
                    nullptr, 8, 8, kRGBA_8888_GrPixelConfig, isRT, mipMapped);

            GrBackend backend = context->contextPriv().getBackend();
            GrBackendTexture backendTex = GrTest::CreateBackendTexture(backend,
                                                                       8,
                                                                       8,
                                                                       kRGBA_8888_GrPixelConfig,
                                                                       mipMapped,
                                                                       backendHandle);

            GrTextureProxy* proxy;
            sk_sp<SkImage> image;
            if (isRT) {
                sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(
                                                                           context,
                                                                           backendTex,
                                                                           kTopLeft_GrSurfaceOrigin,
                                                                           0,
                                                                           nullptr,
                                                                           nullptr);

                SkGpuDevice* device = ((SkSurface_Gpu*)surface.get())->getDevice();
                proxy = device->accessRenderTargetContext()->asTextureProxy();
            } else {
                image = SkImage::MakeFromTexture(context, backendTex,
                                                 kTopLeft_GrSurfaceOrigin,
                                                 kPremul_SkAlphaType, nullptr);
                proxy = as_IB(image)->peekProxy();
            }
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->priv().isInstantiated());

            GrTexture* texture = proxy->priv().peekTexture();
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            if (GrMipMapped::kYes == mipMapped) {
                REPORTER_ASSERT(reporter, texture->texturePriv().hasMipMaps());
                if (isRT) {
                    REPORTER_ASSERT(reporter, texture->texturePriv().mipMapsAreDirty());
                } else {
                    REPORTER_ASSERT(reporter, !texture->texturePriv().mipMapsAreDirty());
                }
            } else {
                REPORTER_ASSERT(reporter, !texture->texturePriv().hasMipMaps());
            }
            context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
        }
    }
}

#endif
