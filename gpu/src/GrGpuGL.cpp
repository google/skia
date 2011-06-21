/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "GrGpuGL.h"
#include "GrMemory.h"
#include "GrTypes.h"

static const GrGLuint GR_MAX_GLUINT = ~0;
static const GrGLint  GR_INVAL_GLINT = ~0;

// we use a spare texture unit to avoid
// mucking with the state of any of the stages.
static const int SPARE_TEX_UNIT = GrGpuGL::kNumStages;

#define SKIP_CACHE_CHECK    true

static const GrGLenum gXfermodeCoeff2Blend[] = {
    GR_GL_ZERO,
    GR_GL_ONE,
    GR_GL_SRC_COLOR,
    GR_GL_ONE_MINUS_SRC_COLOR,
    GR_GL_DST_COLOR,
    GR_GL_ONE_MINUS_DST_COLOR,
    GR_GL_SRC_ALPHA,
    GR_GL_ONE_MINUS_SRC_ALPHA,
    GR_GL_DST_ALPHA,
    GR_GL_ONE_MINUS_DST_ALPHA,
    GR_GL_CONSTANT_COLOR,
    GR_GL_ONE_MINUS_CONSTANT_COLOR,
    GR_GL_CONSTANT_ALPHA,
    GR_GL_ONE_MINUS_CONSTANT_ALPHA,

    // extended blend coeffs
    GR_GL_SRC1_COLOR,
    GR_GL_ONE_MINUS_SRC1_COLOR,
    GR_GL_SRC1_ALPHA,
    GR_GL_ONE_MINUS_SRC1_ALPHA,
};

bool GrGpuGL::BlendCoeffReferencesConstant(GrBlendCoeff coeff) {
    static const bool gCoeffReferencesBlendConst[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,

        // extended blend coeffs
        false,
        false,
        false,
        false,
    };
    return gCoeffReferencesBlendConst[coeff];
    GR_STATIC_ASSERT(kTotalBlendCoeffCount == GR_ARRAY_COUNT(gCoeffReferencesBlendConst));

    GR_STATIC_ASSERT(0 == kZero_BlendCoeff);
    GR_STATIC_ASSERT(1 == kOne_BlendCoeff);
    GR_STATIC_ASSERT(2 == kSC_BlendCoeff);
    GR_STATIC_ASSERT(3 == kISC_BlendCoeff);
    GR_STATIC_ASSERT(4 == kDC_BlendCoeff);
    GR_STATIC_ASSERT(5 == kIDC_BlendCoeff);
    GR_STATIC_ASSERT(6 == kSA_BlendCoeff);
    GR_STATIC_ASSERT(7 == kISA_BlendCoeff);
    GR_STATIC_ASSERT(8 == kDA_BlendCoeff);
    GR_STATIC_ASSERT(9 == kIDA_BlendCoeff);
    GR_STATIC_ASSERT(10 == kConstC_BlendCoeff);
    GR_STATIC_ASSERT(11 == kIConstC_BlendCoeff);
    GR_STATIC_ASSERT(12 == kConstA_BlendCoeff);
    GR_STATIC_ASSERT(13 == kIConstA_BlendCoeff);

    GR_STATIC_ASSERT(14 == kS2C_BlendCoeff);
    GR_STATIC_ASSERT(15 == kIS2C_BlendCoeff);
    GR_STATIC_ASSERT(16 == kS2A_BlendCoeff);
    GR_STATIC_ASSERT(17 == kIS2A_BlendCoeff);

    // assertion for gXfermodeCoeff2Blend have to be in GrGpu scope
    GR_STATIC_ASSERT(kTotalBlendCoeffCount == GR_ARRAY_COUNT(gXfermodeCoeff2Blend));
}

///////////////////////////////////////////////////////////////////////////////

void GrGpuGL::AdjustTextureMatrix(const GrGLTexture* texture,
                                  GrSamplerState::SampleMode mode,
                                  GrMatrix* matrix) {
    GrAssert(NULL != texture);
    GrAssert(NULL != matrix);
    if (GR_Scalar1 != texture->contentScaleX() ||
        GR_Scalar1 != texture->contentScaleY()) {
        if (GrSamplerState::kRadial_SampleMode == mode) {
            GrMatrix scale;
            scale.setScale(texture->contentScaleX(), texture->contentScaleX());
            matrix->postConcat(scale);
        } else if (GrSamplerState::kNormal_SampleMode == mode) {
            GrMatrix scale;
            scale.setScale(texture->contentScaleX(), texture->contentScaleY());
            matrix->postConcat(scale);
        } else {
            GrPrintf("We haven't handled NPOT adjustment for other sample modes!");
        }
    }
    GrGLTexture::Orientation orientation = texture->orientation();
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        GrMatrix invY;
        invY.setAll(GR_Scalar1, 0,           0,
                    0,          -GR_Scalar1, GR_Scalar1,
                    0,          0,           GrMatrix::I()[8]);
        matrix->postConcat(invY);
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
    }
}

bool GrGpuGL::TextureMatrixIsIdentity(const GrGLTexture* texture,
                                      const GrSamplerState& sampler) {
    GrAssert(NULL != texture);
    if (!sampler.getMatrix().isIdentity()) {
        return false;
    }
    if (GR_Scalar1 != texture->contentScaleX() ||
        GR_Scalar1 != texture->contentScaleY()) {
        return false;
    }
    GrGLTexture::Orientation orientation = texture->orientation();
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        return false;
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool gPrintStartupSpew;

static bool fbo_test(int w, int h) {

    GrGLint savedFBO;
    GrGLint savedTexUnit;
    GR_GL_GetIntegerv(GR_GL_ACTIVE_TEXTURE, &savedTexUnit);
    GR_GL_GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &savedFBO);

    GR_GL(ActiveTexture(GR_GL_TEXTURE0 + SPARE_TEX_UNIT));

    GrGLuint testFBO;
    GR_GL(GenFramebuffers(1, &testFBO));
    GR_GL(BindFramebuffer(GR_GL_FRAMEBUFFER, testFBO));
    GrGLuint testRTTex;
    GR_GL(GenTextures(1, &testRTTex));
    GR_GL(BindTexture(GR_GL_TEXTURE_2D, testRTTex));
    // some implementations require texture to be mip-map complete before
    // FBO with level 0 bound as color attachment will be framebuffer complete.
    GR_GL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GR_GL(TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA, w, h,
                     0, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, NULL));
    GR_GL(BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                               GR_GL_TEXTURE_2D, testRTTex, 0));
    GrGLenum status = GR_GL(CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    GR_GL(DeleteFramebuffers(1, &testFBO));
    GR_GL(DeleteTextures(1, &testRTTex));

    GR_GL(ActiveTexture(savedTexUnit));
    GR_GL(BindFramebuffer(GR_GL_FRAMEBUFFER, savedFBO));

    return status == GR_GL_FRAMEBUFFER_COMPLETE;
}

static bool probe_for_npot_render_target_support(bool hasNPOTTextureSupport) {

    /* Experimentation has found that some GLs that support NPOT textures
       do not support FBOs with a NPOT texture. They report "unsupported" FBO
       status. I don't know how to explicitly query for this. Do an
       experiment. Note they may support NPOT with a renderbuffer but not a
       texture. Presumably, the implementation bloats the renderbuffer
       internally to the next POT.
     */
    if (hasNPOTTextureSupport) {
        return fbo_test(200, 200);
    }
    return false;
}

static int probe_for_min_render_target_height(bool hasNPOTRenderTargetSupport,
                                              int maxRenderTargetSize) {
    /* The iPhone 4 has a restriction that for an FBO with texture color
       attachment with height <= 8 then the width must be <= height. Here
       we look for such a limitation.
     */
    if (gPrintStartupSpew) {
        GrPrintf("Small height FBO texture experiments\n");
    }
    int minRenderTargetHeight = GR_INVAL_GLINT;
    for (GrGLuint i = 1; i <= 256; hasNPOTRenderTargetSupport ? ++i : i *= 2) {
        GrGLuint w = maxRenderTargetSize;
        GrGLuint h = i;
        if (fbo_test(w, h)) {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: PASSED\n", w, h);
            }
            minRenderTargetHeight = i;
            break;
        } else {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: FAILED\n", w, h);
            }
        }
    }
    GrAssert(GR_INVAL_GLINT != minRenderTargetHeight);

    return minRenderTargetHeight;
}

static int probe_for_min_render_target_width(bool hasNPOTRenderTargetSupport,
                                              int maxRenderTargetSize) {

    if (gPrintStartupSpew) {
        GrPrintf("Small width FBO texture experiments\n");
    }
    int minRenderTargetWidth = GR_INVAL_GLINT;
    for (GrGLuint i = 1; i <= 256; hasNPOTRenderTargetSupport ? i *= 2 : ++i) {
        GrGLuint w = i;
        GrGLuint h = maxRenderTargetSize;
        if (fbo_test(w, h)) {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: PASSED\n", w, h);
            }
            minRenderTargetWidth = i;
            break;
        } else {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: FAILED\n", w, h);
            }
        }
    }
    GrAssert(GR_INVAL_GLINT != minRenderTargetWidth);

    return minRenderTargetWidth;
}


