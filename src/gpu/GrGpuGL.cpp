
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuGL.h"
#include "GrGLStencilBuffer.h"
#include "GrTypes.h"
#include "SkTemplates.h"

static const GrGLuint GR_MAX_GLUINT = ~0;
static const GrGLint  GR_INVAL_GLINT = ~0;

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glInterface(), RET, X)

// we use a spare texture unit to avoid
// mucking with the state of any of the stages.
static const int SPARE_TEX_UNIT = GrDrawState::kNumStages;

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

static bool fbo_test(const GrGLInterface* gl, int w, int h) {

    GR_GL_CALL(gl, ActiveTexture(GR_GL_TEXTURE0 + SPARE_TEX_UNIT));

    GrGLuint testFBO;
    GR_GL_CALL(gl, GenFramebuffers(1, &testFBO));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, testFBO));
    GrGLuint testRTTex;
    GR_GL_CALL(gl, GenTextures(1, &testRTTex));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, testRTTex));
    // some implementations require texture to be mip-map complete before
    // FBO with level 0 bound as color attachment will be framebuffer complete.
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, 
                                 GR_GL_TEXTURE_MIN_FILTER,
                                 GR_GL_NEAREST));
    GR_GL_CALL(gl, TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA, w, h,
                              0, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, NULL));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    GR_GL_CALL(gl, FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_TEXTURE_2D, testRTTex, 0));
    GrGLenum status;
    GR_GL_CALL_RET(gl, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    GR_GL_CALL(gl, DeleteFramebuffers(1, &testFBO));
    GR_GL_CALL(gl, DeleteTextures(1, &testRTTex));

    return status == GR_GL_FRAMEBUFFER_COMPLETE;
}

static bool probe_for_npot_render_target_support(const GrGLInterface* gl,
                                                 bool hasNPOTTextureSupport) {

    /* Experimentation has found that some GLs that support NPOT textures
       do not support FBOs with a NPOT texture. They report "unsupported" FBO
       status. I don't know how to explicitly query for this. Do an
       experiment. Note they may support NPOT with a renderbuffer but not a
       texture. Presumably, the implementation bloats the renderbuffer
       internally to the next POT.
     */
    if (hasNPOTTextureSupport) {
        return fbo_test(gl, 200, 200);
    }
    return false;
}

