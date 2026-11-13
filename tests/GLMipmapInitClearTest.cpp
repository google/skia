/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// When textures must be initialized, creating an uninitialized mipmapped texture should initialize
// only the base level and leave the mipmaps dirty, so they are regenerated before being sampled.
// The test hooks the GL calls GrGLGpu uses to clear a level (FBO attachment, glClearTexImage,
// glTexSubImage2D) and checks the levels GrGpu asked for, whichever mechanism the driver uses.

#include "tests/Test.h"

#ifdef SK_GL

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <set>

namespace {
struct LevelRecorder {
    sk_sp<const GrGLInterface> fBase;
    bool fRecording = false;
    std::set<int> fLevels;

    void record(GrGLint level) {
        if (fRecording) {
            fLevels.insert(level);
        }
    }
    SkString levels() const {
        SkString str;
        for (int level : fLevels) {
            str.appendf("%s%d", str.isEmpty() ? "" : ", ", level);
        }
        return SkStringPrintf("{%s}", str.c_str());
    }
};
}  // namespace

DEF_GANESH_TEST_FOR_GL_CONTEXT(GLMipmapInitClear, reporter, ctxInfo, CtsEnforcement::kNever) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    // We share the GL context with the factory's GrDirectContext. Get it out of the way first.
    ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);

    GrContextOptions options = ctxInfo.options();
    options.fClearAllTextures = true;  // Forces shouldInitializeTextures(), as on WebGL.

    // Must outlive the GrDirectContext, which calls through the hooks until it is destroyed.
    LevelRecorder recorder;
    recorder.fBase = sk_ref_sp(glCtx->gl());
    sk_sp<GrGLInterface> gl(new GrGLInterface(*recorder.fBase));
    // GrGLFunction has a limited capture size, so each hook captures only the recorder pointer.
    LevelRecorder* state = &recorder;
    gl->fFunctions.fFramebufferTexture2D =
            [state](GrGLenum t, GrGLenum a, GrGLenum tt, GrGLuint texture, GrGLint level) {
                if (texture != 0) {  // Detaching doesn't touch a level.
                    state->record(level);
                }
                state->fBase->fFunctions.fFramebufferTexture2D(t, a, tt, texture, level);
            };
    if (recorder.fBase->fFunctions.fClearTexImage) {
        gl->fFunctions.fClearTexImage = [state](GrGLuint texture, GrGLint level, auto... rest) {
            state->record(level);
            state->fBase->fFunctions.fClearTexImage(texture, level, rest...);
        };
    }
    gl->fFunctions.fTexSubImage2D = [state](GrGLenum target, GrGLint level, auto... rest) {
        state->record(level);
        state->fBase->fFunctions.fTexSubImage2D(target, level, rest...);
    };

    sk_sp<GrDirectContext> dContext = GrDirectContexts::MakeGL(gl, options);
    if (!dContext) {
        ERRORF(reporter, "Could not create a GrDirectContext with the hooked interface");
        return;
    }
    GrGpu* gpu = dContext->priv().getGpu();
    const GrBackendFormat format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
    struct Case {
        const char* fName;
        GrRenderable fRenderable;
        skgpu::Mipmapped fMipmapped;
        // A level 0 FBO clear on a render target uses its own FBO without a hooked call, so then
        // we only check that no level above 0 was touched.
        bool fExpectBaseLevelCall;
    };
    const Case kCases[] = {
            {"mipmapped", GrRenderable::kNo, skgpu::Mipmapped::kYes, true},
            // The shape of GrCopyBaseMipMapToView's copy target.
            {"mipmapped renderable", GrRenderable::kYes, skgpu::Mipmapped::kYes, false},
            // Control: a single level texture is still initialized.
            {"non-mipmapped", GrRenderable::kNo, skgpu::Mipmapped::kNo, true},
    };
    for (const Case& c : kCases) {
        recorder.fLevels.clear();
        recorder.fRecording = true;
        sk_sp<GrTexture> tex = gpu->createTexture({256, 256},
                                                  format,
                                                  GrTextureType::k2D,
                                                  c.fRenderable,
                                                  1,
                                                  c.fMipmapped,
                                                  skgpu::Budgeted::kNo,
                                                  GrProtected::kNo,
                                                  "GLMipmapInitClearTest");
        recorder.fRecording = false;
        if (!tex) {
            ERRORF(reporter, "%s: could not create the texture", c.fName);
            continue;
        }
        const bool levelsOk = c.fExpectBaseLevelCall
                                      ? recorder.fLevels == std::set<int>{0}
                                      : recorder.fLevels.empty() || *recorder.fLevels.rbegin() == 0;
        REPORTER_ASSERT(reporter,
                        levelsOk,
                        "%s: initialized levels %s, expected %s",
                        c.fName,
                        recorder.levels().c_str(),
                        c.fExpectBaseLevelCall ? "{0}" : "none above 0");
        if (c.fMipmapped == skgpu::Mipmapped::kYes) {
            REPORTER_ASSERT(reporter,
                            tex->mipmapsAreDirty(),
                            "%s: mipmaps marked clean after initializing levels %s",
                            c.fName,
                            recorder.levels().c_str());
        }
    }

    dContext.reset();
    // We made GL calls behind the factory context's back.
    glCtx->makeCurrent();
    ctxInfo.directContext()->resetContext();
}

#endif  // SK_GL