GrGpuGL::GrGpuGL() {

    if (gPrintStartupSpew) {
        GrPrintf("------------------------- create GrGpuGL %p --------------\n",
                 this);
        GrPrintf("------ VENDOR %s\n",
                 GrGLGetGLInterface()->fGetString(GR_GL_VENDOR));
        GrPrintf("------ RENDERER %s\n",
                 GrGLGetGLInterface()->fGetString(GR_GL_RENDERER));
        GrPrintf("------ VERSION %s\n",
                 GrGLGetGLInterface()->fGetString(GR_GL_VERSION));
        GrPrintf("------ EXTENSIONS\n %s \n",
                 GrGLGetGLInterface()->fGetString(GR_GL_EXTENSIONS));
    }

    GrGLClearErr();

    resetDirtyFlags();

    GrGLint maxTextureUnits;
    // check FS and fixed-function texture unit limits
    // we only use textures in the fragment stage currently.
    // checks are > to make sure we have a spare unit.
    if (GR_GL_SUPPORT_DESKTOP || GR_GL_SUPPORT_ES2) {
        GR_GL_GetIntegerv(GR_GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        GrAssert(maxTextureUnits > kNumStages);
    }
    if (GR_GL_SUPPORT_DESKTOP || GR_GL_SUPPORT_ES1) {
        GR_GL_GetIntegerv(GR_GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
        GrAssert(maxTextureUnits > kNumStages);
    }
    if (GR_GL_SUPPORT_ES2) {
        GR_GL_GetIntegerv(GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fMaxFragmentUniformVectors);
    } else if (GR_GL_SUPPORT_DESKTOP) {
        GrGLint max;
        GR_GL_GetIntegerv(GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fMaxFragmentUniformVectors = max / 4;
    } else {
        fMaxFragmentUniformVectors = 16;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Check for supported features.

    int major, minor;
    gl_version(&major, &minor);

    GrGLint numFormats;
    GR_GL_GetIntegerv(GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
    GrAutoSTMalloc<10, GrGLint> formats(numFormats);
    GR_GL_GetIntegerv(GR_GL_COMPRESSED_TEXTURE_FORMATS, formats);
    for (int i = 0; i < numFormats; ++i) {
        if (formats[i] == GR_GL_PALETTE8_RGBA8) {
            f8bitPaletteSupport = true;
            break;
        }
    }

    if (gPrintStartupSpew) {
        GrPrintf("Palette8 support: %s\n", (f8bitPaletteSupport ? "YES" : "NO"));
    }

    GR_STATIC_ASSERT(0 == kNone_GrAALevel);
    GR_STATIC_ASSERT(1 == kLow_GrAALevel);
    GR_STATIC_ASSERT(2 == kMed_GrAALevel);
    GR_STATIC_ASSERT(3 == kHigh_GrAALevel);

    memset(fAASamples, 0, sizeof(fAASamples));
    fMSFBOType = kNone_MSFBO;
    if (GR_GL_SUPPORT_ES) {
       if (has_gl_extension("GL_CHROMIUM_framebuffer_multisample")) {
           // chrome's extension is equivalent to the EXT msaa
           // and fbo_blit extensions.
            fMSFBOType = kDesktopEXT_MSFBO;
       } else if (has_gl_extension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kAppleES_MSFBO;
        }
    } else {
        GrAssert(GR_GL_SUPPORT_DESKTOP);
        if ((major >= 3) || has_gl_extension("GL_ARB_framebuffer_object")) {
            fMSFBOType = kDesktopARB_MSFBO;
        } else if (has_gl_extension("GL_EXT_framebuffer_multisample") &&
                   has_gl_extension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = kDesktopEXT_MSFBO;
        }
    }
    if (gPrintStartupSpew) {
        switch (fMSFBOType) {
            case kNone_MSFBO:
                GrPrintf("MSAA Support: NONE\n");
                break;
            case kDesktopARB_MSFBO:
                GrPrintf("MSAA Support: DESKTOP ARB.\n");
                break;
            case kDesktopEXT_MSFBO:
                GrPrintf("MSAA Support: DESKTOP EXT.\n");
                break;
            case kAppleES_MSFBO:
                GrPrintf("MSAA Support: APPLE ES.\n");
                break;
        }
    }

    if (kNone_MSFBO != fMSFBOType) {
        GrGLint maxSamples;
        GR_GL_GetIntegerv(GR_GL_MAX_SAMPLES, &maxSamples);
        if (maxSamples > 1 ) {
            fAASamples[kNone_GrAALevel] = 0;
            fAASamples[kLow_GrAALevel] = GrMax(2,
                                               GrFixedFloorToInt((GR_FixedHalf) *
                                               maxSamples));
            fAASamples[kMed_GrAALevel] = GrMax(2,
                                               GrFixedFloorToInt(((GR_Fixed1*3)/4) *
                                               maxSamples));
            fAASamples[kHigh_GrAALevel] = maxSamples;
        }
        if (gPrintStartupSpew) {
            GrPrintf("\tMax Samples: %d\n", maxSamples);
        }
    }
    fFSAASupport = fAASamples[kHigh_GrAALevel] > 0;

    if (GR_GL_SUPPORT_DESKTOP) {
        fHasStencilWrap = (major >= 2 || (major == 1 && minor >= 4)) ||
                          has_gl_extension("GL_EXT_stencil_wrap");
    } else {
        fHasStencilWrap = (major >= 2) || has_gl_extension("GL_OES_stencil_wrap");
    }
    if (gPrintStartupSpew) {
        GrPrintf("Stencil Wrap: %s\n", (fHasStencilWrap ? "YES" : "NO"));
    }

    if (GR_GL_SUPPORT_DESKTOP) {
        // we could also look for GL_ATI_separate_stencil extension or
        // GL_EXT_stencil_two_side but they use different function signatures
        // than GL2.0+ (and than each other).
        fTwoSidedStencilSupport = (major >= 2);
        // supported on GL 1.4 and higher or by extension
        fStencilWrapOpsSupport = (major > 1) ||
                                 ((1 == major) && (minor >= 4)) ||
                                  has_gl_extension("GL_EXT_stencil_wrap");
    } else {
        // ES 2 has two sided stencil but 1.1 doesn't. There doesn't seem to be
        // an ES1 extension.
        fTwoSidedStencilSupport = (major >= 2);
        // stencil wrap support is in ES2, ES1 requires extension.
        fStencilWrapOpsSupport = (major > 1) ||
                                  has_gl_extension("GL_OES_stencil_wrap");
    }
    if (gPrintStartupSpew) {
        GrPrintf("Stencil Caps: TwoSide: %s, Wrap: %s\n",
                (fTwoSidedStencilSupport ? "YES" : "NO"),
                (fStencilWrapOpsSupport ? "YES" : "NO"));
    }

    if (GR_GL_SUPPORT_DESKTOP) {
        fRGBA8Renderbuffer = true;
    } else {
        fRGBA8Renderbuffer = has_gl_extension("GL_OES_rgb8_rgba8");
    }
    if (gPrintStartupSpew) {
        GrPrintf("RGBA Renderbuffer: %s\n", (fRGBA8Renderbuffer ? "YES" : "NO"));
    }


    if (GR_GL_SUPPORT_ES) {
        if (GR_GL_32BPP_COLOR_FORMAT == GR_GL_BGRA) {
            GrAssert(has_gl_extension("GL_EXT_texture_format_BGRA8888"));
        }
    }

    if (GR_GL_SUPPORT_DESKTOP) {
        fBufferLockSupport = true; // we require VBO support and the desktop VBO
                                   // extension includes glMapBuffer.
    } else {
        fBufferLockSupport = has_gl_extension("GL_OES_mapbuffer");
    }

    if (gPrintStartupSpew) {
        GrPrintf("Map Buffer: %s\n", (fBufferLockSupport ? "YES" : "NO"));
    }

    if (GR_GL_SUPPORT_DESKTOP) {
        if (major >= 2 || has_gl_extension("GL_ARB_texture_non_power_of_two")) {
            fNPOTTextureTileSupport = true;
            fNPOTTextureSupport = true;
        } else {
            fNPOTTextureTileSupport = false;
            fNPOTTextureSupport = false;
        }
    } else {
        if (major >= 2) {
            fNPOTTextureSupport = true;
            fNPOTTextureTileSupport = has_gl_extension("GL_OES_texture_npot");
        } else {
            fNPOTTextureSupport =
                        has_gl_extension("GL_APPLE_texture_2D_limited_npot");
            fNPOTTextureTileSupport = false;
        }
    }

    fAALineSupport = GR_GL_SUPPORT_DESKTOP;

    ////////////////////////////////////////////////////////////////////////////
    // Experiments to determine limitations that can't be queried.
    // TODO: Make these a preprocess that generate some compile time constants.
    // TODO: probe once at startup, rather than once per context creation.

    int expectNPOTTargets = GrGLGetGLInterface()->fNPOTRenderTargetSupport;
    if (expectNPOTTargets == kProbe_GrGLCapability) {
        fNPOTRenderTargetSupport =
            probe_for_npot_render_target_support(fNPOTTextureSupport);
    } else {
        GrAssert(expectNPOTTargets == 0 || expectNPOTTargets == 1);
        fNPOTRenderTargetSupport = static_cast<bool>(expectNPOTTargets);
    }

    if (gPrintStartupSpew) {
        if (fNPOTTextureSupport) {
            GrPrintf("NPOT textures supported\n");
            if (fNPOTTextureTileSupport) {
                GrPrintf("NPOT texture tiling supported\n");
            } else {
                GrPrintf("NPOT texture tiling NOT supported\n");
            }
            if (fNPOTRenderTargetSupport) {
                GrPrintf("NPOT render targets supported\n");
            } else {
                GrPrintf("NPOT render targets NOT supported\n");
            }
        } else {
            GrPrintf("NPOT textures NOT supported\n");
        }
    }

    GR_GL_GetIntegerv(GR_GL_MAX_TEXTURE_SIZE, &fMaxTextureSize);
    GR_GL_GetIntegerv(GR_GL_MAX_RENDERBUFFER_SIZE, &fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = GrMin(fMaxTextureSize, fMaxRenderTargetSize);

    fMinRenderTargetHeight = GrGLGetGLInterface()->fMinRenderTargetHeight;
    if (fMinRenderTargetHeight == kProbe_GrGLCapability) {
        fMinRenderTargetHeight =
            probe_for_min_render_target_height(fNPOTRenderTargetSupport,
                                               fMaxRenderTargetSize);
    }

    fMinRenderTargetWidth = GrGLGetGLInterface()->fMinRenderTargetWidth;
    if (fMinRenderTargetWidth == kProbe_GrGLCapability) {
        fMinRenderTargetWidth =
            probe_for_min_render_target_width(fNPOTRenderTargetSupport,
                                              fMaxRenderTargetSize);
    }

    fLastSuccessfulStencilFmtIdx = 0;
}

GrGpuGL::~GrGpuGL() {
}

void GrGpuGL::resetContext() {
    // We detect cases when blending is effectively off
    fHWBlendDisabled = false;
    GR_GL(Enable(GR_GL_BLEND));

    // we don't use the zb at all
    GR_GL(Disable(GR_GL_DEPTH_TEST));
    GR_GL(DepthMask(GR_GL_FALSE));

    GR_GL(Disable(GR_GL_CULL_FACE));
    GR_GL(FrontFace(GR_GL_CCW));
    fHWDrawState.fDrawFace = kBoth_DrawFace;

    GR_GL(Disable(GR_GL_DITHER));
    if (GR_GL_SUPPORT_DESKTOP) {
        GR_GL(Disable(GR_GL_LINE_SMOOTH));
        GR_GL(Disable(GR_GL_POINT_SMOOTH));
        GR_GL(Disable(GR_GL_MULTISAMPLE));
        fHWAAState.fMSAAEnabled = false;
        fHWAAState.fSmoothLineEnabled = false;
    }

    GR_GL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.fFlagBits = 0;

    // we only ever use lines in hairline mode
    GR_GL(LineWidth(1));

    // invalid
    fActiveTextureUnitIdx = -1;

    // illegal values
    fHWDrawState.fSrcBlend = (GrBlendCoeff)-1;
    fHWDrawState.fDstBlend = (GrBlendCoeff)-1;

    fHWDrawState.fBlendConstant = 0x00000000;
    GR_GL(BlendColor(0,0,0,0));

    fHWDrawState.fColor = GrColor_ILLEGAL;

    fHWDrawState.fViewMatrix = GrMatrix::InvalidMatrix();

    for (int s = 0; s < kNumStages; ++s) {
        fHWDrawState.fTextures[s] = NULL;
        fHWDrawState.fSamplerStates[s].setRadial2Params(-GR_ScalarMax,
                                                        -GR_ScalarMax,
                                                        true);

        fHWDrawState.fSamplerStates[s].setMatrix(GrMatrix::InvalidMatrix());
    }

    fHWBounds.fScissorRect.invalidate();
    fHWBounds.fScissorEnabled = false;
    GR_GL(Disable(GR_GL_SCISSOR_TEST));
    fHWBounds.fViewportRect.invalidate();

    fHWDrawState.fStencilSettings.invalidate();
    fHWStencilClip = false;
    fClipState.fClipIsDirty = true;

    fHWGeometryState.fIndexBuffer = NULL;
    fHWGeometryState.fVertexBuffer = NULL;
    
    fHWGeometryState.fArrayPtrsDirty = true;

    GR_GL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.fRenderTarget = NULL;
}

GrResource* GrGpuGL::onCreatePlatformSurface(const GrPlatformSurfaceDesc& desc) {

    bool isTexture = kTexture_GrPlatformSurfaceType == desc.fSurfaceType ||
                     kTextureRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType;
    bool isRenderTarget = kRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType ||
                          kTextureRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType;

    GrGLRenderTarget::GLRenderTargetIDs rtIDs;
    if (isRenderTarget) {
        rtIDs.fRTFBOID = desc.fPlatformRenderTarget;
        if (kIsMultisampled_GrPlatformRenderTargetFlagBit & desc.fRenderTargetFlags) {
            if (kGrCanResolve_GrPlatformRenderTargetFlagBit  & desc.fRenderTargetFlags) {
                rtIDs.fTexFBOID = desc.fPlatformResolveDestination;
            } else {
                GrAssert(!isTexture); // this should have been filtered by GrContext
                rtIDs.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
            }
        } else {
            rtIDs.fTexFBOID = desc.fPlatformRenderTarget;
        }
        // we don't know what the RB ids are without glGets and we don't care
        // since we aren't responsible for deleting them.
        rtIDs.fStencilRenderbufferID = 0;
        rtIDs.fMSColorRenderbufferID = 0;

        rtIDs.fOwnIDs = false;
    } else {
        rtIDs.reset();
    }

    if (isTexture) {
        GrGLTexture::GLTextureDesc texDesc;
        GrGLenum dontCare;
        if (!canBeTexture(desc.fConfig, &dontCare,
                         &texDesc.fUploadFormat,
                         &texDesc.fUploadType)) {
            return NULL;
        }

        GrGLTexture::TexParams params;

        texDesc.fAllocWidth  = texDesc.fContentWidth  = desc.fWidth;
        texDesc.fAllocHeight = texDesc.fContentHeight = desc.fHeight;

        texDesc.fFormat             = desc.fConfig;
        texDesc.fOrientation        = GrGLTexture::kBottomUp_Orientation;
        texDesc.fStencilBits        = desc.fStencilBits;
        texDesc.fTextureID          = desc.fPlatformTexture;
        texDesc.fUploadByteCount    = GrBytesPerPixel(desc.fConfig);
        texDesc.fOwnsID             = false;

        params.invalidate(); // rather than do glGets.

        return new GrGLTexture(this, texDesc, rtIDs, params);
    } else {
        GrGLIRect viewport;
        viewport.fLeft   = 0;
        viewport.fBottom = 0;
        viewport.fWidth  = desc.fWidth;
        viewport.fHeight = desc.fHeight;

        return new GrGLRenderTarget(this, rtIDs, NULL, desc.fStencilBits,
                                    kIsMultisampled_GrPlatformRenderTargetFlagBit & desc.fRenderTargetFlags,
                                    viewport, NULL);
    }
}

GrRenderTarget* GrGpuGL::onCreateRenderTargetFrom3DApiState() {

    GrGLRenderTarget::GLRenderTargetIDs rtIDs;

    GR_GL_GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, (GrGLint*)&rtIDs.fRTFBOID);
    rtIDs.fTexFBOID = rtIDs.fRTFBOID;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fStencilRenderbufferID = 0;

    GrGLIRect viewport;
    viewport.setFromGLViewport();
    GrGLuint stencilBits;
    GR_GL_GetIntegerv(GR_GL_STENCIL_BITS, (GrGLint*)&stencilBits);

    GrGLint samples;
    GR_GL_GetIntegerv(GR_GL_SAMPLES, &samples);

    rtIDs.fOwnIDs = false;

    return new GrGLRenderTarget(this, rtIDs, NULL, stencilBits, 
                                (samples > 0), viewport, NULL);
}

///////////////////////////////////////////////////////////////////////////////

static const GrGLuint UNKNOWN_BITS = ~0;

struct StencilFormat {
    GrGLenum  fEnum;
    GrGLuint  fBits;
    bool      fPacked;
};

const StencilFormat* GrGLStencilFormats(int* count) {
    // defines stencil formats from more to less preferred
    static const StencilFormat desktopStencilFormats[] = {
        {GR_GL_STENCIL_INDEX8,     8,            false},
        {GR_GL_STENCIL_INDEX16,    16,           false},
        {GR_GL_DEPTH24_STENCIL8,   8,            true },
        {GR_GL_STENCIL_INDEX4,     4,            false},
        {GR_GL_STENCIL_INDEX,      UNKNOWN_BITS, false},
        {GR_GL_DEPTH_STENCIL,      UNKNOWN_BITS, true },
    };

    static const StencilFormat esStencilFormats[] = {
        {GR_GL_STENCIL_INDEX8,     8,   false},
        {GR_GL_DEPTH24_STENCIL8,   8,   true },
        {GR_GL_STENCIL_INDEX4,     4,   false},
    };

    if (GR_GL_SUPPORT_DESKTOP) {
        *count = GR_ARRAY_COUNT(desktopStencilFormats);
        return desktopStencilFormats;
    } else {
        *count = GR_ARRAY_COUNT(esStencilFormats);
        return esStencilFormats;
    }
}

// good to set a break-point here to know when createTexture fails
static GrTexture* return_null_texture() {
//    GrAssert(!"null texture");
    return NULL;
}

#if GR_DEBUG
static size_t as_size_t(int x) {
    return x;
}
#endif

GrTexture* GrGpuGL::onCreateTexture(const GrTextureDesc& desc,
                                    const void* srcData,
                                    size_t rowBytes) {

#if GR_COLLECT_STATS
    ++fStats.fTextureCreateCnt;
#endif

    this->setSpareTextureUnit();

    static const GrGLTexture::TexParams DEFAULT_PARAMS = {
        GR_GL_NEAREST,
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_CLAMP_TO_EDGE
    };

    GrGLTexture::GLTextureDesc glDesc;
    GrGLenum internalFormat;

    glDesc.fContentWidth  = desc.fWidth;
    glDesc.fContentHeight = desc.fHeight;
    glDesc.fAllocWidth    = desc.fWidth;
    glDesc.fAllocHeight   = desc.fHeight;
    glDesc.fStencilBits   = 0;
    glDesc.fFormat        = desc.fFormat;
    glDesc.fOwnsID        = true;

    bool renderTarget = 0 != (desc.fFlags & kRenderTarget_GrTextureFlagBit);
    if (!canBeTexture(desc.fFormat,
                      &internalFormat,
                      &glDesc.fUploadFormat,
                      &glDesc.fUploadType)) {
        return return_null_texture();
    }

    GrAssert(as_size_t(desc.fAALevel) < GR_ARRAY_COUNT(fAASamples));
    GrGLint samples = fAASamples[desc.fAALevel];
    if (kNone_MSFBO == fMSFBOType && desc.fAALevel != kNone_GrAALevel) {
        GrPrintf("AA RT requested but not supported on this platform.");
    }

    GR_GL(GenTextures(1, &glDesc.fTextureID));
    if (!glDesc.fTextureID) {
        return return_null_texture();
    }

    glDesc.fUploadByteCount = GrBytesPerPixel(desc.fFormat);

    // in case we need a temporary, trimmed copy of the src pixels
    GrAutoSMalloc<128 * 128> trimStorage;

    /*
     *  check if our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL doesn't let us pass the rowBytes as
     *  a parameter to glTexImage2D
     */
    if (GR_GL_SUPPORT_DESKTOP) {
        if (srcData) {
            GR_GL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH,
                              rowBytes / glDesc.fUploadByteCount));
        }
    } else {
        size_t trimRowBytes = desc.fWidth * glDesc.fUploadByteCount;
        if (srcData && (trimRowBytes < rowBytes)) {
            // copy the data into our new storage, skipping the trailing bytes
            size_t trimSize = desc.fHeight * trimRowBytes;
            const char* src = (const char*)srcData;
            char* dst = (char*)trimStorage.realloc(trimSize);
            for (int y = 0; y < desc.fHeight; y++) {
                memcpy(dst, src, trimRowBytes);
                src += rowBytes;
                dst += trimRowBytes;
            }
            // now point srcData to our trimmed version
            srcData = trimStorage.get();
        }
    }

    if (renderTarget) {
        if (!this->npotRenderTargetSupport()) {
            glDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
            glDesc.fAllocHeight = GrNextPow2(desc.fHeight);
        }

        glDesc.fAllocWidth = GrMax(fMinRenderTargetWidth,
                                   glDesc.fAllocWidth);
        glDesc.fAllocHeight = GrMax(fMinRenderTargetHeight,
                                    glDesc.fAllocHeight);
        if (glDesc.fAllocWidth > fMaxRenderTargetSize ||
            glDesc.fAllocHeight > fMaxRenderTargetSize) {
            return return_null_texture();
        }
    } else if (!this->npotTextureSupport()) {
        glDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
        glDesc.fAllocHeight = GrNextPow2(desc.fHeight);
        if (glDesc.fAllocWidth > fMaxTextureSize ||
            glDesc.fAllocHeight > fMaxTextureSize) {
            return return_null_texture();
        }
    }

    GR_GL(BindTexture(GR_GL_TEXTURE_2D, glDesc.fTextureID));
    GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                        GR_GL_TEXTURE_MAG_FILTER,
                        DEFAULT_PARAMS.fFilter));
    GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                        GR_GL_TEXTURE_MIN_FILTER,
                        DEFAULT_PARAMS.fFilter));
    GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                        GR_GL_TEXTURE_WRAP_S,
                        DEFAULT_PARAMS.fWrapS));
    GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                        GR_GL_TEXTURE_WRAP_T,
                        DEFAULT_PARAMS.fWrapT));

    GR_GL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, glDesc.fUploadByteCount));
    if (kIndex_8_GrPixelConfig == desc.fFormat &&
        supports8BitPalette()) {
        // ES only supports CompressedTexImage2D, not CompressedTexSubimage2D
        GrAssert(desc.fWidth == glDesc.fAllocWidth);
        GrAssert(desc.fHeight == glDesc.fAllocHeight);
        GrGLsizei imageSize = glDesc.fAllocWidth * glDesc.fAllocHeight +
                              kGrColorTableSize;
        GR_GL(CompressedTexImage2D(GR_GL_TEXTURE_2D, 0, glDesc.fUploadFormat,
                                   glDesc.fAllocWidth, glDesc.fAllocHeight,
                                   0, imageSize, srcData));
        GrGLRestoreResetRowLength();
    } else {
        if (NULL != srcData && (glDesc.fAllocWidth != desc.fWidth ||
                                glDesc.fAllocHeight != desc.fHeight)) {
            GR_GL(TexImage2D(GR_GL_TEXTURE_2D, 0, internalFormat,
                             glDesc.fAllocWidth, glDesc.fAllocHeight,
                             0, glDesc.fUploadFormat, glDesc.fUploadType, NULL));
            GR_GL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, 0, 0, desc.fWidth,
                                desc.fHeight, glDesc.fUploadFormat,
                                glDesc.fUploadType, srcData));
            GrGLRestoreResetRowLength();

            int extraW = glDesc.fAllocWidth  - desc.fWidth;
            int extraH = glDesc.fAllocHeight - desc.fHeight;
            int maxTexels = extraW * extraH;
            maxTexels = GrMax(extraW * desc.fHeight, maxTexels);
            maxTexels = GrMax(desc.fWidth * extraH, maxTexels);

            GrAutoSMalloc<128*128> texels(glDesc.fUploadByteCount * maxTexels);

            uint32_t rowSize = desc.fWidth * glDesc.fUploadByteCount;
            if (extraH) {
                uint8_t* lastRowStart = (uint8_t*) srcData +
                                        (desc.fHeight - 1) * rowSize;
                uint8_t* extraRowStart = (uint8_t*)texels.get();

                for (int i = 0; i < extraH; ++i) {
                    memcpy(extraRowStart, lastRowStart, rowSize);
                    extraRowStart += rowSize;
                }
                GR_GL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, 0, desc.fHeight, desc.fWidth,
                                    extraH, glDesc.fUploadFormat, glDesc.fUploadType,
                                    texels.get()));
            }
            if (extraW) {
                uint8_t* edgeTexel = (uint8_t*)srcData + rowSize - glDesc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (int j = 0; j < desc.fHeight; ++j) {
                    for (int i = 0; i < extraW; ++i) {
                        memcpy(extraTexel, edgeTexel, glDesc.fUploadByteCount);
                        extraTexel += glDesc.fUploadByteCount;
                    }
                    edgeTexel += rowSize;
                }
                GR_GL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, desc.fWidth, 0, extraW,
                                    desc.fHeight, glDesc.fUploadFormat,
                                    glDesc.fUploadType, texels.get()));
            }
            if (extraW && extraH) {
                uint8_t* cornerTexel = (uint8_t*)srcData + desc.fHeight * rowSize
                                       - glDesc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (int i = 0; i < extraW*extraH; ++i) {
                    memcpy(extraTexel, cornerTexel, glDesc.fUploadByteCount);
                    extraTexel += glDesc.fUploadByteCount;
                }
                GR_GL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, desc.fWidth, desc.fHeight,
                                    extraW, extraH, glDesc.fUploadFormat,
                                    glDesc.fUploadType, texels.get()));
            }

        } else {
            GR_GL(TexImage2D(GR_GL_TEXTURE_2D, 0, internalFormat, glDesc.fAllocWidth,
                             glDesc.fAllocHeight, 0, glDesc.fUploadFormat,
                             glDesc.fUploadType, srcData));
            GrGLRestoreResetRowLength();
        }
    }

    glDesc.fOrientation = GrGLTexture::kTopDown_Orientation;

    GrGLRenderTarget::GLRenderTargetIDs rtIDs;
    rtIDs.fStencilRenderbufferID = 0;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fRTFBOID = 0;
    rtIDs.fTexFBOID = 0;
    rtIDs.fOwnIDs = true;
    GrGLenum msColorRenderbufferFormat = -1;

    if (renderTarget) {
#if GR_COLLECT_STATS
        ++fStats.fRenderTargetCreateCnt;
#endif
        bool failed = true;
        GrGLenum status;
        GrGLint err;

        // If need have both RT flag and srcData we have
        // to invert the data before uploading because FBO
        // will be rendered bottom up
        GrAssert(NULL == srcData);
        glDesc.fOrientation =  GrGLTexture::kBottomUp_Orientation;

        GR_GL(GenFramebuffers(1, &rtIDs.fTexFBOID));
        GrAssert(rtIDs.fTexFBOID);

        // If we are using multisampling and we will create two FBOS We render
        // to one and then resolve to the texture bound to the other.
        if (samples > 1 && kNone_MSFBO != fMSFBOType) {
            GR_GL(GenFramebuffers(1, &rtIDs.fRTFBOID));
            GrAssert(0 != rtIDs.fRTFBOID);
            GR_GL(GenRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
            GrAssert(0 != rtIDs.fMSColorRenderbufferID);
            if (!fboInternalFormat(desc.fFormat, &msColorRenderbufferFormat)) {
                GR_GL(DeleteRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
                GR_GL(DeleteTextures(1, &glDesc.fTextureID));
                GR_GL(DeleteFramebuffers(1, &rtIDs.fTexFBOID));
                GR_GL(DeleteFramebuffers(1, &rtIDs.fRTFBOID));
                return return_null_texture();
            }
        } else {
            rtIDs.fRTFBOID = rtIDs.fTexFBOID;
        }
        if (!(kNoStencil_GrTextureFlagBit & desc.fFlags)) {
            GR_GL(GenRenderbuffers(1, &rtIDs.fStencilRenderbufferID));
            GrAssert(0 != rtIDs.fStencilRenderbufferID);
        }

        // someone suggested that some systems might require
        // unbinding the texture before we call FramebufferTexture2D
        // (seems unlikely)
        GR_GL(BindTexture(GR_GL_TEXTURE_2D, 0));

        err = ~GR_GL_NO_ERROR;

        int stencilFmtCnt;
        const StencilFormat* stencilFormats = NULL;
        if (rtIDs.fStencilRenderbufferID) {
            stencilFormats = GrGLStencilFormats(&stencilFmtCnt);
        } else {
            stencilFmtCnt = 1; // only 1 attempt when we don't need a stencil
        }

        for (int i = 0; i < stencilFmtCnt; ++i) {
            // we start with the last stencil format that succeeded in hopes
            // that we won't go through this loop more than once after the
            // first (painful) stencil creation.
            int sIdx = (i + fLastSuccessfulStencilFmtIdx) % stencilFmtCnt;
            
            if (rtIDs.fStencilRenderbufferID) {
                GR_GL(BindRenderbuffer(GR_GL_RENDERBUFFER,
                                       rtIDs.fStencilRenderbufferID));
                if (samples > 1) {
                    GR_GL_NO_ERR(RenderbufferStorageMultisample(
                                                GR_GL_RENDERBUFFER,
                                                samples,
                                                stencilFormats[sIdx].fEnum,
                                                glDesc.fAllocWidth,
                                                glDesc.fAllocHeight));
                } else {
                    GR_GL_NO_ERR(RenderbufferStorage(GR_GL_RENDERBUFFER,
                                                     stencilFormats[sIdx].fEnum,
                                                     glDesc.fAllocWidth,
                                                     glDesc.fAllocHeight));
                }
                err = GrGLGetGLInterface()->fGetError();
                if (err != GR_GL_NO_ERROR) {
                    continue;
                }
            }
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GrAssert(samples > 1);
                GR_GL(BindRenderbuffer(GR_GL_RENDERBUFFER,
                                       rtIDs.fMSColorRenderbufferID));
                GR_GL_NO_ERR(RenderbufferStorageMultisample(
                                                   GR_GL_RENDERBUFFER,
                                                   samples,
                                                   msColorRenderbufferFormat,
                                                   glDesc.fAllocWidth,
                                                   glDesc.fAllocHeight));
                err = GrGLGetGLInterface()->fGetError();
                if (err != GR_GL_NO_ERROR) {
                    continue;
                }
            }
            GR_GL(BindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs.fTexFBOID));

#if GR_COLLECT_STATS
            ++fStats.fRenderTargetChngCnt;
#endif
            GR_GL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_TEXTURE_2D,
                                        glDesc.fTextureID, 0));
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GrGLenum status = GR_GL(CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
                if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                    continue;
                }
                GR_GL(BindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs.fRTFBOID));
            #if GR_COLLECT_STATS
                ++fStats.fRenderTargetChngCnt;
            #endif
                GR_GL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                              GR_GL_COLOR_ATTACHMENT0,
                                              GR_GL_RENDERBUFFER,
                                              rtIDs.fMSColorRenderbufferID));

            }
            if (rtIDs.fStencilRenderbufferID) {
                // bind the stencil to rt fbo if present, othewise the tex fbo
                GR_GL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                              GR_GL_STENCIL_ATTACHMENT,
                                              GR_GL_RENDERBUFFER,
                                              rtIDs.fStencilRenderbufferID));
                // if it is a packed format bind to depth also, otherwise 
                // we may get an unsupported fbo completeness result
                if (stencilFormats[sIdx].fPacked) {
                    GR_GL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                  GR_GL_DEPTH_ATTACHMENT,
                                                  GR_GL_RENDERBUFFER,
                                                  rtIDs.fStencilRenderbufferID));
                }
            }
            status = GR_GL(CheckFramebufferStatus(GR_GL_FRAMEBUFFER));

            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                // undo the depth bind
                if (rtIDs.fStencilRenderbufferID && 
                    stencilFormats[sIdx].fPacked) {
                    GR_GL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                    GR_GL_DEPTH_ATTACHMENT,
                                                    GR_GL_RENDERBUFFER,
                                                    0));
                }
                continue;
            }
            // we're successful!
            failed = false;
            if (rtIDs.fStencilRenderbufferID) {
                fLastSuccessfulStencilFmtIdx = sIdx;
                if (UNKNOWN_BITS == stencilFormats[sIdx].fBits) {
                    GR_GL_GetIntegerv(GR_GL_STENCIL_BITS, (GrGLint*)&glDesc.fStencilBits);
                } else {
                    glDesc.fStencilBits = stencilFormats[sIdx].fBits;
                }
            }
            break;
        }
        if (failed) {
            if (rtIDs.fStencilRenderbufferID) {
                GR_GL(DeleteRenderbuffers(1, &rtIDs.fStencilRenderbufferID));
            }
            if (rtIDs.fMSColorRenderbufferID) {
                GR_GL(DeleteRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
            }
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GR_GL(DeleteFramebuffers(1, &rtIDs.fRTFBOID));
            }
            if (rtIDs.fTexFBOID) {
                GR_GL(DeleteFramebuffers(1, &rtIDs.fTexFBOID));
            }
            GR_GL(DeleteTextures(1, &glDesc.fTextureID));
            return return_null_texture();
        }
    }