static int probe_for_min_render_target_height(const GrGLInterface* gl,
                                              bool hasNPOTRenderTargetSupport,
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
        if (fbo_test(gl, w, h)) {
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

static int probe_for_min_render_target_width(const GrGLInterface* gl,
                                             bool hasNPOTRenderTargetSupport,
                                             int maxRenderTargetSize) {

    if (gPrintStartupSpew) {
        GrPrintf("Small width FBO texture experiments\n");
    }
    int minRenderTargetWidth = GR_INVAL_GLINT;
    for (GrGLuint i = 1; i <= 256; hasNPOTRenderTargetSupport ? i *= 2 : ++i) {
        GrGLuint w = i;
        GrGLuint h = maxRenderTargetSize;
        if (fbo_test(gl, w, h)) {
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

GrGpuGL::GrGpuGL(const GrGLInterface* gl, GrGLBinding glBinding) {

    fPrintedCaps = false;

    gl->ref();
    fGL = gl;
    fGLBinding = glBinding;
    switch (glBinding) {
        case kDesktop_GrGLBinding:
            GrAssert(gl->supportsDesktop());
            break;
        case kES1_GrGLBinding:
            GrAssert(gl->supportsES1());
            break;
        case kES2_GrGLBinding:
            GrAssert(gl->supportsES2());
            break;
        default:
            GrCrash("Expect exactly one valid GL binding bit to be in use.");
    }

    GrGLClearErr(fGL);

    const GrGLubyte* ext;
    GL_CALL_RET(ext, GetString(GR_GL_EXTENSIONS));
    if (gPrintStartupSpew) {
        const GrGLubyte* vendor;
        const GrGLubyte* renderer;
        const GrGLubyte* version;
        GL_CALL_RET(vendor, GetString(GR_GL_VENDOR));
        GL_CALL_RET(renderer, GetString(GR_GL_RENDERER));
        GL_CALL_RET(version, GetString(GR_GL_VERSION));
        GrPrintf("------------------------- create GrGpuGL %p --------------\n",
                 this);
        GrPrintf("------ VENDOR %s\n", vendor);
        GrPrintf("------ RENDERER %s\n", renderer);
        GrPrintf("------ VERSION %s\n",  version);
        GrPrintf("------ EXTENSIONS\n %s \n", ext);
    }

    fGLVersion = GrGLGetVersion(gl);
    GrAssert(0 != fGLVersion);
    fExtensionString = (const char*) ext;

    this->resetDirtyFlags();

    this->initCaps();

    fLastSuccessfulStencilFmtIdx = 0;
}

GrGpuGL::~GrGpuGL() {
    // This subclass must do this before the base class destructor runs
    // since we will unref the GrGLInterface.
    this->releaseResources();
    fGL->unref();
}

///////////////////////////////////////////////////////////////////////////////

static const GrGLuint kUnknownBitCount = ~0;

void GrGpuGL::initCaps() {
    GrGLint maxTextureUnits;
    // check FS and fixed-function texture unit limits
    // we only use textures in the fragment stage currently.
    // checks are > to make sure we have a spare unit.
    if (kES1_GrGLBinding != this->glBinding()) {
        GR_GL_GetIntegerv(fGL, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        GrAssert(maxTextureUnits > GrDrawState::kNumStages);
    }
    if (kES2_GrGLBinding != this->glBinding()) {
        GR_GL_GetIntegerv(fGL, GR_GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
        GrAssert(maxTextureUnits > GrDrawState::kNumStages);
    }
    if (kES2_GrGLBinding == this->glBinding()) {
        GR_GL_GetIntegerv(fGL, GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fGLCaps.fMaxFragmentUniformVectors);
    } else if (kDesktop_GrGLBinding != this->glBinding()) {
        GrGLint max;
        GR_GL_GetIntegerv(fGL, GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fGLCaps.fMaxFragmentUniformVectors = max / 4;
    } else {
        fGLCaps.fMaxFragmentUniformVectors = 16;
    }

    GrGLint numFormats;
    GR_GL_GetIntegerv(fGL, GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
    SkAutoSTMalloc<10, GrGLint> formats(numFormats);
    GR_GL_GetIntegerv(fGL, GR_GL_COMPRESSED_TEXTURE_FORMATS, formats);
    for (int i = 0; i < numFormats; ++i) {
        if (formats[i] == GR_GL_PALETTE8_RGBA8) {
            fCaps.f8BitPaletteSupport = true;
            break;
        }
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fCaps.fStencilWrapOpsSupport = (fGLVersion >= GR_GL_VER(1,4)) ||
                                    this->hasExtension("GL_EXT_stencil_wrap");
    } else {
        fCaps.fStencilWrapOpsSupport = (fGLVersion >= GR_GL_VER(2,0)) ||
                                this->hasExtension("GL_OES_stencil_wrap");
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        // we could also look for GL_ATI_separate_stencil extension or
        // GL_EXT_stencil_two_side but they use different function signatures
        // than GL2.0+ (and than each other).
        fCaps.fTwoSidedStencilSupport = (fGLVersion >= GR_GL_VER(2,0));
        // supported on GL 1.4 and higher or by extension
        fCaps.fStencilWrapOpsSupport = (fGLVersion >= GR_GL_VER(1,4)) ||
                                       this->hasExtension("GL_EXT_stencil_wrap");
    } else {
        // ES 2 has two sided stencil but 1.1 doesn't. There doesn't seem to be
        // an ES1 extension.
        fCaps.fTwoSidedStencilSupport = (fGLVersion >= GR_GL_VER(2,0));
        // stencil wrap support is in ES2, ES1 requires extension.
        fCaps.fStencilWrapOpsSupport = (fGLVersion >= GR_GL_VER(2,0)) ||
                                       this->hasExtension("GL_OES_stencil_wrap");
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fGLCaps.fRGBA8Renderbuffer = true;
    } else {
        fGLCaps.fRGBA8Renderbuffer = this->hasExtension("GL_OES_rgb8_rgba8");
    }


    if (kDesktop_GrGLBinding != this->glBinding()) {
        if (GR_GL_32BPP_COLOR_FORMAT == GR_GL_BGRA) {
            GrAssert(this->hasExtension("GL_EXT_texture_format_BGRA8888"));
        }
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fCaps.fBufferLockSupport = true; // we require VBO support and the desktop VBO
                                         // extension includes glMapBuffer.
    } else {
        fCaps.fBufferLockSupport = this->hasExtension("GL_OES_mapbuffer");
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        if (fGLVersion >= GR_GL_VER(2,0) || 
            this->hasExtension("GL_ARB_texture_non_power_of_two")) {
            fCaps.fNPOTTextureTileSupport = true;
            fCaps.fNPOTTextureSupport = true;
        } else {
            fCaps.fNPOTTextureTileSupport = false;
            fCaps.fNPOTTextureSupport = false;
        }
    } else {
        if (fGLVersion >= GR_GL_VER(2,0)) {
            fCaps.fNPOTTextureSupport = true;
            fCaps.fNPOTTextureTileSupport = this->hasExtension("GL_OES_texture_npot");
        } else {
            fCaps.fNPOTTextureSupport =
                        this->hasExtension("GL_APPLE_texture_2D_limited_npot");
            fCaps.fNPOTTextureTileSupport = false;
        }
    }

    fCaps.fHWAALineSupport = (kDesktop_GrGLBinding == this->glBinding());

    ////////////////////////////////////////////////////////////////////////////
    // Experiments to determine limitations that can't be queried.
    // TODO: Make these a preprocess that generate some compile time constants.
    // TODO: probe once at startup, rather than once per context creation.

    int expectNPOTTargets = fGL->fNPOTRenderTargetSupport;
    if (expectNPOTTargets == kProbe_GrGLCapability) {
        fCaps.fNPOTRenderTargetSupport =
            probe_for_npot_render_target_support(fGL, fCaps.fNPOTTextureSupport);
    } else {
        GrAssert(expectNPOTTargets == 0 || expectNPOTTargets == 1);
        fCaps.fNPOTRenderTargetSupport = (0 != expectNPOTTargets);
    }

    GR_GL_GetIntegerv(fGL, GR_GL_MAX_TEXTURE_SIZE, &fCaps.fMaxTextureSize);
    GR_GL_GetIntegerv(fGL, GR_GL_MAX_RENDERBUFFER_SIZE, &fCaps.fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fCaps.fMaxRenderTargetSize = GrMin(fCaps.fMaxTextureSize, fCaps.fMaxRenderTargetSize);

    fCaps.fMinRenderTargetHeight = fGL->fMinRenderTargetHeight;
    if (fCaps.fMinRenderTargetHeight == kProbe_GrGLCapability) {
        fCaps.fMinRenderTargetHeight =
            probe_for_min_render_target_height(fGL, fCaps.fNPOTRenderTargetSupport,
                                               fCaps.fMaxRenderTargetSize);
    }

    fCaps.fMinRenderTargetWidth = fGL->fMinRenderTargetWidth;
    if (fCaps.fMinRenderTargetWidth == kProbe_GrGLCapability) {
        fCaps.fMinRenderTargetWidth =
            probe_for_min_render_target_width(fGL, fCaps.fNPOTRenderTargetSupport,
                                              fCaps.fMaxRenderTargetSize);
    }

    this->initFSAASupport();
    this->initStencilFormats();
}

void GrGpuGL::initFSAASupport() {
    // TODO: Get rid of GrAALevel and use # samples directly.
    GR_STATIC_ASSERT(0 == kNone_GrAALevel);
    GR_STATIC_ASSERT(1 == kLow_GrAALevel);
    GR_STATIC_ASSERT(2 == kMed_GrAALevel);
    GR_STATIC_ASSERT(3 == kHigh_GrAALevel);
    memset(fGLCaps.fAASamples, 0, sizeof(fGLCaps.fAASamples));

    fGLCaps.fMSFBOType = GLCaps::kNone_MSFBO;
    if (kDesktop_GrGLBinding != this->glBinding()) {
       if (this->hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
           // chrome's extension is equivalent to the EXT msaa
           // and fbo_blit extensions.
            fGLCaps.fMSFBOType = GLCaps::kDesktopEXT_MSFBO;
       } else if (this->hasExtension("GL_APPLE_framebuffer_multisample")) {
            fGLCaps.fMSFBOType = GLCaps::kAppleES_MSFBO;
        }
    } else {
        if ((fGLVersion >= GR_GL_VER(3,0)) || this->hasExtension("GL_ARB_framebuffer_object")) {
            fGLCaps.fMSFBOType = GLCaps::kDesktopARB_MSFBO;
        } else if (this->hasExtension("GL_EXT_framebuffer_multisample") &&
                   this->hasExtension("GL_EXT_framebuffer_blit")) {
            fGLCaps.fMSFBOType = GLCaps::kDesktopEXT_MSFBO;
        }
    }

    if (GLCaps::kNone_MSFBO != fGLCaps.fMSFBOType) {
        GrGLint maxSamples;
        GR_GL_GetIntegerv(fGL, GR_GL_MAX_SAMPLES, &maxSamples);
        if (maxSamples > 1 ) {
            fGLCaps.fAASamples[kNone_GrAALevel] = 0;
            fGLCaps.fAASamples[kLow_GrAALevel] =
                GrMax(2, GrFixedFloorToInt((GR_FixedHalf) * maxSamples));
            fGLCaps.fAASamples[kMed_GrAALevel] =
                GrMax(2, GrFixedFloorToInt(((GR_Fixed1*3)/4) * maxSamples));
            fGLCaps.fAASamples[kHigh_GrAALevel] = maxSamples;
        }
    }
    fCaps.fFSAASupport = fGLCaps.fAASamples[kHigh_GrAALevel] > 0;
}

void GrGpuGL::initStencilFormats() {

    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    // these consts are in order of most preferred to least preferred
    // we don't bother with GL_STENCIL_INDEX1 or GL_DEPTH32F_STENCIL8
    static const GrGLStencilBuffer::Format
                  // internal Format      stencil bits      total bits        packed?
        gS8    = {GR_GL_STENCIL_INDEX8,   8,                8,                false},
        gS16   = {GR_GL_STENCIL_INDEX16,  16,               16,               false},
        gD24S8 = {GR_GL_DEPTH24_STENCIL8, 8,                32,               true },
        gS4    = {GR_GL_STENCIL_INDEX4,   4,                4,                false},
        gS     = {GR_GL_STENCIL_INDEX,    kUnknownBitCount, kUnknownBitCount, false},
        gDS    = {GR_GL_DEPTH_STENCIL,    kUnknownBitCount, kUnknownBitCount, true };

    if (kDesktop_GrGLBinding == this->glBinding()) {
        bool supportsPackedDS = fGLVersion >= GR_GL_VER(3,0) || 
                                this->hasExtension("GL_EXT_packed_depth_stencil") ||
                                this->hasExtension("GL_ARB_framebuffer_object");

        // S1 thru S16 formats are in GL 3.0+, EXT_FBO, and ARB_FBO since we
        // require FBO support we can expect these are legal formats and don't
        // check. These also all support the unsized GL_STENCIL_INDEX.
        fGLCaps.fStencilFormats.push_back() = gS8;
        fGLCaps.fStencilFormats.push_back() = gS16;
        if (supportsPackedDS) {
            fGLCaps.fStencilFormats.push_back() = gD24S8;
        }
        fGLCaps.fStencilFormats.push_back() = gS4;
        if (supportsPackedDS) {
            fGLCaps.fStencilFormats.push_back() = gDS;
        }
    } else {
        // ES2 has STENCIL_INDEX8 without extensions.
        // ES1 with GL_OES_framebuffer_object (which we require for ES1)
        // introduces tokens for S1 thu S8 but there are separate extensions
        // that make them legal (GL_OES_stencil1, ...).
        // GL_OES_packed_depth_stencil adds DEPTH24_STENCIL8
        // ES doesn't support using the unsized formats.

        if (fGLVersion >= GR_GL_VER(2,0) ||
            this->hasExtension("GL_OES_stencil8")) {
            fGLCaps.fStencilFormats.push_back() = gS8;
        }
        //fStencilFormats.push_back() = gS16;
        if (this->hasExtension("GL_OES_packed_depth_stencil")) {
            fGLCaps.fStencilFormats.push_back() = gD24S8;
        }
        if (this->hasExtension("GL_OES_stencil4")) {
            fGLCaps.fStencilFormats.push_back() = gS4;
        }
        // we require some stencil format.
        GrAssert(fGLCaps.fStencilFormats.count() > 0);
    }
}

void GrGpuGL::resetContext() {
    if (gPrintStartupSpew && !fPrintedCaps) {
        fPrintedCaps = true;
        this->getCaps().print();
        fGLCaps.print();
    }

    // We detect cases when blending is effectively off
    fHWBlendDisabled = false;
    GL_CALL(Enable(GR_GL_BLEND));

    // we don't use the zb at all
    GL_CALL(Disable(GR_GL_DEPTH_TEST));
    GL_CALL(DepthMask(GR_GL_FALSE));

    GL_CALL(Disable(GR_GL_CULL_FACE));
    GL_CALL(FrontFace(GR_GL_CCW));
    fHWDrawState.fDrawFace = GrDrawState::kBoth_DrawFace;

    GL_CALL(Disable(GR_GL_DITHER));
    if (kDesktop_GrGLBinding == this->glBinding()) {
        GL_CALL(Disable(GR_GL_LINE_SMOOTH));
        GL_CALL(Disable(GR_GL_POINT_SMOOTH));
        GL_CALL(Disable(GR_GL_MULTISAMPLE));
        fHWAAState.fMSAAEnabled = false;
        fHWAAState.fSmoothLineEnabled = false;
    }

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.fFlagBits = 0;

    // we only ever use lines in hairline mode
    GL_CALL(LineWidth(1));

    // invalid
    fActiveTextureUnitIdx = -1;

    // illegal values
    fHWDrawState.fSrcBlend = (GrBlendCoeff)-1;
    fHWDrawState.fDstBlend = (GrBlendCoeff)-1;

    fHWDrawState.fBlendConstant = 0x00000000;
    GL_CALL(BlendColor(0,0,0,0));

    fHWDrawState.fColor = GrColor_ILLEGAL;

    fHWDrawState.fViewMatrix = GrMatrix::InvalidMatrix();

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        fHWDrawState.fTextures[s] = NULL;
        fHWDrawState.fSamplerStates[s].setRadial2Params(-GR_ScalarMax,
                                                        -GR_ScalarMax,
                                                        true);
        fHWDrawState.fSamplerStates[s].setMatrix(GrMatrix::InvalidMatrix());
        fHWDrawState.fSamplerStates[s].setConvolutionParams(0, NULL, NULL);
    }

    fHWBounds.fScissorRect.invalidate();
    fHWBounds.fScissorEnabled = false;
    GL_CALL(Disable(GR_GL_SCISSOR_TEST));
    fHWBounds.fViewportRect.invalidate();

    fHWDrawState.fStencilSettings.invalidate();
    fHWStencilClip = false;
    fClipInStencil = false;

    fHWGeometryState.fIndexBuffer = NULL;
    fHWGeometryState.fVertexBuffer = NULL;
    
    fHWGeometryState.fArrayPtrsDirty = true;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.fRenderTarget = NULL;
}

GrResource* GrGpuGL::onCreatePlatformSurface(const GrPlatformSurfaceDesc& desc) {

    bool isTexture = kTexture_GrPlatformSurfaceType == desc.fSurfaceType ||
                     kTextureRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType;
    bool isRenderTarget = kRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType ||
                          kTextureRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType;

    GrGLRenderTarget::Desc rtDesc;
    SkAutoTUnref<GrGLStencilBuffer> sb;

    if (isRenderTarget) {
        rtDesc.fRTFBOID = desc.fPlatformRenderTarget;
        rtDesc.fConfig = desc.fConfig;
        if (desc.fSampleCnt) {
            if (kGrCanResolve_GrPlatformRenderTargetFlagBit  & desc.fRenderTargetFlags) {
                rtDesc.fTexFBOID = desc.fPlatformResolveDestination;
            } else {
                GrAssert(!isTexture); // this should have been filtered by GrContext
                rtDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
            }
        } else {
            rtDesc.fTexFBOID = desc.fPlatformRenderTarget;
        }
        // we don't know what the RB ids are without glGets and we don't care
        // since we aren't responsible for deleting them.
        rtDesc.fMSColorRenderbufferID = 0;
        rtDesc.fSampleCnt = desc.fSampleCnt;
        if (desc.fStencilBits) {
            GrGLStencilBuffer::Format format;
            format.fInternalFormat = GrGLStencilBuffer::kUnknownInternalFormat;
            format.fPacked = false;
            format.fStencilBits = desc.fStencilBits;
            format.fTotalBits = desc.fStencilBits;
            sb.reset(new GrGLStencilBuffer(this, 0, desc.fWidth, desc.fHeight,
                                           rtDesc.fSampleCnt, format));
        }
        rtDesc.fOwnIDs = false;
    }

    if (isTexture) {
        GrGLTexture::Desc texDesc;
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
        texDesc.fTextureID          = desc.fPlatformTexture;
        texDesc.fUploadByteCount    = GrBytesPerPixel(desc.fConfig);
        texDesc.fOwnsID             = false;

        params.invalidate(); // rather than do glGets.
        if (isRenderTarget) {
            GrTexture* tex = new GrGLTexture(this, texDesc, rtDesc, params);
            tex->asRenderTarget()->setStencilBuffer(sb.get());
            return tex;
        } else {
            return new GrGLTexture(this, texDesc, params);
        }
    } else {
        GrGLIRect viewport;
        viewport.fLeft   = 0;
        viewport.fBottom = 0;
        viewport.fWidth  = desc.fWidth;
        viewport.fHeight = desc.fHeight;

        GrGLRenderTarget* rt =  new GrGLRenderTarget(this, rtDesc, viewport);
        rt->setStencilBuffer(sb.get());
        return rt;
    }
}


////////////////////////////////////////////////////////////////////////////////

void GrGpuGL::allocateAndUploadTexData(const GrGLTexture::Desc& desc,
                                       GrGLenum internalFormat,
                                       const void* data,
                                       size_t rowBytes) {
    // we assume the texture is bound;
    if (!rowBytes) {
        rowBytes = desc.fUploadByteCount * desc.fContentWidth;
    }

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    /*
     * check whether to allocate a temporary buffer for flipping y or
     * because our data has extra bytes past each row. If so, we need
     * to trim those off here, since GL ES doesn't let us specify
     * GL_UNPACK_ROW_LENGTH.
     */
    bool flipY = GrGLTexture::kBottomUp_Orientation == desc.fOrientation;
    if (kDesktop_GrGLBinding == this->glBinding() && !flipY) {
        if (data && rowBytes != desc.fContentWidth * desc.fUploadByteCount) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH,
                                rowBytes / desc.fUploadByteCount));
        }
    } else {
        size_t trimRowBytes = desc.fContentWidth * desc.fUploadByteCount;
        if (data && (trimRowBytes < rowBytes || flipY)) {
            // copy the data into our new storage, skipping the trailing bytes
            size_t trimSize = desc.fContentHeight * trimRowBytes;
            const char* src = (const char*)data;
            if (flipY) {
                src += (desc.fContentHeight - 1) * rowBytes;
            }
            char* dst = (char*)tempStorage.reset(trimSize);
            for (int y = 0; y < desc.fContentHeight; y++) {
                memcpy(dst, src, trimRowBytes);
                if (flipY) {
                    src -= rowBytes;
                } else {
                    src += rowBytes;
                }
                dst += trimRowBytes;
            }
            // now point data to our trimmed version
            data = tempStorage.get();
            rowBytes = trimRowBytes;
        }
    }

    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, desc.fUploadByteCount));
    if (kIndex_8_GrPixelConfig == desc.fFormat &&
        this->getCaps().f8BitPaletteSupport) {
        // ES only supports CompressedTexImage2D, not CompressedTexSubimage2D
        GrAssert(desc.fContentWidth == desc.fAllocWidth);
        GrAssert(desc.fContentHeight == desc.fAllocHeight);
        GrGLsizei imageSize = desc.fAllocWidth * desc.fAllocHeight +
                              kGrColorTableSize;
        GL_CALL(CompressedTexImage2D(GR_GL_TEXTURE_2D, 0, desc.fUploadFormat,
                                     desc.fAllocWidth, desc.fAllocHeight,
                                     0, imageSize, data));
        GrGLResetRowLength(this->glInterface());
    } else {
        if (NULL != data && (desc.fAllocWidth != desc.fContentWidth ||
                                desc.fAllocHeight != desc.fContentHeight)) {
            GL_CALL(TexImage2D(GR_GL_TEXTURE_2D, 0, internalFormat,
                               desc.fAllocWidth, desc.fAllocHeight,
                               0, desc.fUploadFormat, desc.fUploadType, NULL));
            GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, 0, 0, desc.fContentWidth,
                                  desc.fContentHeight, desc.fUploadFormat,
                                  desc.fUploadType, data));
            GrGLResetRowLength(this->glInterface());

            int extraW = desc.fAllocWidth  - desc.fContentWidth;
            int extraH = desc.fAllocHeight - desc.fContentHeight;
            int maxTexels = extraW * extraH;
            maxTexels = GrMax(extraW * desc.fContentHeight, maxTexels);
            maxTexels = GrMax(desc.fContentWidth * extraH, maxTexels);

            SkAutoSMalloc<128*128> texels(desc.fUploadByteCount * maxTexels);

            // rowBytes is actual stride between rows in data
            // rowDataBytes is the actual amount of non-pad data in a row
            // and the stride used for uploading extraH rows.
            uint32_t rowDataBytes = desc.fContentWidth * desc.fUploadByteCount;
            if (extraH) {
                uint8_t* lastRowStart = (uint8_t*) data +
                                        (desc.fContentHeight - 1) * rowBytes;
                uint8_t* extraRowStart = (uint8_t*)texels.get();

                for (int i = 0; i < extraH; ++i) {
                    memcpy(extraRowStart, lastRowStart, rowDataBytes);
                    extraRowStart += rowDataBytes;
                }
                GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, 0,
                                      desc.fContentHeight, desc.fContentWidth,
                                      extraH, desc.fUploadFormat,
                                      desc.fUploadType, texels.get()));
            }
            if (extraW) {
                uint8_t* edgeTexel = (uint8_t*)data +
                                     rowDataBytes - desc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (int j = 0; j < desc.fContentHeight; ++j) {
                    for (int i = 0; i < extraW; ++i) {
                        memcpy(extraTexel, edgeTexel, desc.fUploadByteCount);
                        extraTexel += desc.fUploadByteCount;
                    }
                    edgeTexel += rowBytes;
                }
                GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, desc.fContentWidth,
                                      0, extraW, desc.fContentHeight,
                                      desc.fUploadFormat, desc.fUploadType,
                                      texels.get()));
            }
            if (extraW && extraH) {
                uint8_t* cornerTexel = (uint8_t*)data + 
                                       desc.fContentHeight * rowBytes -
                                       desc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (int i = 0; i < extraW*extraH; ++i) {
                    memcpy(extraTexel, cornerTexel, desc.fUploadByteCount);
                    extraTexel += desc.fUploadByteCount;
                }
                GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, desc.fContentWidth,
                                      desc.fContentHeight, extraW, extraH, 
                                      desc.fUploadFormat, desc.fUploadType,
                                      texels.get()));
            }

        } else {
            GL_CALL(TexImage2D(GR_GL_TEXTURE_2D, 0, internalFormat,
                               desc.fAllocWidth, desc.fAllocHeight, 0,
                               desc.fUploadFormat, desc.fUploadType, data));
            GrGLResetRowLength(this->glInterface());
        }
    }
}

