/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GL
#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tests/CtsEnforcement.h"
#include "tools/gpu/gl/GLTestContext.h"

struct GrContextOptions;

DEF_GANESH_TEST_FOR_GL_CONTEXT(TextureBindingsResetTest,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
#define GL(F) GR_GL_CALL(ctxInfo.glContext()->gl(), F)

    auto dContext = ctxInfo.directContext();
    GrGpu* gpu = dContext->priv().getGpu();
    GrGLGpu* glGpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    struct Target {
        GrGLenum fName;
        GrGLenum fQuery;
    };
    SkTDArray<Target> targets;
    targets.push_back({GR_GL_TEXTURE_2D, GR_GL_TEXTURE_BINDING_2D});
    bool supportExternal;
    if ((supportExternal = glGpu->glCaps().shaderCaps()->fExternalTextureSupport)) {
        targets.push_back({GR_GL_TEXTURE_EXTERNAL, GR_GL_TEXTURE_BINDING_EXTERNAL});
    }
    bool supportRectangle;
    if ((supportRectangle = glGpu->glCaps().rectangleTextureSupport())) {
        targets.push_back({GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_BINDING_RECTANGLE});
    }
    GrGLint numUnits = 0;
    GL(GetIntegerv(GR_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &numUnits));
    SkTDArray<GrGLuint> claimedIDs;
    claimedIDs.resize(numUnits * targets.size());
    GL(GenTextures(claimedIDs.size(), claimedIDs.begin()));

    auto resetBindings = [&] {
        int i = 0;
        for (int u = 0; u < numUnits; ++u) {
            GL(ActiveTexture(GR_GL_TEXTURE0 + u));
            for (auto target : targets) {
                GL(BindTexture(target.fName, claimedIDs[i++]));
            }
        }
    };
    auto checkBindings = [&] {
        int i = 0;
        for (int u = 0; u < numUnits; ++u) {
            GL(ActiveTexture(GR_GL_TEXTURE0 + u));
            for (auto target : targets) {
                GrGLint boundID = -1;
                GL(GetIntegerv(target.fQuery, &boundID));
                if (boundID != (int) claimedIDs[i] && boundID != 0) {
                    ERRORF(reporter, "Unit %d, target 0x%04x has ID %d bound. Expected %d or 0.", u,
                           target.fName, boundID, claimedIDs[i]);
                    return;
                }
                ++i;
            }
        }
    };

    // Initialize texture unit/target combo bindings to 0.
    dContext->flushAndSubmit();
    resetBindings();
    dContext->resetContext();

    // Test creating a texture and then resetting bindings.
    static constexpr SkISize kDims = {10, 10};
    GrBackendFormat format = gpu->caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                  GrRenderable::kNo);
    auto tex = gpu->createTexture(kDims,
                                  format,
                                  GrTextureType::k2D,
                                  GrRenderable::kNo,
                                  1,
                                  skgpu::Mipmapped::kNo,
                                  skgpu::Budgeted::kNo,
                                  GrProtected::kNo,
                                  /*label=*/"TextureBindingsResetTest");
    REPORTER_ASSERT(reporter, tex);
    dContext->resetGLTextureBindings();
    checkBindings();
    resetBindings();
    dContext->resetContext();

    // Test drawing and then resetting bindings. This should force a MIP regeneration if MIP
    // maps are supported as well.
    auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, info, 1, nullptr);
    surf->getCanvas()->clear(0x80FF0000);
    auto img = surf->makeImageSnapshot();
    surf->getCanvas()->clear(SK_ColorBLUE);
    surf->getCanvas()->save();
    surf->getCanvas()->scale(0.25, 0.25);
    surf->getCanvas()->drawImage(img.get(), 0, 0, SkSamplingOptions({1.0f/3, 1.0f/3}), nullptr);
    surf->getCanvas()->restore();
    dContext->flushAndSubmit(surf.get(), GrSyncCpu::kNo);
    dContext->resetGLTextureBindings();
    checkBindings();
    resetBindings();
    dContext->resetContext();

    if (supportExternal) {
        GrBackendTexture texture2D = dContext->createBackendTexture(10,
                                                                    10,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    skgpu::Mipmapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);
        GrGLTextureInfo info2D;
        REPORTER_ASSERT(reporter, GrBackendTextures::GetGLTextureInfo(texture2D, &info2D));
        GrEGLImage eglImage = ctxInfo.glContext()->texture2DToEGLImage(info2D.fID);
        REPORTER_ASSERT(reporter, eglImage);
        GrGLTextureInfo infoExternal;
        infoExternal.fID = ctxInfo.glContext()->eglImageToExternalTexture(eglImage);
        infoExternal.fTarget = GR_GL_TEXTURE_EXTERNAL;
        infoExternal.fFormat = info2D.fFormat;
        REPORTER_ASSERT(reporter, infoExternal.fID);
        infoExternal.fProtected = info2D.fProtected;
        GrBackendTexture backendTexture =
                GrBackendTextures::MakeGL(10, 10, skgpu::Mipmapped::kNo, infoExternal);
        // Above texture creation will have messed with GL state and bindings.
        resetBindings();
        dContext->resetContext();
        img = SkImages::BorrowTextureFrom(dContext,
                                          backendTexture,
                                          kTopLeft_GrSurfaceOrigin,
                                          kRGBA_8888_SkColorType,
                                          kPremul_SkAlphaType,
                                          nullptr);
        REPORTER_ASSERT(reporter, img);
        surf->getCanvas()->drawImage(img, 0, 0);
        img.reset();
        dContext->flushAndSubmit(surf.get(), GrSyncCpu::kNo);
        dContext->resetGLTextureBindings();
        checkBindings();
        resetBindings();
        GL(DeleteTextures(1, &infoExternal.fID));
        ctxInfo.glContext()->destroyEGLImage(eglImage);
        dContext->deleteBackendTexture(texture2D);
        dContext->resetContext();
    }

    if (supportRectangle) {
        format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_RECTANGLE);
        GrBackendTexture rectangleTexture = dContext->createBackendTexture(
                10, 10, format, skgpu::Mipmapped::kNo, GrRenderable::kNo);
        if (rectangleTexture.isValid()) {
            img = SkImages::BorrowTextureFrom(dContext,
                                              rectangleTexture,
                                              kTopLeft_GrSurfaceOrigin,
                                              kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType,
                                              nullptr);
            REPORTER_ASSERT(reporter, img);
            surf->getCanvas()->drawImage(img, 0, 0);
            img.reset();
            dContext->flushAndSubmit(surf.get(), GrSyncCpu::kNo);
            dContext->resetGLTextureBindings();
            checkBindings();
            resetBindings();
            dContext->deleteBackendTexture(rectangleTexture);
        }
    }

    GL(DeleteTextures(claimedIDs.size(), claimedIDs.begin()));

#undef GL
}

#endif  // SK_GL