#ifdef TRACE_TEXTURE_CREATION
    GrPrintf("--- new texture [%d] size=(%d %d) bpp=%d\n",
             tex->fTextureID, width, height, tex->fUploadByteCount);
#endif
    GrGLTexture* tex = new GrGLTexture(this, glDesc, rtIDs, DEFAULT_PARAMS);

    if (0 != rtIDs.fTexFBOID) {
        GrRenderTarget* rt = tex->asRenderTarget();
        // We've messed with FBO state but may not have set the correct viewport
        // so just dirty the rendertarget state to force a resend.
        fHWDrawState.fRenderTarget = NULL;

        // clear the new stencil buffer if we have one
        if (!(desc.fFlags & kNoStencil_GrTextureFlagBit)) {
            GrRenderTarget* rtSave = fCurrDrawState.fRenderTarget;
            fCurrDrawState.fRenderTarget = rt;
            this->clearStencil(0, ~0);
            fCurrDrawState.fRenderTarget = rtSave;
        }
    }
    return tex;
}

GrVertexBuffer* GrGpuGL::onCreateVertexBuffer(uint32_t size, bool dynamic) {
    GrGLuint id;
    GR_GL(GenBuffers(1, &id));
    if (id) {
        GR_GL(BindBuffer(GR_GL_ARRAY_BUFFER, id));
        fHWGeometryState.fArrayPtrsDirty = true;
        GrGLClearErr();
        // make sure driver can allocate memory for this buffer
        GR_GL_NO_ERR(BufferData(GR_GL_ARRAY_BUFFER, size, NULL,
                                dynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
        if (GrGLGetGLInterface()->fGetError() != GR_GL_NO_ERROR) {
            GR_GL(DeleteBuffers(1, &id));
            // deleting bound buffer does implicit bind to 0
            fHWGeometryState.fVertexBuffer = NULL;
            return NULL;
        }
        GrGLVertexBuffer* vertexBuffer = new GrGLVertexBuffer(this, id,
                                                              size, dynamic);
        fHWGeometryState.fVertexBuffer = vertexBuffer;
        return vertexBuffer;
    }
    return NULL;
}

GrIndexBuffer* GrGpuGL::onCreateIndexBuffer(uint32_t size, bool dynamic) {
    GrGLuint id;
    GR_GL(GenBuffers(1, &id));
    if (id) {
        GR_GL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, id));
        GrGLClearErr();
        // make sure driver can allocate memory for this buffer
        GR_GL_NO_ERR(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER, size, NULL,
                                dynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
        if (GrGLGetGLInterface()->fGetError() != GR_GL_NO_ERROR) {
            GR_GL(DeleteBuffers(1, &id));
            // deleting bound buffer does implicit bind to 0
            fHWGeometryState.fIndexBuffer = NULL;
            return NULL;
        }
        GrIndexBuffer* indexBuffer = new GrGLIndexBuffer(this, id,
                                                         size, dynamic);
        fHWGeometryState.fIndexBuffer = indexBuffer;
        return indexBuffer;
    }
    return NULL;
}

void GrGpuGL::flushScissor(const GrIRect* rect) {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
    const GrGLIRect& vp =
            ((GrGLRenderTarget*)fCurrDrawState.fRenderTarget)->getViewport();

    GrGLIRect scissor;
    if (NULL != rect) {
        scissor.setRelativeTo(vp, rect->fLeft, rect->fTop,
                              rect->width(), rect->height());
        if (scissor.contains(vp)) {
            rect = NULL;
        }
    }

    if (NULL != rect) {
        if (fHWBounds.fScissorRect != scissor) {
            scissor.pushToGLScissor();
            fHWBounds.fScissorRect = scissor;
        }
        if (!fHWBounds.fScissorEnabled) {
            GR_GL(Enable(GR_GL_SCISSOR_TEST));
            fHWBounds.fScissorEnabled = true;
        }
    } else {
        if (fHWBounds.fScissorEnabled) {
            GR_GL(Disable(GR_GL_SCISSOR_TEST));
            fHWBounds.fScissorEnabled = false;
        }
    }
}

void GrGpuGL::onClear(const GrIRect* rect, GrColor color) {
    if (NULL == fCurrDrawState.fRenderTarget) {
        return;
    }
    GrIRect r;
    if (NULL != rect) {
        // flushScissor expects rect to be clipped to the target.
        r = *rect;
        GrIRect rtRect = SkIRect::MakeWH(fCurrDrawState.fRenderTarget->width(),
                                         fCurrDrawState.fRenderTarget->height());
        if (r.intersect(rtRect)) {
            rect = &r;
        } else {
            return;
        }
    }
    this->flushRenderTarget(rect);
    this->flushScissor(rect);
    GR_GL(ColorMask(GR_GL_TRUE,GR_GL_TRUE,GR_GL_TRUE,GR_GL_TRUE));
    fHWDrawState.fFlagBits &= ~kNoColorWrites_StateBit;
    GR_GL(ClearColor(GrColorUnpackR(color)/255.f,
                     GrColorUnpackG(color)/255.f,
                     GrColorUnpackB(color)/255.f,
                     GrColorUnpackA(color)/255.f));
    GR_GL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGpuGL::clearStencil(uint32_t value, uint32_t mask) {
    if (NULL == fCurrDrawState.fRenderTarget) {
        return;
    }
    
    this->flushRenderTarget(&GrIRect::EmptyIRect());

    if (fHWBounds.fScissorEnabled) {
        GR_GL(Disable(GR_GL_SCISSOR_TEST));
        fHWBounds.fScissorEnabled = false;
    }
    GR_GL(StencilMask(mask));
    GR_GL(ClearStencil(value));
    GR_GL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWDrawState.fStencilSettings.invalidate();
}

void GrGpuGL::clearStencilClip(const GrIRect& rect) {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
#if 0
    GrGLint stencilBitCount = fCurrDrawState.fRenderTarget->stencilBits();
    GrAssert(stencilBitCount > 0);
    GrGLint clipStencilMask  = (1 << (stencilBitCount - 1));
#else
    // we could just clear the clip bit but when we go through
    // angle a partial stencil mask will cause clears to be
    // turned into draws. Our contract on GrDrawTarget says that
    // changing the clip between stencil passes may or may not
    // zero the client's clip bits. So we just clear the whole thing.
    static const GrGLint clipStencilMask  = ~0;
#endif
    this->flushRenderTarget(&GrIRect::EmptyIRect());
    flushScissor(&rect);
    GR_GL(StencilMask(clipStencilMask));
    GR_GL(ClearStencil(0));
    GR_GL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWDrawState.fStencilSettings.invalidate();
}

void GrGpuGL::onForceRenderTargetFlush() {
    this->flushRenderTarget(&GrIRect::EmptyIRect());
}

bool GrGpuGL::onReadPixels(GrRenderTarget* target,
                           int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer) {
    GrGLenum internalFormat;  // we don't use this for glReadPixels
    GrGLenum format;
    GrGLenum type;
    if (!this->canBeTexture(config, &internalFormat, &format, &type)) {
        return false;
    }    
    GrGLRenderTarget* tgt = static_cast<GrGLRenderTarget*>(target);
    GrAutoTPtrValueRestore<GrRenderTarget*> autoTargetRestore;
    switch (tgt->getResolveType()) {
        case GrGLRenderTarget::kCantResolve_ResolveType:
            return false;
        case GrGLRenderTarget::kAutoResolves_ResolveType:
            autoTargetRestore.save(&fCurrDrawState.fRenderTarget);
            fCurrDrawState.fRenderTarget = target;
            this->flushRenderTarget(&GrIRect::EmptyIRect());
            break;
        case GrGLRenderTarget::kCanResolve_ResolveType:
            this->resolveRenderTarget(tgt);
            // we don't track the state of the READ FBO ID.
            GR_GL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, tgt->textureFBOID()));
            break;
        default:
            GrCrash("Unknown resolve type");
    }

    const GrGLIRect& glvp = tgt->getViewport();

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height);
    GR_GL(ReadPixels(readRect.fLeft, readRect.fBottom,
                     readRect.fWidth, readRect.fHeight,
                     format, type, buffer));

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom
    {
        size_t stride = width * GrBytesPerPixel(config);
        GrAutoMalloc rowStorage(stride);
        void* tmp = rowStorage.get();

        const int halfY = height >> 1;
        char* top = reinterpret_cast<char*>(buffer);
        char* bottom = top + (height - 1) * stride;
        for (int y = 0; y < halfY; y++) {
            memcpy(tmp, top, stride);
            memcpy(top, bottom, stride);
            memcpy(bottom, tmp, stride);
            top += stride;
            bottom -= stride;
        }
    }
    return true;
}