bool GrGpuGL::createRenderTargetObjects(int width, int height,
                                        GrGLuint texID,
                                        GrGLRenderTarget::Desc* desc) {
    desc->fMSColorRenderbufferID = 0;
    desc->fRTFBOID = 0;
    desc->fTexFBOID = 0;
    desc->fOwnIDs = true;

    GrGLenum status;
    GrGLint err;

    GrGLenum msColorFormat = 0; // suppress warning

    GL_CALL(GenFramebuffers(1, &desc->fTexFBOID));
    if (!desc->fTexFBOID) {
        goto FAILED;
    }
    

    // If we are using multisampling we will create two FBOS. We render
    // to one and then resolve to the texture bound to the other.
    if (desc->fSampleCnt > 1 && GLCaps::kNone_MSFBO != fGLCaps.fMSFBOType) {
        GL_CALL(GenFramebuffers(1, &desc->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &desc->fMSColorRenderbufferID));
        if (!desc->fRTFBOID ||
            !desc->fMSColorRenderbufferID || 
            !this->fboInternalFormat(desc->fConfig, &msColorFormat)) {
            goto FAILED;
        }
    } else {
        desc->fRTFBOID = desc->fTexFBOID;
    }

    if (desc->fRTFBOID != desc->fTexFBOID) {
        GrAssert(desc->fSampleCnt > 1);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER,
                               desc->fMSColorRenderbufferID));
        GR_GL_CALL_NOERRCHECK(this->glInterface(),
                              RenderbufferStorageMultisample(GR_GL_RENDERBUFFER, 
                                                             desc->fSampleCnt,
                                                             msColorFormat,
                                                             width, height));
        err = GR_GL_GET_ERROR(this->glInterface());
        if (err != GR_GL_NO_ERROR) {
            goto FAILED;
        }
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, desc->fRTFBOID));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, 
                                      GR_GL_COLOR_ATTACHMENT0,
                                      GR_GL_RENDERBUFFER,
                                      desc->fMSColorRenderbufferID));
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            goto FAILED;
        }
    }
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, desc->fTexFBOID));

    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                 GR_GL_COLOR_ATTACHMENT0,
                                 GR_GL_TEXTURE_2D,
                                 texID, 0));
    GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
        goto FAILED;
    }

    return true;

