
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

GrGpuGL::GrGpuGL(const GrGLInterface* gl, GrGLBinding glBinding) {

    fPrintedCaps = false;

    gl->ref();
    fGL = gl;
    fGLBinding = glBinding;
    switch (glBinding) {
        case kDesktop_GrGLBinding:
            GrAssert(gl->supportsDesktop());
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
    // This must be called by before the GrDrawTarget destructor
    this->releaseGeometry();
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
    GR_GL_GetIntegerv(fGL, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    GrAssert(maxTextureUnits > GrDrawState::kNumStages);
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
        // we could also look for GL_ATI_separate_stencil extension or
        // GL_EXT_stencil_two_side but they use different function signatures
        // than GL2.0+ (and than each other).
        fCaps.fTwoSidedStencilSupport = (fGLVersion >= GR_GL_VER(2,0));
        // supported on GL 1.4 and higher or by extension
        fCaps.fStencilWrapOpsSupport = (fGLVersion >= GR_GL_VER(1,4)) ||
                                       this->hasExtension("GL_EXT_stencil_wrap");
    } else {
        // ES 2 has two sided stencil and stencil wrap
        fCaps.fTwoSidedStencilSupport = true;
        fCaps.fStencilWrapOpsSupport = true;
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fGLCaps.fRGBA8RenderbufferSupport = true;
    } else {
        fGLCaps.fRGBA8RenderbufferSupport =
                                    this->hasExtension("GL_OES_rgb8_rgba8") ||
                                    this->hasExtension("GL_ARM_rgba8");
    }


    if (kDesktop_GrGLBinding == this->glBinding()) {
        fGLCaps.fBGRAFormatSupport = this->glVersion() >= GR_GL_VER(1,2) ||
                                     this->hasExtension("GL_EXT_bgra");
    } else {
        bool hasBGRAExt = false;
        if (this->hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            fGLCaps.fBGRAFormatSupport = true;
        } else if (this->hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fGLCaps.fBGRAFormatSupport = true;
            fGLCaps.fBGRAIsInternalFormat = true;
        }
        GrAssert(fGLCaps.fBGRAFormatSupport ||
                 kSkia8888_PM_GrPixelConfig != kBGRA_8888_PM_GrPixelConfig);
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fGLCaps.fTextureSwizzleSupport = this->glVersion() >= GR_GL_VER(3,3) ||
                                  this->hasExtension("GL_ARB_texture_swizzle");
    } else {
        fGLCaps.fTextureSwizzleSupport = false;
    }

    if (kDesktop_GrGLBinding == this->glBinding()) {
        fGLCaps.fUnpackRowLengthSupport = true;
        fGLCaps.fUnpackFlipYSupport = false;
        fGLCaps.fPackRowLengthSupport = true;
        fGLCaps.fPackFlipYSupport = false;
    } else {
        fGLCaps.fUnpackRowLengthSupport =this->hasExtension("GL_EXT_unpack_subimage");
        fGLCaps.fUnpackFlipYSupport = this->hasExtension("GL_CHROMIUM_flipy");
        // no extension for pack row length
        fGLCaps.fPackRowLengthSupport = false;
        fGLCaps.fPackFlipYSupport =
            this->hasExtension("GL_ANGLE_pack_reverse_row_order");
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
        } else {
            fCaps.fNPOTTextureTileSupport = false;
        }
    } else {
        // Unextended ES2 supports NPOT textures with clamp_to_edge and non-mip filters only
        fCaps.fNPOTTextureTileSupport = this->hasExtension("GL_OES_texture_npot");
    }

    fGLCaps.fTextureUsageSupport = (kES2_GrGLBinding == this->glBinding()) &&
                                   this->hasExtension("GL_ANGLE_texture_usage");

    // Tex storage is in desktop 4.2 and can be an extension to desktop or ES.
    fGLCaps.fTexStorageSupport = (kDesktop_GrGLBinding == this->glBinding() &&
                                  fGLVersion >= GR_GL_VER(4,2)) ||
                                 this->hasExtension("GL_ARB_texture_storage") ||
                                 this->hasExtension("GL_EXT_texture_storage");

    fCaps.fHWAALineSupport = (kDesktop_GrGLBinding == this->glBinding());

    ////////////////////////////////////////////////////////////////////////////
    // Experiments to determine limitations that can't be queried.
    // TODO: Make these a preprocess that generate some compile time constants.
    // TODO: probe once at startup, rather than once per context creation.

    GR_GL_GetIntegerv(fGL, GR_GL_MAX_TEXTURE_SIZE, &fCaps.fMaxTextureSize);
    GR_GL_GetIntegerv(fGL, GR_GL_MAX_RENDERBUFFER_SIZE, &fCaps.fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fCaps.fMaxRenderTargetSize = GrMin(fCaps.fMaxTextureSize, fCaps.fMaxRenderTargetSize);

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
        // ES2 has STENCIL_INDEX8 without extensions but requires extensions
        // for other formats.
        // ES doesn't support using the unsized format.

        fGLCaps.fStencilFormats.push_back() = gS8;
        //fStencilFormats.push_back() = gS16;
        if (this->hasExtension("GL_OES_packed_depth_stencil")) {
            fGLCaps.fStencilFormats.push_back() = gD24S8;
        }
        if (this->hasExtension("GL_OES_stencil4")) {
            fGLCaps.fStencilFormats.push_back() = gS4;
        }
    }
}

GrPixelConfig GrGpuGL::preferredReadPixelsConfig(GrPixelConfig config) const {
    if (GR_GL_RGBA_8888_PIXEL_OPS_SLOW && GrPixelConfigIsRGBA8888(config)) {
        return GrPixelConfigSwapRAndB(config);
    } else {
        return config;
    }
}

GrPixelConfig GrGpuGL::preferredWritePixelsConfig(GrPixelConfig config) const {
    if (GR_GL_RGBA_8888_PIXEL_OPS_SLOW && GrPixelConfigIsRGBA8888(config)) {
        return GrPixelConfigSwapRAndB(config);
    } else {
        return config;
    }
}

bool GrGpuGL::fullReadPixelsIsFasterThanPartial() const {
    return SkToBool(GR_GL_FULL_READPIXELS_FASTER_THAN_PARTIAL);
}