void GrGpuGL::flushRenderTarget(const GrIRect* bound) {

    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    GrGLRenderTarget* rt = (GrGLRenderTarget*)fCurrDrawState.fRenderTarget;
    if (fHWDrawState.fRenderTarget != fCurrDrawState.fRenderTarget) {
        GR_GL(BindFramebuffer(GR_GL_FRAMEBUFFER, rt->renderFBOID()));
    #if GR_COLLECT_STATS
        ++fStats.fRenderTargetChngCnt;
    #endif
    #if GR_DEBUG
        GrGLenum status = GR_GL(CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            GrPrintf("GrGpuGL::flushRenderTarget glCheckFramebufferStatus %x\n", status);
        }
    #endif
        fDirtyFlags.fRenderTargetChanged = true;
        fHWDrawState.fRenderTarget = fCurrDrawState.fRenderTarget;
        const GrGLIRect& vp = rt->getViewport();
        if (fHWBounds.fViewportRect != vp) {
            vp.pushToGLViewport();
            fHWBounds.fViewportRect = vp;
        }
    }
    if (NULL == bound || !bound->isEmpty()) {
        rt->flagAsNeedingResolve(bound);
    }
}

GrGLenum gPrimitiveType2GLMode[] = {
    GR_GL_TRIANGLES,
    GR_GL_TRIANGLE_STRIP,
    GR_GL_TRIANGLE_FAN,
    GR_GL_POINTS,
    GR_GL_LINES,
    GR_GL_LINE_STRIP
};