FAILED:
    if (desc->fMSColorRenderbufferID) {
        GL_CALL(DeleteRenderbuffers(1, &desc->fMSColorRenderbufferID));
    }
    if (desc->fRTFBOID != desc->fTexFBOID) {
        GL_CALL(DeleteFramebuffers(1, &desc->fRTFBOID));
    }
    if (desc->fTexFBOID) {
        GL_CALL(DeleteFramebuffers(1, &desc->fTexFBOID));
    }
    return false;
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

    static const GrGLTexture::TexParams DEFAULT_PARAMS = {
        GR_GL_NEAREST,
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_CLAMP_TO_EDGE
    };

    GrGLTexture::Desc glTexDesc;
    GrGLRenderTarget::Desc  glRTDesc;
    GrGLenum internalFormat;

    glTexDesc.fContentWidth  = desc.fWidth;
    glTexDesc.fContentHeight = desc.fHeight;
    glTexDesc.fAllocWidth    = desc.fWidth;
    glTexDesc.fAllocHeight   = desc.fHeight;
    glTexDesc.fFormat        = desc.fFormat;
    glTexDesc.fOwnsID        = true;

    glRTDesc.fMSColorRenderbufferID = 0;
    glRTDesc.fRTFBOID = 0;
    glRTDesc.fTexFBOID = 0;
    glRTDesc.fOwnIDs = true;
    glRTDesc.fConfig = glTexDesc.fFormat;

    bool renderTarget = 0 != (desc.fFlags & kRenderTarget_GrTextureFlagBit);
    if (!canBeTexture(desc.fFormat,
                      &internalFormat,
                      &glTexDesc.fUploadFormat,
                      &glTexDesc.fUploadType)) {
        return return_null_texture();
    }

    const Caps& caps = this->getCaps();

    // We keep GrRenderTargets in GL's normal orientation so that they
    // can be drawn to by the outside world without the client having
    // to render upside down.
    glTexDesc.fOrientation = renderTarget ? GrGLTexture::kBottomUp_Orientation :
                                            GrGLTexture::kTopDown_Orientation;

    GrAssert(as_size_t(desc.fAALevel) < GR_ARRAY_COUNT(fGLCaps.fAASamples));
    glRTDesc.fSampleCnt = fGLCaps.fAASamples[desc.fAALevel];
    if (GLCaps::kNone_MSFBO == fGLCaps.fMSFBOType &&
        desc.fAALevel != kNone_GrAALevel) {
        GrPrintf("AA RT requested but not supported on this platform.");
    }

    glTexDesc.fUploadByteCount = GrBytesPerPixel(desc.fFormat);

    if (renderTarget) {
        if (!caps.fNPOTRenderTargetSupport) {
            glTexDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
            glTexDesc.fAllocHeight = GrNextPow2(desc.fHeight);
        }

        glTexDesc.fAllocWidth = GrMax(caps.fMinRenderTargetWidth,
                                      glTexDesc.fAllocWidth);
        glTexDesc.fAllocHeight = GrMax(caps.fMinRenderTargetHeight,
                                       glTexDesc.fAllocHeight);
        if (glTexDesc.fAllocWidth > caps.fMaxRenderTargetSize ||
            glTexDesc.fAllocHeight > caps.fMaxRenderTargetSize) {
            return return_null_texture();
        }
    } else if (!caps.fNPOTTextureSupport) {
        glTexDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
        glTexDesc.fAllocHeight = GrNextPow2(desc.fHeight);
        if (glTexDesc.fAllocWidth > caps.fMaxTextureSize ||
            glTexDesc.fAllocHeight > caps.fMaxTextureSize) {
            return return_null_texture();
        }
    }

    GL_CALL(GenTextures(1, &glTexDesc.fTextureID));
    if (!glTexDesc.fTextureID) {
        return return_null_texture();
    }

    this->setSpareTextureUnit();
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, glTexDesc.fTextureID));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MAG_FILTER,
                          DEFAULT_PARAMS.fFilter));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MIN_FILTER,
                          DEFAULT_PARAMS.fFilter));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_WRAP_S,
                          DEFAULT_PARAMS.fWrapS));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_WRAP_T,
                          DEFAULT_PARAMS.fWrapT));

    this->allocateAndUploadTexData(glTexDesc, internalFormat,srcData, rowBytes);

    GrGLTexture* tex;
    if (renderTarget) {
        GrGLenum msColorRenderbufferFormat = -1;
#if GR_COLLECT_STATS
        ++fStats.fRenderTargetCreateCnt;
#endif
        if (!this->createRenderTargetObjects(glTexDesc.fAllocWidth,
                                             glTexDesc.fAllocHeight,
                                             glTexDesc.fTextureID,
                                             &glRTDesc)) {
            GL_CALL(DeleteTextures(1, &glTexDesc.fTextureID));
            return return_null_texture();
        }
        tex = new GrGLTexture(this, glTexDesc, glRTDesc, DEFAULT_PARAMS);
    } else {
        tex = new GrGLTexture(this, glTexDesc, DEFAULT_PARAMS);
    }