void GrGpuGL::onResetContext() {
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
    fHWDrawState.setDrawFace(GrDrawState::kBoth_DrawFace);

    GL_CALL(Disable(GR_GL_DITHER));
    if (kDesktop_GrGLBinding == this->glBinding()) {
        GL_CALL(Disable(GR_GL_LINE_SMOOTH));
        GL_CALL(Disable(GR_GL_POINT_SMOOTH));
        GL_CALL(Disable(GR_GL_MULTISAMPLE));
        fHWAAState.fMSAAEnabled = false;
        fHWAAState.fSmoothLineEnabled = false;
    }

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.resetStateFlags();

    // we only ever use lines in hairline mode
    GL_CALL(LineWidth(1));

    // invalid
    fActiveTextureUnitIdx = -1;

    // illegal values
    fHWDrawState.setBlendFunc((GrBlendCoeff)0xFF, (GrBlendCoeff)0xFF);

    fHWDrawState.setBlendConstant(0x00000000);
    GL_CALL(BlendColor(0,0,0,0));

    fHWDrawState.setColor(GrColor_ILLEGAL);

    fHWDrawState.setViewMatrix(GrMatrix::InvalidMatrix());

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        fHWDrawState.setTexture(s, NULL);
        fHWDrawState.sampler(s)->setRadial2Params(-GR_ScalarMax,
                                                  -GR_ScalarMax,
                                                  true);
        *fHWDrawState.sampler(s)->matrix() = GrMatrix::InvalidMatrix();
        fHWDrawState.sampler(s)->setConvolutionParams(0, NULL, NULL);
    }

    fHWBounds.fScissorRect.invalidate();
    fHWBounds.fScissorEnabled = false;
    GL_CALL(Disable(GR_GL_SCISSOR_TEST));
    fHWBounds.fViewportRect.invalidate();

    fHWDrawState.stencil()->invalidate();
    fHWStencilClip = false;
    fClipInStencil = false;

    fHWGeometryState.fIndexBuffer = NULL;
    fHWGeometryState.fVertexBuffer = NULL;
    
    fHWGeometryState.fArrayPtrsDirty = true;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.setRenderTarget(NULL);

    // we assume these values
    if (this->glCaps().fUnpackRowLengthSupport) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
    if (this->glCaps().fPackRowLengthSupport) {
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }
    if (this->glCaps().fUnpackFlipYSupport) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
    }
    if (this->glCaps().fPackFlipYSupport) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, GR_GL_FALSE));
    }
}

GrTexture* GrGpuGL::onCreatePlatformTexture(const GrPlatformTextureDesc& desc) {
    GrGLTexture::Desc glTexDesc;
    if (!configToGLFormats(desc.fConfig, false, NULL, NULL, NULL)) {
        return NULL;
    }

    glTexDesc.fWidth = desc.fWidth;
    glTexDesc.fHeight = desc.fHeight;
    glTexDesc.fConfig = desc.fConfig;
    glTexDesc.fTextureID = static_cast<GrGLuint>(desc.fTextureHandle);
    glTexDesc.fOwnsID = false;
    glTexDesc.fOrientation = GrGLTexture::kBottomUp_Orientation;

    GrGLTexture* texture = NULL;
    if (desc.fFlags & kRenderTarget_GrPlatformTextureFlag) {
        GrGLRenderTarget::Desc glRTDesc;
        glRTDesc.fRTFBOID = 0;
        glRTDesc.fTexFBOID = 0;
        glRTDesc.fMSColorRenderbufferID = 0;
        glRTDesc.fOwnIDs = true;
        glRTDesc.fConfig = desc.fConfig;
        glRTDesc.fSampleCnt = desc.fSampleCnt;
        if (!this->createRenderTargetObjects(glTexDesc.fWidth,
                                             glTexDesc.fHeight,
                                             glTexDesc.fTextureID,
                                             &glRTDesc)) {
            return NULL;
        }
        texture = new GrGLTexture(this, glTexDesc, glRTDesc);
    } else {
        texture = new GrGLTexture(this, glTexDesc);
    }
    if (NULL == texture) {
        return NULL;
    }
    
    this->setSpareTextureUnit();
    return texture;
}

GrRenderTarget* GrGpuGL::onCreatePlatformRenderTarget(const GrPlatformRenderTargetDesc& desc) {
    GrGLRenderTarget::Desc glDesc;
    glDesc.fConfig = desc.fConfig;
    glDesc.fRTFBOID = static_cast<GrGLuint>(desc.fRenderTargetHandle);
    glDesc.fMSColorRenderbufferID = 0;
    glDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    glDesc.fSampleCnt = desc.fSampleCnt;
    glDesc.fOwnIDs = false;
    GrGLIRect viewport;
    viewport.fLeft   = 0;
    viewport.fBottom = 0;
    viewport.fWidth  = desc.fWidth;
    viewport.fHeight = desc.fHeight;
    
    GrRenderTarget* tgt = new GrGLRenderTarget(this, glDesc, viewport);
    if (desc.fStencilBits) {
        GrGLStencilBuffer::Format format;
        format.fInternalFormat = GrGLStencilBuffer::kUnknownInternalFormat;
        format.fPacked = false;
        format.fStencilBits = desc.fStencilBits;
        format.fTotalBits = desc.fStencilBits;
        GrGLStencilBuffer* sb = new GrGLStencilBuffer(this,
                                                      0,
                                                      desc.fWidth,
                                                      desc.fHeight,
                                                      desc.fSampleCnt,
                                                      format);
        tgt->setStencilBuffer(sb);
        sb->unref();
    }
    return tgt;
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
        if (!this->configToGLFormats(desc.fConfig, false, NULL, NULL, NULL)) {
            return NULL;
        }
        texDesc.fWidth  = desc.fWidth;
        texDesc.fHeight = desc.fHeight;

        texDesc.fConfig             = desc.fConfig;
        texDesc.fOrientation        = GrGLTexture::kBottomUp_Orientation;
        texDesc.fTextureID          = desc.fPlatformTexture;
        texDesc.fOwnsID             = false;
        
        if (isRenderTarget) {
            GrTexture* tex = new GrGLTexture(this, texDesc, rtDesc);
            tex->asRenderTarget()->setStencilBuffer(sb.get());
            return tex;
        } else {
            return new GrGLTexture(this, texDesc);
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

void GrGpuGL::onWriteTexturePixels(GrTexture* texture,
                                   int left, int top, int width, int height,
                                   GrPixelConfig config, const void* buffer,
                                   size_t rowBytes) {
    if (NULL == buffer) {
        return;
    }
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);

    this->setSpareTextureUnit();
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, glTex->textureID()));
    GrGLTexture::Desc desc;
    desc.fConfig = glTex->config();
    desc.fWidth = glTex->width();
    desc.fHeight = glTex->height();
    desc.fOrientation = glTex->orientation();
    desc.fTextureID = glTex->textureID();

    this->uploadTexData(desc, false,
                        left, top, width, height, 
                        config, buffer, rowBytes);
}