#define SWAP_PER_DRAW 0

#if SWAP_PER_DRAW
    #if GR_MAC_BUILD
        #include <AGL/agl.h>
    #elif GR_WIN32_BUILD
        void SwapBuf() {
            DWORD procID = GetCurrentProcessId();
            HWND hwnd = GetTopWindow(GetDesktopWindow());
            while(hwnd) {
                DWORD wndProcID = 0;
                GetWindowThreadProcessId(hwnd, &wndProcID);
                if(wndProcID == procID) {
                    SwapBuffers(GetDC(hwnd));
                }
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
            }
         }
    #endif
#endif

void GrGpuGL::onGpuDrawIndexed(GrPrimitiveType type,
                               uint32_t startVertex,
                               uint32_t startIndex,
                               uint32_t vertexCount,
                               uint32_t indexCount) {
    GrAssert((size_t)type < GR_ARRAY_COUNT(gPrimitiveType2GLMode));

    GrGLvoid* indices = (GrGLvoid*)(sizeof(uint16_t) * startIndex);

    GrAssert(NULL != fHWGeometryState.fIndexBuffer);
    GrAssert(NULL != fHWGeometryState.fVertexBuffer);

    // our setupGeometry better have adjusted this to zero since
    // DrawElements always draws from the begining of the arrays for idx 0.
    GrAssert(0 == startVertex);

    GR_GL(DrawElements(gPrimitiveType2GLMode[type], indexCount,
                       GR_GL_UNSIGNED_SHORT, indices));
#if SWAP_PER_DRAW
    glFlush();
    #if GR_MAC_BUILD
        aglSwapBuffers(aglGetCurrentContext());
        int set_a_break_pt_here = 9;
        aglSwapBuffers(aglGetCurrentContext());
    #elif GR_WIN32_BUILD
        SwapBuf();
        int set_a_break_pt_here = 9;
        SwapBuf();
    #endif
#endif
}