#ifdef TRACE_TEXTURE_CREATION
    GrPrintf("--- new texture [%d] size=(%d %d) bpp=%d\n",
             tex->fTextureID, width, height, tex->fUploadByteCount);
#endif
    return tex;
}

namespace {
void inline get_stencil_rb_sizes(const GrGLInterface* gl,
                                 GrGLuint rb, 
                                 GrGLStencilBuffer::Format* format) {
    // we shouldn't ever know one size and not the other
    GrAssert((kUnknownBitCount == format->fStencilBits) ==
             (kUnknownBitCount == format->fTotalBits));
    if (kUnknownBitCount == format->fStencilBits) {
        GR_GL_GetRenderbufferParameteriv(gl, GR_GL_RENDERBUFFER,
                                         GR_GL_RENDERBUFFER_STENCIL_SIZE,
                                         (GrGLint*)&format->fStencilBits);
        if (format->fPacked) {
            GR_GL_GetRenderbufferParameteriv(gl, GR_GL_RENDERBUFFER,
                                             GR_GL_RENDERBUFFER_DEPTH_SIZE,
                                             (GrGLint*)&format->fTotalBits);
            format->fTotalBits += format->fStencilBits;
        } else {
            format->fTotalBits = format->fStencilBits;
        }
    }
}
}