namespace {
bool adjust_pixel_ops_params(int surfaceWidth,
                             int surfaceHeight,
                             size_t bpp,
                             int* left, int* top, int* width, int* height,
                             const void** data,
                             size_t* rowBytes) {
    if (!*rowBytes) {
        *rowBytes = *width * bpp;
    }

    GrIRect subRect = GrIRect::MakeXYWH(*left, *top, *width, *height);
    GrIRect bounds = GrIRect::MakeWH(surfaceWidth, surfaceHeight);

    if (!subRect.intersect(bounds)) {
        return false;
    }
    *data = reinterpret_cast<const void*>(reinterpret_cast<intptr_t>(*data) +
          (subRect.fTop - *top) * *rowBytes + (subRect.fLeft - *left) * bpp);

    *left = subRect.fLeft;
    *top = subRect.fTop;
    *width = subRect.width();
    *height = subRect.height();
    return true;
}
}

bool GrGpuGL::uploadTexData(const GrGLTexture::Desc& desc,
                            bool isNewTexture,
                            int left, int top, int width, int height,
                            GrPixelConfig dataConfig,
                            const void* data,
                            size_t rowBytes) {
    GrAssert(NULL != data || isNewTexture);

    size_t bpp = GrBytesPerPixel(dataConfig);
    if (!adjust_pixel_ops_params(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                 &width, &height, &data, &rowBytes)) {
        return false;
    }
    size_t trimRowBytes = width * bpp;

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    bool useTexStorage = isNewTexture &&
                         this->glCaps().fTexStorageSupport;
    if (useTexStorage) {
        if (kDesktop_GrGLBinding == this->glBinding()) {
            // 565 is not a sized internal format on desktop GL. So on desktop
            // with 565 we always use an unsized internal format to let the
            // system pick the best sized format to convert the 565 data to.
            // Since glTexStorage only allows sized internal formats we will
            // instead fallback to glTexImage2D.
            useTexStorage = desc.fConfig != kRGB_565_GrPixelConfig;
        } else {
            // ES doesn't allow paletted textures to be used with tex storage
            useTexStorage = desc.fConfig != kIndex_8_GrPixelConfig;
        }
    }

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum externalType;
    // glTexStorage requires sized internal formats on both desktop and ES. ES
    // glTexImage requires an unsized format.
    if (!this->configToGLFormats(dataConfig, useTexStorage, &internalFormat,
                                 &externalFormat, &externalType)) {
        return false;
    }

    if (!isNewTexture && GR_GL_PALETTE8_RGBA8 == internalFormat) {
        // paletted textures cannot be updated
        return false;
    }

    /*
     *  check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES may not let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;
    bool swFlipY = false;
    bool glFlipY = false;
    if (NULL != data) {
        if (GrGLTexture::kBottomUp_Orientation == desc.fOrientation) {
            if (this->glCaps().fUnpackFlipYSupport) {
                glFlipY = true;
            } else {
                swFlipY = true;
            }
        }
        if (this->glCaps().fUnpackRowLengthSupport && !swFlipY) {
            // can't use this for flipping, only non-neg values allowed. :(
            if (rowBytes != trimRowBytes) {
                GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
                GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                restoreGLRowLength = true;
            }
        } else {
            if (trimRowBytes != rowBytes || swFlipY) {
                // copy data into our new storage, skipping the trailing bytes
                size_t trimSize = height * trimRowBytes;
                const char* src = (const char*)data;
                if (swFlipY) {
                    src += (height - 1) * rowBytes;
                }
                char* dst = (char*)tempStorage.reset(trimSize);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, trimRowBytes);
                    if (swFlipY) {
                        src -= rowBytes;
                    } else {
                        src += rowBytes;
                    }
                    dst += trimRowBytes;
                }
                // now point data to our copied version
                data = tempStorage.get();
            }
        }
        if (glFlipY) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_TRUE));
        }
        GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, static_cast<GrGLint>(bpp)));
    }
    bool succeeded = true;
    if (isNewTexture && 
        0 == left && 0 == top &&
        desc.fWidth == width && desc.fHeight == height) {
        GrGLClearErr(this->glInterface());
        if (useTexStorage) {
            // We never resize  or change formats of textures. We don't use
            // mipmaps currently.
            GR_GL_CALL_NOERRCHECK(this->glInterface(),
                                  TexStorage2D(GR_GL_TEXTURE_2D,
                                               1, // levels
                                               internalFormat,
                                               desc.fWidth, desc.fHeight));
        } else {
            if (GR_GL_PALETTE8_RGBA8 == internalFormat) {
                GrGLsizei imageSize = desc.fWidth * desc.fHeight +
                                      kGrColorTableSize;
                GR_GL_CALL_NOERRCHECK(this->glInterface(),
                                      CompressedTexImage2D(GR_GL_TEXTURE_2D,
                                                           0, // level
                                                           internalFormat,
                                                           desc.fWidth,
                                                           desc.fHeight,
                                                           0, // border
                                                           imageSize,
                                                           data));
            } else {
                GR_GL_CALL_NOERRCHECK(this->glInterface(),
                                      TexImage2D(GR_GL_TEXTURE_2D,
                                                 0, // level
                                                 internalFormat,
                                                 desc.fWidth, desc.fHeight,
                                                 0, // border
                                                 externalFormat, externalType,
                                                 data));
            }
        }
        GrGLenum error = GR_GL_GET_ERROR(this->glInterface());
        if (error != GR_GL_NO_ERROR) {
            succeeded = false;
        } else {
            // if we have data and we used TexStorage to create the texture, we
            // now upload with TexSubImage.
            if (NULL != data && useTexStorage) {
                GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D,
                                      0, // level
                                      left, top,
                                      width, height,
                                      externalFormat, externalType,
                                      data));
            }
        }
    } else {
        if (swFlipY || glFlipY) {
            top = desc.fHeight - (top + height);
        }
        GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D,
                              0, // level
                              left, top,
                              width, height,
                              externalFormat, externalType, data));
    }

    if (restoreGLRowLength) {
        GrAssert(this->glCaps().fUnpackRowLengthSupport);
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
    if (glFlipY) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
    }
    return succeeded;
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
    if (desc->fSampleCnt > 0) {
        if (GLCaps::kNone_MSFBO == fGLCaps.fMSFBOType) {
            goto FAILED;
        }
        GL_CALL(GenFramebuffers(1, &desc->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &desc->fMSColorRenderbufferID));
        if (!desc->fRTFBOID ||
            !desc->fMSColorRenderbufferID || 
            !this->configToGLFormats(desc->fConfig,
                                     // GLES requires sized internal formats
                                     kES2_GrGLBinding == this->glBinding(),
                                     &msColorFormat, NULL, NULL)) {
            goto FAILED;
        }
    } else {
        desc->fRTFBOID = desc->fTexFBOID;
    }

    // below here we may bind the FBO
    fHWDrawState.setRenderTarget(NULL);
    if (desc->fRTFBOID != desc->fTexFBOID) {
        GrAssert(desc->fSampleCnt > 1);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER,
                               desc->fMSColorRenderbufferID));
        GrGLClearErr(this->glInterface());
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

    GrGLTexture::Desc glTexDesc;
    GrGLRenderTarget::Desc  glRTDesc;

    glTexDesc.fWidth  = desc.fWidth;
    glTexDesc.fHeight = desc.fHeight;
    glTexDesc.fConfig        = desc.fConfig;
    glTexDesc.fOwnsID        = true;

    glRTDesc.fMSColorRenderbufferID = 0;
    glRTDesc.fRTFBOID = 0;
    glRTDesc.fTexFBOID = 0;
    glRTDesc.fOwnIDs = true;
    glRTDesc.fConfig = glTexDesc.fConfig;

    bool renderTarget = 0 != (desc.fFlags & kRenderTarget_GrTextureFlagBit);

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

    if (renderTarget) {
        if (glTexDesc.fWidth > caps.fMaxRenderTargetSize ||
            glTexDesc.fHeight > caps.fMaxRenderTargetSize) {
            return return_null_texture();
        }
    }

    GL_CALL(GenTextures(1, &glTexDesc.fTextureID));
    if (renderTarget && this->glCaps().fTextureUsageSupport) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_USAGE,
                              GR_GL_FRAMEBUFFER_ATTACHMENT));
    }
    if (!glTexDesc.fTextureID) {
        return return_null_texture();
    }

    this->setSpareTextureUnit();
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, glTexDesc.fTextureID));

    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTexture::TexParams initialTexParams;
    // we only set a subset here so invalidate first
    initialTexParams.invalidate();
    initialTexParams.fFilter = GR_GL_NEAREST;
    initialTexParams.fWrapS = GR_GL_CLAMP_TO_EDGE;
    initialTexParams.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MAG_FILTER,
                          initialTexParams.fFilter));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MIN_FILTER,
                          initialTexParams.fFilter));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_WRAP_S,
                          initialTexParams.fWrapS));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_WRAP_T,
                          initialTexParams.fWrapT));
    if (!this->uploadTexData(glTexDesc, true, 0, 0,
                             glTexDesc.fWidth, glTexDesc.fHeight,
                             desc.fConfig, srcData, rowBytes)) {
        GL_CALL(DeleteTextures(1, &glTexDesc.fTextureID));
        return return_null_texture();
    }

    GrGLTexture* tex;
    if (renderTarget) {
#if GR_COLLECT_STATS
        ++fStats.fRenderTargetCreateCnt;
#endif
        if (!this->createRenderTargetObjects(glTexDesc.fWidth,
                                             glTexDesc.fHeight,
                                             glTexDesc.fTextureID,
                                             &glRTDesc)) {
            GL_CALL(DeleteTextures(1, &glTexDesc.fTextureID));
            return return_null_texture();
        }
        tex = new GrGLTexture(this, glTexDesc, glRTDesc);
    } else {
        tex = new GrGLTexture(this, glTexDesc);
    }
    tex->setCachedTexParams(initialTexParams, this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    GrPrintf("--- new texture [%d] size=(%d %d) config=%d\n",
             glTexDesc.fTextureID, desc.fWidth, desc.fHeight, desc.fConfig);
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
    GrAssert(width >= rt->width());
    GrAssert(height >= rt->height());

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
        GrGLClearErr(this->glInterface());
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

        fHWDrawState.setRenderTarget(NULL);
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
        if (GR_GL_GET_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
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
        if (GR_GL_GET_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
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
    const GrDrawState& drawState = this->getDrawState();
    const GrGLRenderTarget* rt =
        static_cast<const GrGLRenderTarget*>(drawState.getRenderTarget());

    GrAssert(NULL != rt);
    const GrGLIRect& vp = rt->getViewport();

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
    const GrDrawState& drawState = this->getDrawState();
    const GrRenderTarget* rt = drawState.getRenderTarget();
    // parent class should never let us get here with no RT
    GrAssert(NULL != rt);

    GrIRect clippedRect;
    if (NULL != rect) {
        // flushScissor expects rect to be clipped to the target.
        clippedRect = *rect;
        GrIRect rtRect = SkIRect::MakeWH(rt->width(), rt->height());
        if (clippedRect.intersect(rtRect)) {
            rect = &clippedRect;
        } else {
            return;
        }
    }
    this->flushRenderTarget(rect);
    this->flushScissor(rect);

    GrGLfloat r, g, b, a;
    static const GrGLfloat scale255 = 1.f / 255.f;
    a = GrColorUnpackA(color) * scale255;
    GrGLfloat scaleRGB = scale255;
    if (GrPixelConfigIsUnpremultiplied(rt->config())) {
        scaleRGB *= a;
    }
    r = GrColorUnpackR(color) * scaleRGB;
    g = GrColorUnpackG(color) * scaleRGB;
    b = GrColorUnpackB(color) * scaleRGB;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWDrawState.disableState(GrDrawState::kNoColorWrites_StateBit);
    GL_CALL(ClearColor(r, g, b, a));
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGpuGL::clearStencil() {
    if (NULL == this->getDrawState().getRenderTarget()) {
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
    fHWDrawState.stencil()->invalidate();
}

void GrGpuGL::clearStencilClip(const GrIRect& rect, bool insideClip) {
    const GrDrawState& drawState = this->getDrawState();
    const GrRenderTarget* rt = drawState.getRenderTarget();
    GrAssert(NULL != rt);

    // this should only be called internally when we know we have a
    // stencil buffer.
    GrAssert(NULL != rt->getStencilBuffer());
    GrGLint stencilBitCount =  rt->getStencilBuffer()->bits();
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
    fHWDrawState.stencil()->invalidate();
}

void GrGpuGL::onForceRenderTargetFlush() {
    this->flushRenderTarget(&GrIRect::EmptyIRect());
}

bool GrGpuGL::readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                        int left, int top,
                                        int width, int height,
                                        GrPixelConfig config,
                                        size_t rowBytes) const {
    // if GL can do the flip then we'll never pay for it.
    if (this->glCaps().fPackFlipYSupport) {
        return false;
    }

    // If we have to do memcpy to handle non-trim rowBytes then we
    // get the flip for free. Otherwise it costs.
    if (this->glCaps().fPackRowLengthSupport) {
        return true;
    }
    // If we have to do memcpys to handle rowBytes then y-flip is free
    // Note the rowBytes might be tight to the passed in data, but if data
    // gets clipped in x to the target the rowBytes will no longer be tight.
    if (left >= 0 && (left + width) < renderTarget->width()) {
           return 0 == rowBytes ||
                  GrBytesPerPixel(config) * width == rowBytes;
    } else {
        return false;
    }
}

bool GrGpuGL::onReadPixels(GrRenderTarget* target,
                           int left, int top,
                           int width, int height,
                           GrPixelConfig config,
                           void* buffer,
                           size_t rowBytes,
                           bool invertY) {
    GrGLenum format;
    GrGLenum type;
    if (!this->configToGLFormats(config, false, NULL, &format, &type)) {
        return false;
    }
    size_t bpp = GrBytesPerPixel(config);
    if (!adjust_pixel_ops_params(target->width(), target->height(), bpp,
                                 &left, &top, &width, &height,
                                 const_cast<const void**>(&buffer),
                                 &rowBytes)) {
        return false;
    }

    // resolve the render target if necessary
    GrGLRenderTarget* tgt = static_cast<GrGLRenderTarget*>(target);
    GrDrawState::AutoRenderTargetRestore artr;
    switch (tgt->getResolveType()) {
        case GrGLRenderTarget::kCantResolve_ResolveType:
            return false;
        case GrGLRenderTarget::kAutoResolves_ResolveType:
            artr.set(this->drawState(), target);
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
    
    size_t tightRowBytes = bpp * width;
    if (0 == rowBytes) {
        rowBytes = tightRowBytes;
    }
    size_t readDstRowBytes = tightRowBytes;
    void* readDst = buffer;
    
    // determine if GL can read using the passed rowBytes or if we need
    // a scratch buffer.
    SkAutoSMalloc<32 * sizeof(GrColor)> scratch;
    if (rowBytes != tightRowBytes) {
        if (this->glCaps().fPackRowLengthSupport) {
            GrAssert(!(rowBytes % sizeof(GrColor)));
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, rowBytes / sizeof(GrColor)));
            readDstRowBytes = rowBytes;
        } else {
            scratch.reset(tightRowBytes * height);
            readDst = scratch.get();
        }
    }
    if (!invertY && this->glCaps().fPackFlipYSupport) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 1));
    }
    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom,
                       readRect.fWidth, readRect.fHeight,
                       format, type, readDst));
    if (readDstRowBytes != tightRowBytes) {
        GrAssert(this->glCaps().fPackRowLengthSupport);
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }
    if (!invertY && this->glCaps().fPackFlipYSupport) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 0));
        invertY = true;
    }

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom. We must preserve the padding contents. Note
    // that the above readPixels did not overwrite the padding.
    if (readDst == buffer) {
        GrAssert(rowBytes == readDstRowBytes);
        if (!invertY) {
            scratch.reset(tightRowBytes);
            void* tmpRow = scratch.get();
            // flip y in-place by rows
            const int halfY = height >> 1;
            char* top = reinterpret_cast<char*>(buffer);
            char* bottom = top + (height - 1) * rowBytes;
            for (int y = 0; y < halfY; y++) {
                memcpy(tmpRow, top, tightRowBytes);
                memcpy(top, bottom, tightRowBytes);
                memcpy(bottom, tmpRow, tightRowBytes);
                top += rowBytes;
                bottom -= rowBytes;
            }
        }
    } else {
        GrAssert(readDst != buffer);        GrAssert(rowBytes != tightRowBytes);
        // copy from readDst to buffer while flipping y
        const int halfY = height >> 1;
        const char* src = reinterpret_cast<const char*>(readDst);
        char* dst = reinterpret_cast<char*>(buffer);
        if (!invertY) {
            dst += (height-1) * rowBytes;
        }
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, tightRowBytes);
            src += readDstRowBytes;
            if (invertY) {
                dst += rowBytes;
            } else {
                dst -= rowBytes;
            }
        }
    }
    return true;
}

void GrGpuGL::flushRenderTarget(const GrIRect* bound) {

    GrGLRenderTarget* rt =
        static_cast<GrGLRenderTarget*>(this->drawState()->getRenderTarget());
    GrAssert(NULL != rt);

    if (fHWDrawState.getRenderTarget() != rt) {
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
        fHWDrawState.setRenderTarget(rt);
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
        fHWDrawState.setRenderTarget(NULL);
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
    const GrDrawState& drawState = this->getDrawState();

    const GrStencilSettings* settings = &drawState.getStencil();

    // use stencil for clipping if clipping is enabled and the clip
    // has been written into the stencil.
    bool stencilClip = fClipInStencil && drawState.isClipState();
    bool drawClipToStencil =
        drawState.isStateFlagEnabled(kModifyStencilClip_StateBit);
    bool stencilChange = (fHWDrawState.getStencil() != *settings) ||
                         (fHWStencilClip != stencilClip) ||
                         (fHWDrawState.isStateFlagEnabled(kModifyStencilClip_StateBit) !=
                          drawClipToStencil);

    if (stencilChange) {

        // we can't simultaneously perform stencil-clipping and 
        // modify the stencil clip
        GrAssert(!stencilClip || !drawClipToStencil);

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
                GrAssert(settings->frontPassOp() != kIncWrap_StencilOp);
                GrAssert(settings->frontPassOp() != kDecWrap_StencilOp);
                GrAssert(settings->frontFailOp() != kIncWrap_StencilOp);
                GrAssert(settings->backFailOp() != kDecWrap_StencilOp);
                GrAssert(settings->backPassOp() != kIncWrap_StencilOp);
                GrAssert(settings->backPassOp() != kDecWrap_StencilOp);
                GrAssert(settings->backFailOp() != kIncWrap_StencilOp);
                GrAssert(settings->frontFailOp() != kDecWrap_StencilOp);
            }
    #endif
            int stencilBits = 0;
            GrStencilBuffer* stencilBuffer =
                drawState.getRenderTarget()->getStencilBuffer();
            if (NULL != stencilBuffer) {
                stencilBits = stencilBuffer->bits();
            }
            // TODO: dynamically attach a stencil buffer
            GrAssert(stencilBits || settings->isDisabled());

            GrGLuint clipStencilMask = 0;
            GrGLuint userStencilMask = ~0;
            if (stencilBits > 0) {
                clipStencilMask =  1 << (stencilBits - 1);
                userStencilMask = clipStencilMask - 1;
            }

            unsigned int frontRef  = settings->frontFuncRef();
            unsigned int frontMask = settings->frontFuncMask();
            unsigned int frontWriteMask = settings->frontWriteMask();
            GrGLenum frontFunc;

            if (drawClipToStencil) {
                GrAssert(settings->frontFunc() < kBasicStencilFuncCount);
                frontFunc = grToGLStencilFunc[settings->frontFunc()];
            } else {
                frontFunc = grToGLStencilFunc[ConvertStencilFunc(
                        stencilClip, settings->frontFunc())];

                ConvertStencilFuncAndMask(settings->frontFunc(),
                                          stencilClip,
                                          clipStencilMask,
                                          userStencilMask,
                                          &frontRef,
                                          &frontMask);
                frontWriteMask &= userStencilMask;
            }
            GrAssert((size_t)
                settings->frontFailOp() < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert((size_t)
                settings->frontPassOp() < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert((size_t)
                settings->backFailOp() < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert((size_t)
                settings->backPassOp() < GR_ARRAY_COUNT(grToGLStencilOp));
            if (this->getCaps().fTwoSidedStencilSupport) {
                GrGLenum backFunc;

                unsigned int backRef  = settings->backFuncRef();
                unsigned int backMask = settings->backFuncMask();
                unsigned int backWriteMask = settings->backWriteMask();


                if (drawClipToStencil) {
                    GrAssert(settings->backFunc() < kBasicStencilFuncCount);
                    backFunc = grToGLStencilFunc[settings->backFunc()];
                } else {
                    backFunc = grToGLStencilFunc[ConvertStencilFunc(
                        stencilClip, settings->backFunc())];
                    ConvertStencilFuncAndMask(settings->backFunc(),
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
                                    grToGLStencilOp[settings->frontFailOp()],
                                    grToGLStencilOp[settings->frontPassOp()],
                                    grToGLStencilOp[settings->frontPassOp()]));

                GL_CALL(StencilOpSeparate(GR_GL_BACK,
                                    grToGLStencilOp[settings->backFailOp()],
                                    grToGLStencilOp[settings->backPassOp()],
                                    grToGLStencilOp[settings->backPassOp()]));
            } else {
                GL_CALL(StencilFunc(frontFunc, frontRef, frontMask));
                GL_CALL(StencilMask(frontWriteMask));
                GL_CALL(StencilOp(grToGLStencilOp[settings->frontFailOp()],
                                grToGLStencilOp[settings->frontPassOp()],
                                grToGLStencilOp[settings->frontPassOp()]));
            }
        }
        *fHWDrawState.stencil() = *settings;
        fHWStencilClip = stencilClip;
    }
}

void GrGpuGL::flushAAState(GrPrimitiveType type) {
    const GrRenderTarget* rt = this->getDrawState().getRenderTarget();
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
            if (rt->isMultisampled() && 
                fHWAAState.fMSAAEnabled) {
                GL_CALL(Disable(GR_GL_MULTISAMPLE));
                fHWAAState.fMSAAEnabled = false;
            }
        } else if (rt->isMultisampled() &&
                   this->getDrawState().isHWAntialiasState() !=
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
        if (kSA_BlendCoeff != fHWDrawState.getSrcBlendCoeff() ||
            kISA_BlendCoeff != fHWDrawState.getDstBlendCoeff()) {
            GL_CALL(BlendFunc(gXfermodeCoeff2Blend[kSA_BlendCoeff],
                              gXfermodeCoeff2Blend[kISA_BlendCoeff]));
            fHWDrawState.setBlendFunc(kSA_BlendCoeff, kISA_BlendCoeff);
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
            if (fHWDrawState.getSrcBlendCoeff() != srcCoeff ||
                fHWDrawState.getDstBlendCoeff() != dstCoeff) {
                GL_CALL(BlendFunc(gXfermodeCoeff2Blend[srcCoeff],
                                  gXfermodeCoeff2Blend[dstCoeff]));
                fHWDrawState.setBlendFunc(srcCoeff, dstCoeff);
            }
            GrColor blendConst = fCurrDrawState.getBlendConstant();
            if ((BlendCoeffReferencesConstant(srcCoeff) ||
                 BlendCoeffReferencesConstant(dstCoeff)) &&
                fHWDrawState.getBlendConstant() != blendConst) {

                float c[] = {
                    GrColorUnpackR(blendConst) / 255.f,
                    GrColorUnpackG(blendConst) / 255.f,
                    GrColorUnpackB(blendConst) / 255.f,
                    GrColorUnpackA(blendConst) / 255.f
                };
                GL_CALL(BlendColor(c[0], c[1], c[2], c[3]));
                fHWDrawState.setBlendConstant(blendConst);
            }
        }
    }
}

namespace {

unsigned gr_to_gl_filter(GrSamplerState::Filter filter) {
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

const GrGLenum* get_swizzle(GrPixelConfig config,
                            const GrSamplerState& sampler) {
    if (GrPixelConfigIsAlphaOnly(config)) {
        static const GrGLenum gAlphaSmear[] = { GR_GL_ALPHA, GR_GL_ALPHA,
                                                GR_GL_ALPHA, GR_GL_ALPHA };
        return gAlphaSmear;
    } else if (sampler.swapsRAndB()) {
        static const GrGLenum gRedBlueSwap[] = { GR_GL_BLUE, GR_GL_GREEN,
                                                 GR_GL_RED,  GR_GL_ALPHA };
        return gRedBlueSwap;
    } else {
        static const GrGLenum gStraight[] = { GR_GL_RED, GR_GL_GREEN,
                                              GR_GL_BLUE,  GR_GL_ALPHA };
        return gStraight;
    }
}

void set_tex_swizzle(GrGLenum swizzle[4], const GrGLInterface* gl) {
    // should add texparameteri to interface to make 1 instead of 4 calls here
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D,
                                 GR_GL_TEXTURE_SWIZZLE_R,
                                 swizzle[0]));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D,
                                 GR_GL_TEXTURE_SWIZZLE_G,
                                 swizzle[1]));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D,
                                 GR_GL_TEXTURE_SWIZZLE_B,
                                 swizzle[2]));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D,
                                 GR_GL_TEXTURE_SWIZZLE_A,
                                 swizzle[3]));
}
}

bool GrGpuGL::flushGLStateCommon(GrPrimitiveType type) {

    GrDrawState* drawState = this->drawState();
    // GrGpu::setupClipAndFlushState should have already checked this
    // and bailed if not true.
    GrAssert(NULL != drawState->getRenderTarget());

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        // bind texture and set sampler state
        if (this->isStageEnabled(s)) {
            GrGLTexture* nextTexture = 
                static_cast<GrGLTexture*>(drawState->getTexture(s));

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

            if (fHWDrawState.getTexture(s) != nextTexture) {
                setTextureUnit(s);
                GL_CALL(BindTexture(GR_GL_TEXTURE_2D, nextTexture->textureID()));
            #if GR_COLLECT_STATS
                ++fStats.fTextureChngCnt;
            #endif
                //GrPrintf("---- bindtexture %d\n", nextTexture->textureID());
                fHWDrawState.setTexture(s, nextTexture);
                // The texture matrix has to compensate for texture width/height
                // and NPOT-embedded-in-POT
                fDirtyFlags.fTextureChangedMask |= (1 << s);
            }

            const GrSamplerState& sampler = drawState->getSampler(s);
            ResetTimestamp timestamp;
            const GrGLTexture::TexParams& oldTexParams =
                                    nextTexture->getCachedTexParams(&timestamp);
            bool setAll = timestamp < this->getResetTimestamp();
            GrGLTexture::TexParams newTexParams;

            newTexParams.fFilter = gr_to_gl_filter(sampler.getFilter());

            const GrGLenum* wraps =  GrGLTexture::WrapMode2GLWrap();
            newTexParams.fWrapS = wraps[sampler.getWrapX()];
            newTexParams.fWrapT = wraps[sampler.getWrapY()];
            memcpy(newTexParams.fSwizzleRGBA,
                   get_swizzle(nextTexture->config(), sampler),
                   sizeof(newTexParams.fSwizzleRGBA));
            if (setAll || newTexParams.fFilter != oldTexParams.fFilter) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                        GR_GL_TEXTURE_MAG_FILTER,
                                        newTexParams.fFilter));
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                        GR_GL_TEXTURE_MIN_FILTER,
                                        newTexParams.fFilter));
            }
            if (setAll || newTexParams.fWrapS != oldTexParams.fWrapS) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                        GR_GL_TEXTURE_WRAP_S,
                                        newTexParams.fWrapS));
            }
            if (setAll || newTexParams.fWrapT != oldTexParams.fWrapT) {
                setTextureUnit(s);
                GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                                        GR_GL_TEXTURE_WRAP_T,
                                        newTexParams.fWrapT));
            }
            if (this->glCaps().fTextureSwizzleSupport &&
                (setAll ||
                 memcmp(newTexParams.fSwizzleRGBA,
                        oldTexParams.fSwizzleRGBA,
                        sizeof(newTexParams.fSwizzleRGBA)))) {
                setTextureUnit(s);
                set_tex_swizzle(newTexParams.fSwizzleRGBA,
                                this->glInterface());
            }
            nextTexture->setCachedTexParams(newTexParams,
                                            this->getResetTimestamp());
        }
    }

    GrIRect* rect = NULL;
    GrIRect clipBounds;
    if (drawState->isClipState() &&
        fClip.hasConservativeBounds()) {
        fClip.getConservativeBounds().roundOut(&clipBounds);
        rect = &clipBounds;
    }
    this->flushRenderTarget(rect);
    this->flushAAState(type);
    
    if (drawState->isDitherState() != fHWDrawState.isDitherState()) {
        if (drawState->isDitherState()) {
            GL_CALL(Enable(GR_GL_DITHER));
        } else {
            GL_CALL(Disable(GR_GL_DITHER));
        }
    }

    if (drawState->isColorWriteDisabled() !=
        fHWDrawState.isColorWriteDisabled()) {
        GrGLenum mask;
        if (drawState->isColorWriteDisabled()) {
            mask = GR_GL_FALSE;
        } else {
            mask = GR_GL_TRUE;
        }
        GL_CALL(ColorMask(mask, mask, mask, mask));
    }

    if (fHWDrawState.getDrawFace() != drawState->getDrawFace()) {
        switch (fCurrDrawState.getDrawFace()) {
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
        fHWDrawState.setDrawFace(drawState->getDrawFace());
    }

#if GR_DEBUG
    // check for circular rendering
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        GrAssert(!this->isStageEnabled(s) ||
                 NULL == drawState->getRenderTarget() ||
                 NULL == drawState->getTexture(s) ||
                 drawState->getTexture(s)->asRenderTarget() !=
                    drawState->getRenderTarget());
    }
#endif

    this->flushStencil();

    // This copy must happen after flushStencil() is called. flushStencil()
    // relies on detecting when the kModifyStencilClip_StateBit state has
    // changed since the last draw.
    fHWDrawState.copyStateFlags(*drawState);
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
    GrDrawState* drawState = this->drawState();
    if (drawState->getRenderTarget() == renderTarget) {
        drawState->setRenderTarget(NULL);
    }
    if (fHWDrawState.getRenderTarget() == renderTarget) {
        fHWDrawState.setRenderTarget(NULL);
    }
}

void GrGpuGL::notifyTextureDelete(GrGLTexture* texture) {
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        GrDrawState* drawState = this->drawState();
        if (drawState->getTexture(s) == texture) {
            fCurrDrawState.setTexture(s, NULL);
        }
        if (fHWDrawState.getTexture(s) == texture) {
            // deleting bound texture does implied bind to 0
            fHWDrawState.setTexture(s, NULL);
       }
    }
}

bool GrGpuGL::configToGLFormats(GrPixelConfig config,
                                bool getSizedInternalFormat,
                                GrGLenum* internalFormat,
                                GrGLenum* externalFormat,
                                GrGLenum* externalType) {
    GrGLenum dontCare;
    if (NULL == internalFormat) {
        internalFormat = &dontCare;
    }
    if (NULL == externalFormat) {
        externalFormat = &dontCare;
    }
    if (NULL == externalType) {
        externalType = &dontCare;
    }

    switch (config) {
        case kRGBA_8888_PM_GrPixelConfig:
        case kRGBA_8888_UPM_GrPixelConfig:
            *internalFormat = GR_GL_RGBA;
            *externalFormat = GR_GL_RGBA;
            if (getSizedInternalFormat) {
                *internalFormat = GR_GL_RGBA8;
            } else {
                *internalFormat = GR_GL_RGBA;
            }
            *externalType = GR_GL_UNSIGNED_BYTE;
            break;
        case kBGRA_8888_PM_GrPixelConfig:
        case kBGRA_8888_UPM_GrPixelConfig:
            if (!fGLCaps.fBGRAFormatSupport) {
                return false;
            }
            if (fGLCaps.fBGRAIsInternalFormat) {
                if (getSizedInternalFormat) {
                    *internalFormat = GR_GL_BGRA8;
                } else {
                    *internalFormat = GR_GL_BGRA;
                }
            } else {
                if (getSizedInternalFormat) {
                    *internalFormat = GR_GL_RGBA8;
                } else {
                    *internalFormat = GR_GL_RGBA;
                }
            }
            *externalFormat = GR_GL_BGRA;
            *externalType = GR_GL_UNSIGNED_BYTE;
            break;
        case kRGB_565_GrPixelConfig:
            *internalFormat = GR_GL_RGB;
            *externalFormat = GR_GL_RGB;
            if (getSizedInternalFormat) {
                if (this->glBinding() == kDesktop_GrGLBinding) {
                    return false;
                } else {
                    *internalFormat = GR_GL_RGB565;
                }
            } else {
                *internalFormat = GR_GL_RGB;
            }
            *externalType = GR_GL_UNSIGNED_SHORT_5_6_5;
            break;
        case kRGBA_4444_GrPixelConfig:
            *internalFormat = GR_GL_RGBA;
            *externalFormat = GR_GL_RGBA;
            if (getSizedInternalFormat) {
                *internalFormat = GR_GL_RGBA4;
            } else {
                *internalFormat = GR_GL_RGBA;
            }
            *externalType = GR_GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case kIndex_8_GrPixelConfig:
            if (this->getCaps().f8BitPaletteSupport) {
                *internalFormat = GR_GL_PALETTE8_RGBA8;
                // glCompressedTexImage doesn't take external params
                *externalFormat = GR_GL_PALETTE8_RGBA8;
                // no sized/unsized internal format distinction here
                *internalFormat = GR_GL_PALETTE8_RGBA8;
                // unused with CompressedTexImage
                *externalType = GR_GL_UNSIGNED_BYTE;
            } else {
                return false;
            }
            break;
        case kAlpha_8_GrPixelConfig:
            *internalFormat = GR_GL_ALPHA;
            *externalFormat = GR_GL_ALPHA;
            if (getSizedInternalFormat) {
                *internalFormat = GR_GL_ALPHA8;
            } else {
                *internalFormat = GR_GL_ALPHA;
            }
            *externalType = GR_GL_UNSIGNED_BYTE;
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
             (fRGBA8RenderbufferSupport ? "YES": "NO"));
    GrPrintf("BGRA is an internal format: %s\n",
             (fBGRAIsInternalFormat ? "YES": "NO"));
    GrPrintf("Support texture swizzle: %s\n",
             (fTextureSwizzleSupport ? "YES": "NO"));
    GrPrintf("Unpack Row length support: %s\n",
             (fUnpackRowLengthSupport ? "YES": "NO"));
    GrPrintf("Unpack Flip Y support: %s\n",
             (fUnpackFlipYSupport ? "YES": "NO"));
    GrPrintf("Pack Row length support: %s\n",
             (fPackRowLengthSupport ? "YES": "NO"));
    GrPrintf("Pack Flip Y support: %s\n",
             (fPackFlipYSupport ? "YES": "NO"));
}