void GrGpuGL::onGpuDrawNonIndexed(GrPrimitiveType type,
                                  uint32_t startVertex,
                                  uint32_t vertexCount) {
    GrAssert((size_t)type < GR_ARRAY_COUNT(gPrimitiveType2GLMode));

    GrAssert(NULL != fHWGeometryState.fVertexBuffer);

    // our setupGeometry better have adjusted this to zero.
    // DrawElements doesn't take an offset so we always adjus the startVertex.
    GrAssert(0 == startVertex);

    // pass 0 for parameter first. We have to adjust gl*Pointer() to
    // account for startVertex in the DrawElements case. So we always
    // rely on setupGeometry to have accounted for startVertex.
    GR_GL(DrawArrays(gPrimitiveType2GLMode[type], 0, vertexCount));
#if SWAP_PER_DRAW
    glFlush();
    #if GR_MAC_BUILD
        aglSwapBuffers(aglGetCurrentContext());
        int set_a_break_pt_here = 9;
        aglSwapBuffers(aglGetCurrentContext());
    #elif GR_WIN32_BUILD
        SwapBuf();
        int set_a_break_pt_here = 9;
        SwapBuf();
    #endif
#endif
}

void GrGpuGL::resolveRenderTarget(GrGLRenderTarget* rt) {

    if (rt->needsResolve()) {
        GrAssert(kNone_MSFBO != fMSFBOType);
        GrAssert(rt->textureFBOID() != rt->renderFBOID());
        GR_GL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER,
                                        rt->renderFBOID()));
        GR_GL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER,
                                        rt->textureFBOID()));
    #if GR_COLLECT_STATS
        ++fStats.fRenderTargetChngCnt;
    #endif
        // make sure we go through flushRenderTarget() since we've modified
        // the bound DRAW FBO ID.
        fHWDrawState.fRenderTarget = NULL;
        const GrGLIRect& vp = rt->getViewport();
        const GrIRect dirtyRect = rt->getResolveRect();
        GrGLIRect r;
        r.setRelativeTo(vp, dirtyRect.fLeft, dirtyRect.fTop, 
                        dirtyRect.width(), dirtyRect.height());

        if (kAppleES_MSFBO == fMSFBOType) {
            // Apple's extension uses the scissor as the blit bounds.
            GR_GL(Enable(GR_GL_SCISSOR_TEST));
            GR_GL(Scissor(r.fLeft, r.fBottom,
                          r.fWidth, r.fHeight));
            GR_GL(ResolveMultisampleFramebuffer());
            fHWBounds.fScissorRect.invalidate();
            fHWBounds.fScissorEnabled = true;
        } else {
            if (kDesktopARB_MSFBO != fMSFBOType) {
                // this respects the scissor during the blit, so disable it.
                GrAssert(kDesktopEXT_MSFBO == fMSFBOType);
                flushScissor(NULL);
            }
            int right = r.fLeft + r.fWidth;
            int top = r.fBottom + r.fHeight;
            GR_GL(BlitFramebuffer(r.fLeft, r.fBottom, right, top,
                                  r.fLeft, r.fBottom, right, top,
                                  GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
        }
        rt->flagAsResolved();
    }
}

static const GrGLenum grToGLStencilFunc[] = {
    GR_GL_ALWAYS,           // kAlways_StencilFunc
    GR_GL_NEVER,            // kNever_StencilFunc
    GR_GL_GREATER,          // kGreater_StencilFunc
    GR_GL_GEQUAL,           // kGEqual_StencilFunc
    GR_GL_LESS,             // kLess_StencilFunc
    GR_GL_LEQUAL,           // kLEqual_StencilFunc,
    GR_GL_EQUAL,            // kEqual_StencilFunc,
    GR_GL_NOTEQUAL,         // kNotEqual_StencilFunc,
};
GR_STATIC_ASSERT(GR_ARRAY_COUNT(grToGLStencilFunc) == kBasicStencilFuncCount);
GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
GR_STATIC_ASSERT(1 == kNever_StencilFunc);
GR_STATIC_ASSERT(2 == kGreater_StencilFunc);
GR_STATIC_ASSERT(3 == kGEqual_StencilFunc);
GR_STATIC_ASSERT(4 == kLess_StencilFunc);
GR_STATIC_ASSERT(5 == kLEqual_StencilFunc);
GR_STATIC_ASSERT(6 == kEqual_StencilFunc);
GR_STATIC_ASSERT(7 == kNotEqual_StencilFunc);

static const GrGLenum grToGLStencilOp[] = {
    GR_GL_KEEP,        // kKeep_StencilOp
    GR_GL_REPLACE,     // kReplace_StencilOp
    GR_GL_INCR_WRAP,   // kIncWrap_StencilOp
    GR_GL_INCR,        // kIncClamp_StencilOp
    GR_GL_DECR_WRAP,   // kDecWrap_StencilOp
    GR_GL_DECR,        // kDecClamp_StencilOp
    GR_GL_ZERO,        // kZero_StencilOp
    GR_GL_INVERT,      // kInvert_StencilOp
};
GR_STATIC_ASSERT(GR_ARRAY_COUNT(grToGLStencilOp) == kStencilOpCount);
GR_STATIC_ASSERT(0 == kKeep_StencilOp);
GR_STATIC_ASSERT(1 == kReplace_StencilOp);
GR_STATIC_ASSERT(2 == kIncWrap_StencilOp);
GR_STATIC_ASSERT(3 == kIncClamp_StencilOp);
GR_STATIC_ASSERT(4 == kDecWrap_StencilOp);
GR_STATIC_ASSERT(5 == kDecClamp_StencilOp);
GR_STATIC_ASSERT(6 == kZero_StencilOp);
GR_STATIC_ASSERT(7 == kInvert_StencilOp);