bool GrGpuGL::createStencilBufferForRenderTarget(GrRenderTarget* rt,
                                                 int width, int height) {

    // All internally created RTs are also textures. We don't create
    // SBs for a client's standalone RT (that is RT that isnt also a texture).
    GrAssert(rt->asTexture());
    GrAssert(width >= rt->allocatedWidth());
    GrAssert(height >= rt->allocatedHeight());

    int samples = rt->numSamples();
    GrGLuint sbID;
    GL_CALL(GenRenderbuffers(1, &sbID));
    if (!sbID) {
        return false;
    }

    GrGLStencilBuffer* sb = NULL;

    int stencilFmtCnt = fGLCaps.fStencilFormats.count();
    for (int i = 0; i < stencilFmtCnt; ++i) {
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, sbID));
        // we start with the last stencil format that succeeded in hopes
        // that we won't go through this loop more than once after the
        // first (painful) stencil creation.
        int sIdx = (i + fLastSuccessfulStencilFmtIdx) % stencilFmtCnt;
        const GrGLStencilBuffer::Format& sFmt = fGLCaps.fStencilFormats[sIdx];
        // we do this "if" so that we don't call the multisample
        // version on a GL that doesn't have an MSAA extension.
        if (samples > 1) {
            GR_GL_CALL_NOERRCHECK(this->glInterface(),
                                  RenderbufferStorageMultisample(
                                        GR_GL_RENDERBUFFER,
                                        samples,
                                        sFmt.fInternalFormat,
                                        width,
                                        height));
        } else {
            GR_GL_CALL_NOERRCHECK(this->glInterface(),
                                  RenderbufferStorage(GR_GL_RENDERBUFFER,
                                                      sFmt.fInternalFormat,
                                                      width, height));
        }

        GrGLenum err = GR_GL_GET_ERROR(this->glInterface());
        if (err == GR_GL_NO_ERROR) {
            // After sized formats we attempt an unsized format and take whatever
            // sizes GL gives us. In that case we query for the size.
            GrGLStencilBuffer::Format format = sFmt;
            get_stencil_rb_sizes(this->glInterface(), sbID, &format);
            sb = new GrGLStencilBuffer(this, sbID, width, height, 
                                       samples, format);
            if (this->attachStencilBufferToRenderTarget(sb, rt)) {
                fLastSuccessfulStencilFmtIdx = sIdx;
                rt->setStencilBuffer(sb);
                sb->unref();
                return true;
           }
           sb->abandon(); // otherwise we lose sbID
           sb->unref();
        }
    }
    GL_CALL(DeleteRenderbuffers(1, &sbID));
    return false;
}

bool GrGpuGL::attachStencilBufferToRenderTarget(GrStencilBuffer* sb,
                                                GrRenderTarget* rt) {
    GrGLRenderTarget* glrt = (GrGLRenderTarget*) rt;

    GrGLuint fbo = glrt->renderFBOID();

    if (NULL == sb) {
        if (NULL != rt->getStencilBuffer()) {
            GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                          GR_GL_STENCIL_ATTACHMENT,
                                          GR_GL_RENDERBUFFER, 0));
            GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                          GR_GL_DEPTH_ATTACHMENT,
                                          GR_GL_RENDERBUFFER, 0));
#if GR_DEBUG
            GrGLenum status;
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            GrAssert(GR_GL_FRAMEBUFFER_COMPLETE == status);
#endif
        }
        return true;
    } else {
        GrGLStencilBuffer* glsb = (GrGLStencilBuffer*) sb;
        GrGLuint rb = glsb->renderbufferID();

        fHWDrawState.fRenderTarget = NULL;
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, fbo));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                      GR_GL_STENCIL_ATTACHMENT,
                                      GR_GL_RENDERBUFFER, rb));
        if (glsb->format().fPacked) {
            GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                          GR_GL_DEPTH_ATTACHMENT,
                                          GR_GL_RENDERBUFFER, rb));
        } else {
            GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                          GR_GL_DEPTH_ATTACHMENT,
                                          GR_GL_RENDERBUFFER, 0));
        }

        GrGLenum status;
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                          GR_GL_STENCIL_ATTACHMENT,
                                          GR_GL_RENDERBUFFER, 0));
            if (glsb->format().fPacked) {
                GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                              GR_GL_DEPTH_ATTACHMENT,
                                              GR_GL_RENDERBUFFER, 0));
            }
            return false;
        } else {
            return true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBuffer* GrGpuGL::onCreateVertexBuffer(uint32_t size, bool dynamic) {
    GrGLuint id;
    GL_CALL(GenBuffers(1, &id));
    if (id) {
        GL_CALL(BindBuffer(GR_GL_ARRAY_BUFFER, id));
        fHWGeometryState.fArrayPtrsDirty = true;
        GrGLClearErr(this->glInterface());
        // make sure driver can allocate memory for this buffer
        GR_GL_CALL_NOERRCHECK(this->glInterface(),
                              BufferData(GR_GL_ARRAY_BUFFER, size, NULL, 
                              dynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
        if (this->glInterface()->fGetError() != GR_GL_NO_ERROR) {
            GL_CALL(DeleteBuffers(1, &id));
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
    GL_CALL(GenBuffers(1, &id));
    if (id) {
        GL_CALL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, id));
        GrGLClearErr(this->glInterface());
        // make sure driver can allocate memory for this buffer
        GR_GL_CALL_NOERRCHECK(this->glInterface(),
                              BufferData(GR_GL_ELEMENT_ARRAY_BUFFER, size, NULL,
                              dynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
        if (this->glInterface()->fGetError() != GR_GL_NO_ERROR) {
            GL_CALL(DeleteBuffers(1, &id));
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
            scissor.pushToGLScissor(this->glInterface());
            fHWBounds.fScissorRect = scissor;
        }
        if (!fHWBounds.fScissorEnabled) {
            GL_CALL(Enable(GR_GL_SCISSOR_TEST));
            fHWBounds.fScissorEnabled = true;
        }
    } else {
        if (fHWBounds.fScissorEnabled) {
            GL_CALL(Disable(GR_GL_SCISSOR_TEST));
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
    GL_CALL(ColorMask(GR_GL_TRUE,GR_GL_TRUE,GR_GL_TRUE,GR_GL_TRUE));
    fHWDrawState.fFlagBits &= ~kNoColorWrites_StateBit;
    GL_CALL(ClearColor(GrColorUnpackR(color)/255.f,
                       GrColorUnpackG(color)/255.f,
                       GrColorUnpackB(color)/255.f,
                       GrColorUnpackA(color)/255.f));
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGpuGL::clearStencil() {
    if (NULL == fCurrDrawState.fRenderTarget) {
        return;
    }
    
    this->flushRenderTarget(&GrIRect::EmptyIRect());

    if (fHWBounds.fScissorEnabled) {
        GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        fHWBounds.fScissorEnabled = false;
    }
    GL_CALL(StencilMask(0xffffffff));
    GL_CALL(ClearStencil(0));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWDrawState.fStencilSettings.invalidate();
}

void GrGpuGL::clearStencilClip(const GrIRect& rect, bool insideClip) {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    // this should only be called internally when we know we have a
    // stencil buffer.
    GrAssert(NULL != fCurrDrawState.fRenderTarget->getStencilBuffer());
    GrGLint stencilBitCount = 
        fCurrDrawState.fRenderTarget->getStencilBuffer()->bits();
#if 0
    GrAssert(stencilBitCount > 0);
    GrGLint clipStencilMask  = (1 << (stencilBitCount - 1));
#else
    // we could just clear the clip bit but when we go through
    // ANGLE a partial stencil mask will cause clears to be
    // turned into draws. Our contract on GrDrawTarget says that
    // changing the clip between stencil passes may or may not
    // zero the client's clip bits. So we just clear the whole thing.
    static const GrGLint clipStencilMask  = ~0;
#endif
    GrGLint value;
    if (insideClip) {
        value = (1 << (stencilBitCount - 1));
    } else {
        value = 0;
    }
    this->flushRenderTarget(&GrIRect::EmptyIRect());
    this->flushScissor(&rect);
    GL_CALL(StencilMask(clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
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
            GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER,
                                    tgt->textureFBOID()));
            break;
        default:
            GrCrash("Unknown resolve type");
    }

    const GrGLIRect& glvp = tgt->getViewport();

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height);
    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom,
                       readRect.fWidth, readRect.fHeight,
                       format, type, buffer));

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom
    {
        size_t stride = width * GrBytesPerPixel(config);
        SkAutoMalloc rowStorage(stride);
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
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, rt->renderFBOID()));
    #if GR_COLLECT_STATS
        ++fStats.fRenderTargetChngCnt;
    #endif
    #if GR_DEBUG
        GrGLenum status;
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            GrPrintf("GrGpuGL::flushRenderTarget glCheckFramebufferStatus %x\n", status);
        }
    #endif
        fDirtyFlags.fRenderTargetChanged = true;
        fHWDrawState.fRenderTarget = fCurrDrawState.fRenderTarget;
        const GrGLIRect& vp = rt->getViewport();
        if (fHWBounds.fViewportRect != vp) {
            vp.pushToGLViewport(this->glInterface());
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

    GL_CALL(DrawElements(gPrimitiveType2GLMode[type], indexCount,
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
    GL_CALL(DrawArrays(gPrimitiveType2GLMode[type], 0, vertexCount));
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
        GrAssert(GLCaps::kNone_MSFBO != fGLCaps.fMSFBOType);
        GrAssert(rt->textureFBOID() != rt->renderFBOID());
        GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER,
                                rt->renderFBOID()));
        GL_CALL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER,
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

        if (GLCaps::kAppleES_MSFBO == fGLCaps.fMSFBOType) {
            // Apple's extension uses the scissor as the blit bounds.
            GL_CALL(Enable(GR_GL_SCISSOR_TEST));
            GL_CALL(Scissor(r.fLeft, r.fBottom,
                            r.fWidth, r.fHeight));
            GL_CALL(ResolveMultisampleFramebuffer());
            fHWBounds.fScissorRect.invalidate();
            fHWBounds.fScissorEnabled = true;
        } else {
            if (GLCaps::kDesktopARB_MSFBO != fGLCaps.fMSFBOType) {
                // this respects the scissor during the blit, so disable it.
                GrAssert(GLCaps::kDesktopEXT_MSFBO == fGLCaps.fMSFBOType);
                this->flushScissor(NULL);
            }
            int right = r.fLeft + r.fWidth;
            int top = r.fBottom + r.fHeight;
            GL_CALL(BlitFramebuffer(r.fLeft, r.fBottom, right, top,
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
    bool stencilClip = fClipInStencil &&
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
            GL_CALL(Disable(GR_GL_STENCIL_TEST));
        } else {
            GL_CALL(Enable(GR_GL_STENCIL_TEST));
    #if GR_DEBUG
            if (!this->getCaps().fStencilWrapOpsSupport) {
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
            int stencilBits = 0;
            GrStencilBuffer* stencilBuffer = 
                            fCurrDrawState.fRenderTarget->getStencilBuffer();
            if (NULL != stencilBuffer) {
                stencilBits = stencilBuffer->bits();
            }
            // TODO: dynamically attach a stencil buffer
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
            if (this->getCaps().fTwoSidedStencilSupport) {
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

                GL_CALL(StencilFuncSeparate(GR_GL_FRONT, frontFunc,
                                            frontRef, frontMask));
                GL_CALL(StencilMaskSeparate(GR_GL_FRONT, frontWriteMask));
                GL_CALL(StencilFuncSeparate(GR_GL_BACK, backFunc,
                                            backRef, backMask));
                GL_CALL(StencilMaskSeparate(GR_GL_BACK, backWriteMask));
                GL_CALL(StencilOpSeparate(GR_GL_FRONT,
                                    grToGLStencilOp[settings->fFrontFailOp],
                                    grToGLStencilOp[settings->fFrontPassOp],
                                    grToGLStencilOp[settings->fFrontPassOp]));

                GL_CALL(StencilOpSeparate(GR_GL_BACK,
                                    grToGLStencilOp[settings->fBackFailOp],
                                    grToGLStencilOp[settings->fBackPassOp],
                                    grToGLStencilOp[settings->fBackPassOp]));
            } else {
                GL_CALL(StencilFunc(frontFunc, frontRef, frontMask));
                GL_CALL(StencilMask(frontWriteMask));
                GL_CALL(StencilOp(grToGLStencilOp[settings->fFrontFailOp],
                                grToGLStencilOp[settings->fFrontPassOp],
                                grToGLStencilOp[settings->fFrontPassOp]));
            }
        }
        fHWDrawState.fStencilSettings = fCurrDrawState.fStencilSettings;
        fHWStencilClip = stencilClip;
    }
}

void GrGpuGL::flushAAState(GrPrimitiveType type) {
    if (kDesktop_GrGLBinding == this->glBinding()) {
        // ES doesn't support toggling GL_MULTISAMPLE and doesn't have
        // smooth lines.

        // we prefer smooth lines over multisampled lines
        // msaa should be disabled if drawing smooth lines.
        if (GrIsPrimTypeLines(type)) {
            bool smooth = this->willUseHWAALines();
            if (!fHWAAState.fSmoothLineEnabled && smooth) {
                GL_CALL(Enable(GR_GL_LINE_SMOOTH));
                fHWAAState.fSmoothLineEnabled = true;
            } else if (fHWAAState.fSmoothLineEnabled && !smooth) {
                GL_CALL(Disable(GR_GL_LINE_SMOOTH));
                fHWAAState.fSmoothLineEnabled = false;
            }
            if (fCurrDrawState.fRenderTarget->isMultisampled() && 
                fHWAAState.fMSAAEnabled) {
                GL_CALL(Disable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = false;
            }
        } else if (fCurrDrawState.fRenderTarget->isMultisampled() &&
                   SkToBool(kHWAntialias_StateBit & fCurrDrawState.fFlagBits) !=
                   fHWAAState.fMSAAEnabled) {
            if (fHWAAState.fMSAAEnabled) {
                GL_CALL(Disable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = false;
            } else {
                GL_CALL(Enable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = true;
            }
        }
    }
}

void GrGpuGL::flushBlend(GrPrimitiveType type,
                         GrBlendCoeff srcCoeff,
                         GrBlendCoeff dstCoeff) {
    if (GrIsPrimTypeLines(type) && this->willUseHWAALines()) {
        if (fHWBlendDisabled) {
            GL_CALL(Enable(GR_GL_BLEND));
            fHWBlendDisabled = false;
        }
        if (kSA_BlendCoeff != fHWDrawState.fSrcBlend ||
            kISA_BlendCoeff != fHWDrawState.fDstBlend) {
            GL_CALL(BlendFunc(gXfermodeCoeff2Blend[kSA_BlendCoeff],
                              gXfermodeCoeff2Blend[kISA_BlendCoeff]));
            fHWDrawState.fSrcBlend = kSA_BlendCoeff;
            fHWDrawState.fDstBlend = kISA_BlendCoeff;
        }
    } else {
        // any optimization to disable blending should
        // have already been applied and tweaked the coeffs
        // to (1, 0).
        bool blendOff = kOne_BlendCoeff == srcCoeff &&
                        kZero_BlendCoeff == dstCoeff;
        if (fHWBlendDisabled != blendOff) {
            if (blendOff) {
                GL_CALL(Disable(GR_GL_BLEND));
            } else {
                GL_CALL(Enable(GR_GL_BLEND));
            }
            fHWBlendDisabled = blendOff;
        }
        if (!blendOff) {
            if (fHWDrawState.fSrcBlend != srcCoeff ||
                fHWDrawState.fDstBlend != dstCoeff) {
                GL_CALL(BlendFunc(gXfermodeCoeff2Blend[srcCoeff],
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
                GL_CALL(BlendColor(c[0], c[1], c[2], c[3]));
                fHWDrawState.fBlendConstant = fCurrDrawState.fBlendConstant;
            }
        }
    }
}

static unsigned grToGLFilter(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::kBilinear_Filter:
        case GrSamplerState::k4x4Downsample_Filter:
            return GR_GL_LINEAR;
        case GrSamplerState::kNearest_Filter:
        case GrSamplerState::kConvolution_Filter:
            return GR_GL_NEAREST;
        default:
            GrAssert(!"Unknown filter type");
            return GR_GL_LINEAR;
    }
}

bool GrGpuGL::flushGLStateCommon(GrPrimitiveType type) {

    // GrGpu::setupClipAndFlushState should have already checked this
    // and bailed if not true.
    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
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
                GL_CALL(BindTexture(GR_GL_TEXTURE_2D, nextTexture->textureID()));
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

            newTexParams.fFilter = grToGLFilter(sampler.getFilter());

            const GrGLenum* wraps = 
                                GrGLTexture::WrapMode2GLWrap(this->glBinding());
            newTexParams.fWrapS = wraps[sampler.getWrapX()];
            newTexParams.fWrapT = wraps[sampler.getWrapY()];

            if (newTexParams.fFilter != oldTexParams.fFilter) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                      GR_GL_TEXTURE_MAG_FILTER,
                                      newTexParams.fFilter));
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                      GR_GL_TEXTURE_MIN_FILTER,
                                      newTexParams.fFilter));
            }
            if (newTexParams.fWrapS != oldTexParams.fWrapS) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                      GR_GL_TEXTURE_WRAP_S,
                                      newTexParams.fWrapS));
            }
            if (newTexParams.fWrapT != oldTexParams.fWrapT) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
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
            GL_CALL(Enable(GR_GL_DITHER));
        } else {
            GL_CALL(Disable(GR_GL_DITHER));
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
        GL_CALL(ColorMask(mask, mask, mask, mask));
    }

    if (fHWDrawState.fDrawFace != fCurrDrawState.fDrawFace) {
        switch (fCurrDrawState.fDrawFace) {
            case GrDrawState::kCCW_DrawFace:
                GL_CALL(Enable(GR_GL_CULL_FACE));
                GL_CALL(CullFace(GR_GL_BACK));
                break;
            case GrDrawState::kCW_DrawFace:
                GL_CALL(Enable(GR_GL_CULL_FACE));
                GL_CALL(CullFace(GR_GL_FRONT));
                break;
            case GrDrawState::kBoth_DrawFace:
                GL_CALL(Disable(GR_GL_CULL_FACE));
                break;
            default:
                GrCrash("Unknown draw face.");
        }
        fHWDrawState.fDrawFace = fCurrDrawState.fDrawFace;
    }

#if GR_DEBUG
    // check for circular rendering
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
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
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
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
            if (kDesktop_GrGLBinding != this->glBinding()) {
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
            if (this->getCaps().f8BitPaletteSupport) {
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
    GrAssert(unit >= 0 && unit < GrDrawState::kNumStages);
    if (fActiveTextureUnitIdx != unit) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + unit));
        fActiveTextureUnitIdx = unit;
    }
}