void GrGpuGL::flushStencil() {
    const GrStencilSettings* settings = &fCurrDrawState.fStencilSettings;

    // use stencil for clipping if clipping is enabled and the clip
    // has been written into the stencil.
    bool stencilClip = fClipState.fClipInStencil &&
                       (kClip_StateBit & fCurrDrawState.fFlagBits);
    bool stencilChange = fHWStencilClip != stencilClip  ||
                         fHWDrawState.fStencilSettings != *settings ||
                         ((fHWDrawState.fFlagBits & kModifyStencilClip_StateBit) !=
                          (fCurrDrawState.fFlagBits & kModifyStencilClip_StateBit));

    if (stencilChange) {

        // we can't simultaneously perform stencil-clipping and modify the stencil clip
        GrAssert(!stencilClip || !(fCurrDrawState.fFlagBits & kModifyStencilClip_StateBit));

        if (settings->isDisabled()) {
            if (stencilClip) {
                settings = &gClipStencilSettings;
            }
        }

        if (settings->isDisabled()) {
            GR_GL(Disable(GR_GL_STENCIL_TEST));
        } else {
            GR_GL(Enable(GR_GL_STENCIL_TEST));
    #if GR_DEBUG
            if (!fStencilWrapOpsSupport) {
                GrAssert(settings->fFrontPassOp != kIncWrap_StencilOp);
                GrAssert(settings->fFrontPassOp != kDecWrap_StencilOp);
                GrAssert(settings->fFrontFailOp != kIncWrap_StencilOp);
                GrAssert(settings->fBackFailOp != kDecWrap_StencilOp);
                GrAssert(settings->fBackPassOp != kIncWrap_StencilOp);
                GrAssert(settings->fBackPassOp != kDecWrap_StencilOp);
                GrAssert(settings->fBackFailOp != kIncWrap_StencilOp);
                GrAssert(settings->fFrontFailOp != kDecWrap_StencilOp);
            }
    #endif
            int stencilBits = fCurrDrawState.fRenderTarget->stencilBits();
            GrAssert(stencilBits ||
                     (GrStencilSettings::gDisabled ==
                      fCurrDrawState.fStencilSettings));
            GrGLuint clipStencilMask = 1 << (stencilBits - 1);
            GrGLuint userStencilMask = clipStencilMask - 1;

            unsigned int frontRef  = settings->fFrontFuncRef;
            unsigned int frontMask = settings->fFrontFuncMask;
            unsigned int frontWriteMask = settings->fFrontWriteMask;
            GrGLenum frontFunc;

            if (fCurrDrawState.fFlagBits & kModifyStencilClip_StateBit) {

                GrAssert(settings->fFrontFunc < kBasicStencilFuncCount);
                frontFunc = grToGLStencilFunc[settings->fFrontFunc];
            } else {
                frontFunc = grToGLStencilFunc[ConvertStencilFunc(stencilClip, settings->fFrontFunc)];

                ConvertStencilFuncAndMask(settings->fFrontFunc,
                                          stencilClip,
                                          clipStencilMask,
                                          userStencilMask,
                                          &frontRef,
                                          &frontMask);
                frontWriteMask &= userStencilMask;
            }
            GrAssert(settings->fFrontFailOp >= 0 &&
                     (unsigned) settings->fFrontFailOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fFrontPassOp >= 0 &&
                     (unsigned) settings->fFrontPassOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fBackFailOp >= 0 &&
                     (unsigned) settings->fBackFailOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fBackPassOp >= 0 &&
                     (unsigned) settings->fBackPassOp < GR_ARRAY_COUNT(grToGLStencilOp));
            if (fTwoSidedStencilSupport) {
                GrGLenum backFunc;

                unsigned int backRef  = settings->fBackFuncRef;
                unsigned int backMask = settings->fBackFuncMask;
                unsigned int backWriteMask = settings->fBackWriteMask;


                if (fCurrDrawState.fFlagBits & kModifyStencilClip_StateBit) {
                    GrAssert(settings->fBackFunc < kBasicStencilFuncCount);
                    backFunc = grToGLStencilFunc[settings->fBackFunc];
                } else {
                    backFunc = grToGLStencilFunc[ConvertStencilFunc(stencilClip, settings->fBackFunc)];
                    ConvertStencilFuncAndMask(settings->fBackFunc,
                                              stencilClip,
                                              clipStencilMask,
                                              userStencilMask,
                                              &backRef,
                                              &backMask);
                    backWriteMask &= userStencilMask;
                }

                GR_GL(StencilFuncSeparate(GR_GL_FRONT, frontFunc, frontRef, frontMask));
                GR_GL(StencilMaskSeparate(GR_GL_FRONT, frontWriteMask));
                GR_GL(StencilFuncSeparate(GR_GL_BACK, backFunc, backRef, backMask));
                GR_GL(StencilMaskSeparate(GR_GL_BACK, backWriteMask));
                GR_GL(StencilOpSeparate(GR_GL_FRONT, grToGLStencilOp[settings->fFrontFailOp],
                                                  grToGLStencilOp[settings->fFrontPassOp],
                                                  grToGLStencilOp[settings->fFrontPassOp]));

                GR_GL(StencilOpSeparate(GR_GL_BACK,  grToGLStencilOp[settings->fBackFailOp],
                                                  grToGLStencilOp[settings->fBackPassOp],
                                                  grToGLStencilOp[settings->fBackPassOp]));
            } else {
                GR_GL(StencilFunc(frontFunc, frontRef, frontMask));
                GR_GL(StencilMask(frontWriteMask));
                GR_GL(StencilOp(grToGLStencilOp[settings->fFrontFailOp],
                                grToGLStencilOp[settings->fFrontPassOp],
                                grToGLStencilOp[settings->fFrontPassOp]));
            }
        }
        fHWDrawState.fStencilSettings = fCurrDrawState.fStencilSettings;
        fHWStencilClip = stencilClip;
    }
}

bool GrGpuGL::useSmoothLines() {
    // there is a conflict between using smooth lines and our use of
    // premultiplied alpha. Smooth lines tweak the incoming alpha value
    // but not in a premul-alpha way. So we only use them when our alpha
    // is 0xff.

    // TODO: write a smarter line frag shader.

    return (kAntialias_StateBit & fCurrDrawState.fFlagBits) &&
           canDisableBlend();
}

void GrGpuGL::flushAAState(GrPrimitiveType type) {
    if (GR_GL_SUPPORT_DESKTOP) {
        // ES doesn't support toggling GL_MULTISAMPLE and doesn't have
        // smooth lines.

        // we prefer smooth lines over multisampled lines
        // msaa should be disabled if drawing smooth lines.
        if (GrIsPrimTypeLines(type)) {
            bool smooth = useSmoothLines();
            if (!fHWAAState.fSmoothLineEnabled && smooth) {
                GR_GL(Enable(GR_GL_LINE_SMOOTH));
                fHWAAState.fSmoothLineEnabled = true;
            } else if (fHWAAState.fSmoothLineEnabled && !smooth) {
                GR_GL(Disable(GR_GL_LINE_SMOOTH));
                fHWAAState.fSmoothLineEnabled = false;
            }
            if (fCurrDrawState.fRenderTarget->isMultisampled() && 
                fHWAAState.fMSAAEnabled) {
                GR_GL(Disable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = false;
            }
        } else if (fCurrDrawState.fRenderTarget->isMultisampled() &&
                   !!(kAntialias_StateBit & fCurrDrawState.fFlagBits) !=
                   fHWAAState.fMSAAEnabled) {
            if (fHWAAState.fMSAAEnabled) {
                GR_GL(Disable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = false;
            } else {
                GR_GL(Enable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = true;
            }
        }
    }
}

void GrGpuGL::flushBlend(GrPrimitiveType type, 
                         GrBlendCoeff srcCoeff, 
                         GrBlendCoeff dstCoeff) {
    if (GrIsPrimTypeLines(type) && useSmoothLines()) {
        if (fHWBlendDisabled) {
            GR_GL(Enable(GR_GL_BLEND));
            fHWBlendDisabled = false;
        }
        if (kSA_BlendCoeff != fHWDrawState.fSrcBlend ||
            kISA_BlendCoeff != fHWDrawState.fDstBlend) {
            GR_GL(BlendFunc(gXfermodeCoeff2Blend[kSA_BlendCoeff],
                            gXfermodeCoeff2Blend[kISA_BlendCoeff]));
            fHWDrawState.fSrcBlend = kSA_BlendCoeff;
            fHWDrawState.fDstBlend = kISA_BlendCoeff;
        }
    } else {
        bool blendOff = canDisableBlend();
        if (fHWBlendDisabled != blendOff) {
            if (blendOff) {
                GR_GL(Disable(GR_GL_BLEND));
            } else {
                GR_GL(Enable(GR_GL_BLEND));
            }
            fHWBlendDisabled = blendOff;
        }
        if (!blendOff) {
            if (fHWDrawState.fSrcBlend != srcCoeff ||
                fHWDrawState.fDstBlend != dstCoeff) {
                GR_GL(BlendFunc(gXfermodeCoeff2Blend[srcCoeff],
                                gXfermodeCoeff2Blend[dstCoeff]));
                fHWDrawState.fSrcBlend = srcCoeff;
                fHWDrawState.fDstBlend = dstCoeff;
            }
            if ((BlendCoeffReferencesConstant(srcCoeff) ||
                 BlendCoeffReferencesConstant(dstCoeff)) &&
                fHWDrawState.fBlendConstant != fCurrDrawState.fBlendConstant) {

                float c[] = {
                    GrColorUnpackR(fCurrDrawState.fBlendConstant) / 255.f,
                    GrColorUnpackG(fCurrDrawState.fBlendConstant) / 255.f,
                    GrColorUnpackB(fCurrDrawState.fBlendConstant) / 255.f,
                    GrColorUnpackA(fCurrDrawState.fBlendConstant) / 255.f
                };
                GR_GL(BlendColor(c[0], c[1], c[2], c[3]));
                fHWDrawState.fBlendConstant = fCurrDrawState.fBlendConstant;
            }
        }
    }
}

bool GrGpuGL::flushGLStateCommon(GrPrimitiveType type) {

    // GrGpu::setupClipAndFlushState should have already checked this
    // and bailed if not true.
    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    for (int s = 0; s < kNumStages; ++s) {
        // bind texture and set sampler state
        if (this->isStageEnabled(s)) {
            GrGLTexture* nextTexture = (GrGLTexture*)fCurrDrawState.fTextures[s];

            // true for now, but maybe not with GrEffect.
            GrAssert(NULL != nextTexture);
            // if we created a rt/tex and rendered to it without using a
            // texture and now we're texuring from the rt it will still be
            // the last bound texture, but it needs resolving. So keep this
            // out of the "last != next" check.
            GrGLRenderTarget* texRT = 
                static_cast<GrGLRenderTarget*>(nextTexture->asRenderTarget());
            if (NULL != texRT) {
                resolveRenderTarget(texRT);
            }

            if (fHWDrawState.fTextures[s] != nextTexture) {
                setTextureUnit(s);
                GR_GL(BindTexture(GR_GL_TEXTURE_2D, nextTexture->textureID()));
            #if GR_COLLECT_STATS
                ++fStats.fTextureChngCnt;
            #endif
                //GrPrintf("---- bindtexture %d\n", nextTexture->textureID());
                fHWDrawState.fTextures[s] = nextTexture;
                // The texture matrix has to compensate for texture width/height
                // and NPOT-embedded-in-POT
                fDirtyFlags.fTextureChangedMask |= (1 << s);
            }

            const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
            const GrGLTexture::TexParams& oldTexParams =
                                                nextTexture->getTexParams();
            GrGLTexture::TexParams newTexParams;

            if (GrSamplerState::kNearest_Filter == sampler.getFilter()) {
                newTexParams.fFilter = GR_GL_NEAREST;
            } else {
                newTexParams.fFilter = GR_GL_LINEAR;
            }

            newTexParams.fWrapS =
                        GrGLTexture::WrapMode2GLWrap()[sampler.getWrapX()];
            newTexParams.fWrapT =
                        GrGLTexture::WrapMode2GLWrap()[sampler.getWrapY()];

            if (newTexParams.fFilter != oldTexParams.fFilter) {
                setTextureUnit(s);
                GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                                    GR_GL_TEXTURE_MAG_FILTER,
                                    newTexParams.fFilter));
                GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                                    GR_GL_TEXTURE_MIN_FILTER,
                                    newTexParams.fFilter));
            }
            if (newTexParams.fWrapS != oldTexParams.fWrapS) {
                setTextureUnit(s);
                GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                                    GR_GL_TEXTURE_WRAP_S,
                                    newTexParams.fWrapS));
            }
            if (newTexParams.fWrapT != oldTexParams.fWrapT) {
                setTextureUnit(s);
                GR_GL(TexParameteri(GR_GL_TEXTURE_2D,
                                    GR_GL_TEXTURE_WRAP_T,
                                    newTexParams.fWrapT));
            }
            nextTexture->setTexParams(newTexParams);
        }
    }

    GrIRect* rect = NULL;
    GrIRect clipBounds;
    if ((fCurrDrawState.fFlagBits & kClip_StateBit) &&
        fClip.hasConservativeBounds()) {
        fClip.getConservativeBounds().roundOut(&clipBounds);
        rect = &clipBounds;
    }
    this->flushRenderTarget(rect);
    this->flushAAState(type);
    
    if ((fCurrDrawState.fFlagBits & kDither_StateBit) !=
        (fHWDrawState.fFlagBits & kDither_StateBit)) {
        if (fCurrDrawState.fFlagBits & kDither_StateBit) {
            GR_GL(Enable(GR_GL_DITHER));
        } else {
            GR_GL(Disable(GR_GL_DITHER));
        }
    }

    if ((fCurrDrawState.fFlagBits & kNoColorWrites_StateBit) !=
        (fHWDrawState.fFlagBits & kNoColorWrites_StateBit)) {
        GrGLenum mask;
        if (fCurrDrawState.fFlagBits & kNoColorWrites_StateBit) {
            mask = GR_GL_FALSE;
        } else {
            mask = GR_GL_TRUE;
        }
        GR_GL(ColorMask(mask, mask, mask, mask));
    }

    if (fHWDrawState.fDrawFace != fCurrDrawState.fDrawFace) {
        switch (fCurrDrawState.fDrawFace) {
            case kCCW_DrawFace:
                GR_GL(Enable(GR_GL_CULL_FACE));
                GR_GL(CullFace(GR_GL_BACK));
                break;
            case kCW_DrawFace:
                GR_GL(Enable(GR_GL_CULL_FACE));
                GR_GL(CullFace(GR_GL_FRONT));
                break;
            case kBoth_DrawFace:
                GR_GL(Disable(GR_GL_CULL_FACE));
                break;
            default:
                GrCrash("Unknown draw face.");
        }
        fHWDrawState.fDrawFace = fCurrDrawState.fDrawFace;
    }