void GrGpuGL::setSpareTextureUnit() {
    if (fActiveTextureUnitIdx != (GR_GL_TEXTURE0 + SPARE_TEX_UNIT)) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + SPARE_TEX_UNIT));
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
            if (fGLCaps.fRGBA8Renderbuffer) {
                *format = GR_GL_RGBA8;
                return true;
            } else {
                return false;
            }
        case kRGB_565_GrPixelConfig:
            // ES2 supports 565. ES1 supports it
            // with FBO extension desktop GL has
            // no such internal format
            GrAssert(kDesktop_GrGLBinding != this->glBinding());  
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
        GL_CALL(BindBuffer(GR_GL_ARRAY_BUFFER, vbuf->bufferID()));
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
            GL_CALL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, ibuf->bufferID()));
            fHWGeometryState.fIndexBuffer = ibuf;
        }
    }
}

int GrGpuGL::getMaxEdges() const {
    // FIXME:  This is a pessimistic estimate based on how many other things
    // want to add uniforms.  This should be centralized somewhere.
    return GR_CT_MIN(fGLCaps.fMaxFragmentUniformVectors - 8,
                     GrDrawState::kMaxEdges);
}

void GrGpuGL::GLCaps::print() const {
    for (int i = 0; i < fStencilFormats.count(); ++i) {
        GrPrintf("Stencil Format %d, stencil bits: %02d, total bits: %02d\n",
                 i,
                 fStencilFormats[i].fStencilBits,
                 fStencilFormats[i].fTotalBits);
    }

    GR_STATIC_ASSERT(0 == kNone_MSFBO);
    GR_STATIC_ASSERT(1 == kDesktopARB_MSFBO);
    GR_STATIC_ASSERT(2 == kDesktopEXT_MSFBO);
    GR_STATIC_ASSERT(3 == kAppleES_MSFBO);
    static const char* gMSFBOExtStr[] = {
        "None",
        "ARB",
        "EXT",
        "Apple",
    };
    GrPrintf("MSAA Type: %s\n", gMSFBOExtStr[fMSFBOType]);
    for (int i = 0; i < (int)GR_ARRAY_COUNT(fAASamples); ++i) {
        GrPrintf("AA Level %d has %d samples\n", i, fAASamples[i]);
    }
    GrPrintf("Max FS Uniform Vectors: %d\n", fMaxFragmentUniformVectors);
    GrPrintf("Support RGBA8 Render Buffer: %s\n",
             (fRGBA8Renderbuffer ? "YES": "NO"));
}