#if GR_DEBUG
    // check for circular rendering
    for (int s = 0; s < kNumStages; ++s) {
        GrAssert(!this->isStageEnabled(s) ||
                 NULL == fCurrDrawState.fRenderTarget ||
                 NULL == fCurrDrawState.fTextures[s] ||
                 fCurrDrawState.fTextures[s]->asRenderTarget() !=
                    fCurrDrawState.fRenderTarget);
    }
#endif

    flushStencil();

    // flushStencil may look at the private state bits, so keep it before this.
    fHWDrawState.fFlagBits = fCurrDrawState.fFlagBits;
    return true;
}

void GrGpuGL::notifyVertexBufferBind(const GrGLVertexBuffer* buffer) {
    if (fHWGeometryState.fVertexBuffer != buffer) {
        fHWGeometryState.fArrayPtrsDirty = true;
        fHWGeometryState.fVertexBuffer = buffer;
    }
}

void GrGpuGL::notifyVertexBufferDelete(const GrGLVertexBuffer* buffer) {
    if (fHWGeometryState.fVertexBuffer == buffer) {
        // deleting bound buffer does implied bind to 0
        fHWGeometryState.fVertexBuffer = NULL;
        fHWGeometryState.fArrayPtrsDirty = true;
    }
}

void GrGpuGL::notifyIndexBufferBind(const GrGLIndexBuffer* buffer) {
    fHWGeometryState.fIndexBuffer = buffer;
}

void GrGpuGL::notifyIndexBufferDelete(const GrGLIndexBuffer* buffer) {
    if (fHWGeometryState.fIndexBuffer == buffer) {
        // deleting bound buffer does implied bind to 0
        fHWGeometryState.fIndexBuffer = NULL;
    }
}

void GrGpuGL::notifyRenderTargetDelete(GrRenderTarget* renderTarget) {
    GrAssert(NULL != renderTarget);
    if (fCurrDrawState.fRenderTarget == renderTarget) {
        fCurrDrawState.fRenderTarget = NULL;
    }
    if (fHWDrawState.fRenderTarget == renderTarget) {
        fHWDrawState.fRenderTarget = NULL;
    }
}

void GrGpuGL::notifyTextureDelete(GrGLTexture* texture) {
    for (int s = 0; s < kNumStages; ++s) {
        if (fCurrDrawState.fTextures[s] == texture) {
            fCurrDrawState.fTextures[s] = NULL;
        }
        if (fHWDrawState.fTextures[s] == texture) {
            // deleting bound texture does implied bind to 0
            fHWDrawState.fTextures[s] = NULL;
       }
    }
}

bool GrGpuGL::canBeTexture(GrPixelConfig config,
                           GrGLenum* internalFormat,
                           GrGLenum* format,
                           GrGLenum* type) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kRGBX_8888_GrPixelConfig: // todo: can we tell it our X?
            *format = GR_GL_32BPP_COLOR_FORMAT;
            if (GR_GL_SUPPORT_ES) {
                // according to GL_EXT_texture_format_BGRA8888 the *internal*
                // format for a BGRA is BGRA not RGBA (as on desktop)
                *internalFormat = GR_GL_32BPP_COLOR_FORMAT;
            } else {
                *internalFormat = GR_GL_RGBA;
            }
            *type = GR_GL_UNSIGNED_BYTE;
            break;
        case kRGB_565_GrPixelConfig:
            *format = GR_GL_RGB;
            *internalFormat = GR_GL_RGB;
            *type = GR_GL_UNSIGNED_SHORT_5_6_5;
            break;
        case kRGBA_4444_GrPixelConfig:
            *format = GR_GL_RGBA;
            *internalFormat = GR_GL_RGBA;
            *type = GR_GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case kIndex_8_GrPixelConfig:
            if (this->supports8BitPalette()) {
                *format = GR_GL_PALETTE8_RGBA8;
                *internalFormat = GR_GL_PALETTE8_RGBA8;
                *type = GR_GL_UNSIGNED_BYTE;   // unused I think
            } else {
                return false;
            }
            break;
        case kAlpha_8_GrPixelConfig:
            *format = GR_GL_ALPHA;
            *internalFormat = GR_GL_ALPHA;
            *type = GR_GL_UNSIGNED_BYTE;
            break;
        default:
            return false;
    }
    return true;
}

void GrGpuGL::setTextureUnit(int unit) {
    GrAssert(unit >= 0 && unit < kNumStages);
    if (fActiveTextureUnitIdx != unit) {
        GR_GL(ActiveTexture(GR_GL_TEXTURE0 + unit));
        fActiveTextureUnitIdx = unit;
    }
}

void GrGpuGL::setSpareTextureUnit() {
    if (fActiveTextureUnitIdx != (GR_GL_TEXTURE0 + SPARE_TEX_UNIT)) {
        GR_GL(ActiveTexture(GR_GL_TEXTURE0 + SPARE_TEX_UNIT));
        fActiveTextureUnitIdx = SPARE_TEX_UNIT;
    }
}

/* On ES the internalFormat and format must match for TexImage and we use
   GL_RGB, GL_RGBA for color formats. We also generally like having the driver
   decide the internalFormat. However, on ES internalFormat for
   RenderBufferStorage* has to be a specific format (not a base format like
   GL_RGBA).
 */
bool GrGpuGL::fboInternalFormat(GrPixelConfig config, GrGLenum* format) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kRGBX_8888_GrPixelConfig:
            if (fRGBA8Renderbuffer) {
                *format = GR_GL_RGBA8;
                return true;
            } else {
                return false;
            }
        case kRGB_565_GrPixelConfig:
            GrAssert(GR_GL_SUPPORT_ES);  // ES2 supports 565. ES1 supports it
                                         // with FBO extension desktop GL has
                                         // no such internal format
            *format = GR_GL_RGB565;
            return true;
        case kRGBA_4444_GrPixelConfig:
            *format = GR_GL_RGBA4;
            return true;
        default:
            return false;
    }
}

void GrGpuGL::resetDirtyFlags() {
    Gr_bzero(&fDirtyFlags, sizeof(fDirtyFlags));
}

void GrGpuGL::setBuffers(bool indexed,
                         int* extraVertexOffset,
                         int* extraIndexOffset) {

    GrAssert(NULL != extraVertexOffset);

    const GeometryPoolState& geoPoolState = this->getGeomPoolState();

    GrGLVertexBuffer* vbuf;
    switch (this->getGeomSrc().fVertexSrc) {
    case kBuffer_GeometrySrcType:
        *extraVertexOffset = 0;
        vbuf = (GrGLVertexBuffer*) this->getGeomSrc().fVertexBuffer;
        break;
    case kArray_GeometrySrcType:
    case kReserved_GeometrySrcType:
        this->finalizeReservedVertices();
        *extraVertexOffset = geoPoolState.fPoolStartVertex;
        vbuf = (GrGLVertexBuffer*) geoPoolState.fPoolVertexBuffer;
        break;
    default:
        vbuf = NULL; // suppress warning
        GrCrash("Unknown geometry src type!");
    }

    GrAssert(NULL != vbuf);
    GrAssert(!vbuf->isLocked());
    if (fHWGeometryState.fVertexBuffer != vbuf) {
        GR_GL(BindBuffer(GR_GL_ARRAY_BUFFER, vbuf->bufferID()));
        fHWGeometryState.fArrayPtrsDirty = true;
        fHWGeometryState.fVertexBuffer = vbuf;
    }

    if (indexed) {
        GrAssert(NULL != extraIndexOffset);

        GrGLIndexBuffer* ibuf;
        switch (this->getGeomSrc().fIndexSrc) {
        case kBuffer_GeometrySrcType:
            *extraIndexOffset = 0;
            ibuf = (GrGLIndexBuffer*)this->getGeomSrc().fIndexBuffer;
            break;
        case kArray_GeometrySrcType:
        case kReserved_GeometrySrcType:
            this->finalizeReservedIndices();
            *extraIndexOffset = geoPoolState.fPoolStartIndex;
            ibuf = (GrGLIndexBuffer*) geoPoolState.fPoolIndexBuffer;
            break;
        default:
            ibuf = NULL; // suppress warning
            GrCrash("Unknown geometry src type!");
        }

        GrAssert(NULL != ibuf);
        GrAssert(!ibuf->isLocked());
        if (fHWGeometryState.fIndexBuffer != ibuf) {
            GR_GL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, ibuf->bufferID()));
            fHWGeometryState.fIndexBuffer = ibuf;
        }
    }
}

int GrGpuGL::getMaxEdges() const {
    // FIXME:  This is a pessimistic estimate based on how many other things
    // want to add uniforms.  This should be centralized somewhere.
    return GR_CT_MIN(fMaxFragmentUniformVectors - 8, kMaxEdges);
}

