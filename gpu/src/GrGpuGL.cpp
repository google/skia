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
#if GR_WIN32_BUILD
    // need to get wglGetProcAddress
    #undef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN 1
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN
#endif


static const GLuint GR_MAX_GLUINT = ~0;
static const GLint  GR_INVAL_GLINT = ~0;

// we use a spare texture unit to avoid
// mucking with the state of any of the stages.
static const int SPARE_TEX_UNIT = GrGpuGL::kNumStages;

#define SKIP_CACHE_CHECK    true

static const GLenum gXfermodeCoeff2Blend[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
};

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


static bool fbo_test(GrGLExts exts, int w, int h) {

    GLint savedFBO;
    GLint savedTexUnit;
    GR_GL_GetIntegerv(GL_ACTIVE_TEXTURE, &savedTexUnit);
    GR_GL_GetIntegerv(GR_FRAMEBUFFER_BINDING, &savedFBO);

    GR_GL(ActiveTexture(GL_TEXTURE0 + SPARE_TEX_UNIT));

    GLuint testFBO;
    GR_GLEXT(exts, GenFramebuffers(1, &testFBO));
    GR_GLEXT(exts, BindFramebuffer(GR_FRAMEBUFFER, testFBO));
    GLuint testRTTex;
    GR_GL(GenTextures(1, &testRTTex));
    GR_GL(BindTexture(GL_TEXTURE_2D, testRTTex));
    // some implementations require texture to be mip-map complete before
    // FBO with level 0 bound as color attachment will be framebuffer complete.
    GR_GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GR_GL(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GR_GL(BindTexture(GL_TEXTURE_2D, 0));
    GR_GLEXT(exts, FramebufferTexture2D(GR_FRAMEBUFFER, GR_COLOR_ATTACHMENT0,
                                        GL_TEXTURE_2D, testRTTex, 0));
    GLenum status = GR_GLEXT(exts, CheckFramebufferStatus(GR_FRAMEBUFFER));
    GR_GLEXT(exts, DeleteFramebuffers(1, &testFBO));
    GR_GL(DeleteTextures(1, &testRTTex));

    GR_GL(ActiveTexture(savedTexUnit));
    GR_GLEXT(exts, BindFramebuffer(GR_FRAMEBUFFER, savedFBO));

    return status == GR_FRAMEBUFFER_COMPLETE;
}

GrGpuGL::GrGpuGL() {

    if (gPrintStartupSpew) {
        GrPrintf("------------------------- create GrGpuGL %p --------------\n",
                 this);
        GrPrintf("------ VENDOR %s\n", glGetString(GL_VENDOR));
        GrPrintf("------ RENDERER %s\n", glGetString(GL_RENDERER));
        GrPrintf("------ VERSION %s\n", glGetString(GL_VERSION));
        GrPrintf("------ EXTENSIONS\n %s \n", glGetString(GL_EXTENSIONS));
    }

    GrGLClearErr();

    GrGLInitExtensions(&fExts);

    resetDirtyFlags();

    GLint maxTextureUnits;
    // check FS and fixed-function texture unit limits
    // we only use textures in the fragment stage currently.
    // checks are > to make sure we have a spare unit.
#if GR_SUPPORT_GLDESKTOP || GR_SUPPORT_GLES2
    GR_GL_GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    GrAssert(maxTextureUnits > kNumStages);
#endif
#if GR_SUPPORT_GLDESKTOP || GR_SUPPORT_GLES1
    GR_GL_GetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    GrAssert(maxTextureUnits > kNumStages);
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Check for supported features.

    int major, minor;
    gl_version(&major, &minor);

    GLint numFormats;
    GR_GL_GetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
    GrAutoSTMalloc<10, GLint> formats(numFormats);
    GR_GL_GetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
    for (int i = 0; i < numFormats; ++i) {
        if (formats[i] == GR_PALETTE8_RGBA8) {
            f8bitPaletteSupport = true;
            break;
        }
    }

    if (gPrintStartupSpew) {
        GrPrintf("Palette8 support: %s\n", (f8bitPaletteSupport ? "YES" : "NO"));
    }

    GR_STATIC_ASSERT(0 == kNone_AALevel);
    GR_STATIC_ASSERT(1 == kLow_AALevel);
    GR_STATIC_ASSERT(2 == kMed_AALevel);
    GR_STATIC_ASSERT(3 == kHigh_AALevel);

    memset(fAASamples, 0, sizeof(fAASamples));
    fMSFBOType = kNone_MSFBO;
    if (has_gl_extension("GL_IMG_multisampled_render_to_texture")) {
        fMSFBOType = kIMG_MSFBO;
        if (gPrintStartupSpew) {
            GrPrintf("MSAA Support: IMG ES EXT.\n");
        }
    }
    else if (has_gl_extension("GL_APPLE_framebuffer_multisample")) {
        fMSFBOType = kApple_MSFBO;
        if (gPrintStartupSpew) {
            GrPrintf("MSAA Support: APPLE ES EXT.\n");
        }
    }
#if GR_SUPPORT_GLDESKTOP
    else if ((major >= 3) ||
             has_gl_extension("GL_ARB_framebuffer_object") ||
             (has_gl_extension("GL_EXT_framebuffer_multisample") &&
              has_gl_extension("GL_EXT_framebuffer_blit"))) {
        fMSFBOType = kDesktop_MSFBO;
         if (gPrintStartupSpew) {
             GrPrintf("MSAA Support: DESKTOP\n");
         }
    }
#endif
    else {
        if (gPrintStartupSpew) {
            GrPrintf("MSAA Support: NONE\n");
        }
    }

    if (kNone_MSFBO != fMSFBOType) {
        GLint maxSamples;
        GLenum maxSampleGetter = (kIMG_MSFBO == fMSFBOType) ?
                                                            GR_MAX_SAMPLES_IMG :
                                                            GR_MAX_SAMPLES;
        GR_GL_GetIntegerv(maxSampleGetter, &maxSamples);
        if (maxSamples > 1 ) {
            fAASamples[kNone_AALevel] = 0;
            fAASamples[kLow_AALevel] = GrMax(2,
                                             GrFixedFloorToInt((GR_FixedHalf) *
                                                             maxSamples));
            fAASamples[kMed_AALevel] = GrMax(2,
                                             GrFixedFloorToInt(((GR_Fixed1*3)/4) *
                                                             maxSamples));
            fAASamples[kHigh_AALevel] = maxSamples;
        }
        if (gPrintStartupSpew) {
            GrPrintf("\tMax Samples: %d\n", maxSamples);
        }
    }

#if GR_SUPPORT_GLDESKTOP
    fHasStencilWrap = (major >= 2 || (major == 1 && minor >= 4)) ||
                      has_gl_extension("GL_EXT_stencil_wrap");
#else
    fHasStencilWrap = (major >= 2) || has_gl_extension("GL_OES_stencil_wrap");
#endif
    if (gPrintStartupSpew) {
        GrPrintf("Stencil Wrap: %s\n", (fHasStencilWrap ? "YES" : "NO"));
    }

#if GR_SUPPORT_GLDESKTOP
    // we could also look for GL_ATI_separate_stencil extension or
    // GL_EXT_stencil_two_side but they use different function signatures
    // than GL2.0+ (and than each other).
    fTwoSidedStencilSupport = (major >= 2);
    // supported on GL 1.4 and higher or by extension
    fStencilWrapOpsSupport = (major > 1) ||
                             ((1 == major) && (minor >= 4)) ||
                              has_gl_extension("GL_EXT_stencil_wrap");
#else
    // ES 2 has two sided stencil but 1.1 doesn't. There doesn't seem to be
    // an ES1 extension.
    fTwoSidedStencilSupport = (major >= 2);
    // stencil wrap support is in ES2, ES1 requires extension.
    fStencilWrapOpsSupport = (major > 1) ||
                              has_gl_extension("GL_OES_stencil_wrap");

#endif
    if (gPrintStartupSpew) {
        GrPrintf("Stencil Caps: TwoSide: %s, Wrap: %s\n",
                (fTwoSidedStencilSupport ? "YES" : "NO"),
                (fStencilWrapOpsSupport ? "YES" : "NO"));
    }

#if GR_SUPPORT_GLDESKTOP
    fRGBA8Renderbuffer = true;
#else
    fRGBA8Renderbuffer = has_gl_extension("GL_OES_rgb8_rgba8");
#endif
    if (gPrintStartupSpew) {
        GrPrintf("RGBA Renderbuffer: %s\n", (fRGBA8Renderbuffer ? "YES" : "NO"));
    }

#if GR_SUPPORT_GLES
    if (GR_GL_32BPP_COLOR_FORMAT == GR_BGRA) {
        GrAssert(has_gl_extension("GL_EXT_texture_format_BGRA8888"));
    }
#endif

#if GR_SUPPORT_GLDESKTOP
    fBufferLockSupport = true; // we require VBO support and the desktop VBO
                               // extension includes glMapBuffer.
#else
    fBufferLockSupport = has_gl_extension("GL_OES_mapbuffer");
#endif

    if (gPrintStartupSpew) {
        GrPrintf("Map Buffer: %s\n", (fBufferLockSupport ? "YES" : "NO"));
    }

#if GR_SUPPORT_GLDESKTOP
    if (major >= 2 || has_gl_extension("GL_ARB_texture_non_power_of_two")) {
        fNPOTTextureTileSupport = true;
        fNPOTTextureSupport = true;
    } else {
        fNPOTTextureTileSupport = false;
        fNPOTTextureSupport = false;
    }
#else
    if (major >= 2) {
        fNPOTTextureSupport = true;
        fNPOTTextureTileSupport = has_gl_extension("GL_OES_texture_npot");
    } else {
        fNPOTTextureSupport = has_gl_extension("GL_APPLE_texture_2D_limited_npot");
        fNPOTTextureTileSupport = false;
    }
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Experiments to determine limitations that can't be queried. TODO: Make
    // these a preprocess that generate some compile time constants.

    // sanity check to make sure we can at least create an FBO from a POT texture

    bool simpleFBOSuccess = fbo_test(fExts, 128, 128);
    if (gPrintStartupSpew) {
        if (!simpleFBOSuccess) {
            GrPrintf("FBO Sanity Test: FAILED\n");
        } else {
            GrPrintf("FBO Sanity Test: PASSED\n");
        }
    }
    GrAssert(simpleFBOSuccess);

    /* Experimentation has found that some GLs that support NPOT textures
       do not support FBOs with a NPOT texture. They report "unsupported" FBO
       status. I don't know how to explicitly query for this. Do an
       experiment. Note they may support NPOT with a renderbuffer but not a
       texture. Presumably, the implementation bloats the renderbuffer
       internally to the next POT.
     */
    bool fNPOTRenderTargetSupport = false;
    if (fNPOTTextureSupport) {
        fNPOTRenderTargetSupport = fbo_test(fExts, 200, 200);
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

    /* The iPhone 4 has a restriction that for an FBO with texture color
       attachment with height <= 8 then the width must be <= height. Here
       we look for such a limitation.
     */
    fMinRenderTargetHeight = GR_INVAL_GLINT;
    GLint maxRenderSize;
    GR_GL_GetIntegerv(GR_MAX_RENDERBUFFER_SIZE, &maxRenderSize);

    if (gPrintStartupSpew) {
        GrPrintf("Small height FBO texture experiments\n");
    }

    for (GLuint i = 1; i <= 256; fNPOTRenderTargetSupport ? ++i : i *= 2) {
        GLuint w = maxRenderSize;
        GLuint h = i;
        if (fbo_test(fExts, w, h)) {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: PASSED\n", w, h);
            }
            fMinRenderTargetHeight = i;
            break;
        } else {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: FAILED\n", w, h);
            }
        }
    }
    GrAssert(GR_INVAL_GLINT != fMinRenderTargetHeight);

    if (gPrintStartupSpew) {
        GrPrintf("Small width FBO texture experiments\n");
    }
    fMinRenderTargetWidth = GR_MAX_GLUINT;
    for (GLuint i = 1; i <= 256; fNPOTRenderTargetSupport ? i *= 2 : ++i) {
        GLuint w = i;
        GLuint h = maxRenderSize;
        if (fbo_test(fExts, w, h)) {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: PASSED\n", w, h);
            }
            fMinRenderTargetWidth = i;
            break;
        } else {
            if (gPrintStartupSpew) {
                GrPrintf("\t[%d, %d]: FAILED\n", w, h);
            }
        }
    }
    GrAssert(GR_INVAL_GLINT != fMinRenderTargetWidth);

    GR_GL_GetIntegerv(GL_MAX_TEXTURE_SIZE, &fMaxTextureDimension);
}

GrGpuGL::~GrGpuGL() {
}

void GrGpuGL::resetContext() {
    // We detect cases when blending is effectively off
    fHWBlendDisabled = false;
    GR_GL(Enable(GL_BLEND));

    // we don't use the zb at all
    GR_GL(Disable(GL_DEPTH_TEST));
    GR_GL(DepthMask(GL_FALSE));

    GR_GL(Disable(GL_CULL_FACE));
    GR_GL(FrontFace(GL_CCW));
    fHWDrawState.fDrawFace = kBoth_DrawFace;

    GR_GL(Disable(GL_DITHER));
#if GR_SUPPORT_GLDESKTOP
    GR_GL(Disable(GL_LINE_SMOOTH));
    GR_GL(Disable(GL_POINT_SMOOTH));
    GR_GL(Disable(GL_MULTISAMPLE));
#endif

    GR_GL(ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    fHWDrawState.fFlagBits = 0;

    // we only ever use lines in hairline mode
    GR_GL(LineWidth(1));

    // invalid
    fActiveTextureUnitIdx = -1;

    // illegal values
    fHWDrawState.fSrcBlend = (GrBlendCoeff)-1;
    fHWDrawState.fDstBlend = (GrBlendCoeff)-1;
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
    GR_GL(Disable(GL_SCISSOR_TEST));
    fHWBounds.fViewportRect.invalidate();

    fHWDrawState.fStencilSettings.invalidate();
    fHWStencilClip = false;
    fClipState.fClipIsDirty = true;

    fHWGeometryState.fIndexBuffer = NULL;
    fHWGeometryState.fVertexBuffer = NULL;
    GR_GL(BindBuffer(GL_ARRAY_BUFFER, 0));
    GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    fHWGeometryState.fArrayPtrsDirty = true;

    GR_GL(ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    fHWDrawState.fRenderTarget = NULL;
}

GrRenderTarget* GrGpuGL::createPlatformRenderTargetHelper(
                                                intptr_t platformRenderTarget,
                                                int stencilBits,
                                                int width,
                                                int height) {
    GrGLRenderTarget::GLRenderTargetIDs rtIDs;
    rtIDs.fStencilRenderbufferID = 0;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fTexFBOID              = 0;
    rtIDs.fOwnIDs                = false;
    GrGLIRect viewport;

    // viewport is in GL coords (top >= bottom)
    viewport.fLeft      = 0;
    viewport.fBottom    = 0;
    viewport.fWidth     = width;
    viewport.fHeight    = height;

    rtIDs.fRTFBOID  = (GLuint)platformRenderTarget;
    rtIDs.fTexFBOID = (GLuint)platformRenderTarget;

    return new GrGLRenderTarget(rtIDs, NULL, stencilBits, viewport, NULL, this);
}

GrRenderTarget* GrGpuGL::createRenderTargetFrom3DApiStateHelper() {

    GrGLRenderTarget::GLRenderTargetIDs rtIDs;

    GR_GL_GetIntegerv(GR_FRAMEBUFFER_BINDING, (GLint*)&rtIDs.fRTFBOID);
    rtIDs.fTexFBOID = rtIDs.fRTFBOID;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fStencilRenderbufferID = 0;

    GrGLIRect viewport;
    viewport.setFromGLViewport();
    GLuint stencilBits;
    GR_GL_GetIntegerv(GL_STENCIL_BITS, (GLint*)&stencilBits);

    rtIDs.fOwnIDs = false;

    return new GrGLRenderTarget(rtIDs, NULL, stencilBits, viewport, NULL, this);
}

///////////////////////////////////////////////////////////////////////////////

static const GLuint UNKNOWN_BITS = ~0;

// defines stencil formats from more to less preferred
static const struct {
    GLenum  fEnum;
    GLuint  fBits;
} gStencilFormats[] = {
    {GR_STENCIL_INDEX8,     8},

#if GR_SUPPORT_GLDESKTOP
    {GR_STENCIL_INDEX16,    16},
#endif

    {GR_DEPTH24_STENCIL8,   8},
    {GR_STENCIL_INDEX4,     4},

#if GR_SUPPORT_GLDESKTOP
    {GL_STENCIL_INDEX,      UNKNOWN_BITS},
    {GR_DEPTH_STENCIL,      UNKNOWN_BITS}
#endif
};

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

GrTexture* GrGpuGL::createTextureHelper(const TextureDesc& desc,
                                        const void* srcData,
                                        size_t rowBytes) {

#if GR_COLLECT_STATS
    ++fStats.fTextureCreateCnt;
#endif

    setSpareTextureUnit();

    static const GrGLTexture::TexParams DEFAULT_PARAMS = {
        GL_NEAREST,
        GL_CLAMP_TO_EDGE,
        GL_CLAMP_TO_EDGE
    };

    GrGLTexture::GLTextureDesc glDesc;
    GLenum internalFormat;

    glDesc.fContentWidth  = desc.fWidth;
    glDesc.fContentHeight = desc.fHeight;
    glDesc.fAllocWidth    = desc.fWidth;
    glDesc.fAllocHeight   = desc.fHeight;
    glDesc.fStencilBits   = 0;
    glDesc.fFormat        = desc.fFormat;

    bool renderTarget = 0 != (desc.fFlags & kRenderTarget_TextureFlag);
    if (!canBeTexture(desc.fFormat,
                      &internalFormat,
                      &glDesc.fUploadFormat,
                      &glDesc.fUploadType)) {
        return return_null_texture();
    }

    GrAssert(as_size_t(desc.fAALevel) < GR_ARRAY_COUNT(fAASamples));
    GLint samples = fAASamples[desc.fAALevel];
    if (kNone_MSFBO == fMSFBOType && desc.fAALevel != kNone_AALevel) {
        GrPrintf("AA RT requested but not supported on this platform.");
    }

    GR_GL(GenTextures(1, &glDesc.fTextureID));
    if (!glDesc.fTextureID) {
        return return_null_texture();
    }

    glDesc.fUploadByteCount = GrTexture::BytesPerPixel(desc.fFormat);

    /*
     *  check if our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL doesn't let us pass the rowBytes as
     *  a parameter to glTexImage2D
     */
#if GR_SUPPORT_GLDESKTOP
    if (srcData) {
        GR_GL(PixelStorei(GL_UNPACK_ROW_LENGTH,
                          rowBytes / glDesc.fUploadByteCount));
    }
#else
    GrAutoSMalloc<128 * 128> trimStorage;
    size_t trimRowBytes = desc.fWidth * glDesc.fUploadByteCount;
    if (srcData && (trimRowBytes < rowBytes)) {
        size_t trimSize = desc.fHeight * trimRowBytes;
        trimStorage.realloc(trimSize);
        // now copy the data into our new storage, skipping the trailing bytes
        const char* src = (const char*)srcData;
        char* dst = (char*)trimStorage.get();
        for (uint32_t y = 0; y < desc.fHeight; y++) {
            memcpy(dst, src, trimRowBytes);
            src += rowBytes;
            dst += trimRowBytes;
        }
        // now point srcData to our trimmed version
        srcData = trimStorage.get();
    }
#endif

    if (renderTarget) {
        if (!this->npotRenderTargetSupport()) {
            glDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
            glDesc.fAllocHeight = GrNextPow2(desc.fHeight);
        }

        glDesc.fAllocWidth = GrMax<int>(fMinRenderTargetWidth,
                                        glDesc.fAllocWidth);
        glDesc.fAllocHeight = GrMax<int>(fMinRenderTargetHeight,
                                         glDesc.fAllocHeight);
    } else if (!this->npotTextureSupport()) {
        glDesc.fAllocWidth  = GrNextPow2(desc.fWidth);
        glDesc.fAllocHeight = GrNextPow2(desc.fHeight);
    }

    GR_GL(BindTexture(GL_TEXTURE_2D, glDesc.fTextureID));
    GR_GL(TexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        DEFAULT_PARAMS.fFilter));
    GR_GL(TexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        DEFAULT_PARAMS.fFilter));
    GR_GL(TexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S,
                        DEFAULT_PARAMS.fWrapS));
    GR_GL(TexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T,
                        DEFAULT_PARAMS.fWrapT));

    GR_GL(PixelStorei(GL_UNPACK_ALIGNMENT, glDesc.fUploadByteCount));
    if (GrTexture::kIndex_8_PixelConfig == desc.fFormat &&
        supports8BitPalette()) {
        // ES only supports CompressedTexImage2D, not CompressedTexSubimage2D
        GrAssert(desc.fWidth == glDesc.fAllocWidth);
        GrAssert(desc.fHeight == glDesc.fAllocHeight);
        GLsizei imageSize = glDesc.fAllocWidth * glDesc.fAllocHeight +
                            kColorTableSize;
        GR_GL(CompressedTexImage2D(GL_TEXTURE_2D, 0, glDesc.fUploadFormat,
                                   glDesc.fAllocWidth, glDesc.fAllocHeight,
                                   0, imageSize, srcData));
        GrGL_RestoreResetRowLength();
    } else {
        if (NULL != srcData && (glDesc.fAllocWidth != desc.fWidth ||
                                glDesc.fAllocHeight != desc.fHeight)) {
            GR_GL(TexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                             glDesc.fAllocWidth, glDesc.fAllocHeight,
                             0, glDesc.fUploadFormat, glDesc.fUploadType, NULL));
            GR_GL(TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.fWidth,
                                desc.fHeight, glDesc.fUploadFormat,
                                glDesc.fUploadType, srcData));
            GrGL_RestoreResetRowLength();

            uint32_t extraW = glDesc.fAllocWidth  - desc.fWidth;
            uint32_t extraH = glDesc.fAllocHeight - desc.fHeight;
            uint32_t maxTexels = extraW * extraH;
            maxTexels = GrMax(extraW * desc.fHeight, maxTexels);
            maxTexels = GrMax(desc.fWidth * extraH, maxTexels);

            GrAutoSMalloc<128*128> texels(glDesc.fUploadByteCount * maxTexels);

            uint32_t rowSize = desc.fWidth * glDesc.fUploadByteCount;
            if (extraH) {
                uint8_t* lastRowStart = (uint8_t*) srcData +
                                        (desc.fHeight - 1) * rowSize;
                uint8_t* extraRowStart = (uint8_t*)texels.get();

                for (uint32_t i = 0; i < extraH; ++i) {
                    memcpy(extraRowStart, lastRowStart, rowSize);
                    extraRowStart += rowSize;
                }
                GR_GL(TexSubImage2D(GL_TEXTURE_2D, 0, 0, desc.fHeight, desc.fWidth,
                                    extraH, glDesc.fUploadFormat, glDesc.fUploadType,
                                    texels.get()));
            }
            if (extraW) {
                uint8_t* edgeTexel = (uint8_t*)srcData + rowSize - glDesc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (uint32_t j = 0; j < desc.fHeight; ++j) {
                    for (uint32_t i = 0; i < extraW; ++i) {
                        memcpy(extraTexel, edgeTexel, glDesc.fUploadByteCount);
                        extraTexel += glDesc.fUploadByteCount;
                    }
                    edgeTexel += rowSize;
                }
                GR_GL(TexSubImage2D(GL_TEXTURE_2D, 0, desc.fWidth, 0, extraW,
                                    desc.fHeight, glDesc.fUploadFormat,
                                    glDesc.fUploadType, texels.get()));
            }
            if (extraW && extraH) {
                uint8_t* cornerTexel = (uint8_t*)srcData + desc.fHeight * rowSize
                                       - glDesc.fUploadByteCount;
                uint8_t* extraTexel = (uint8_t*)texels.get();
                for (uint32_t i = 0; i < extraW*extraH; ++i) {
                    memcpy(extraTexel, cornerTexel, glDesc.fUploadByteCount);
                    extraTexel += glDesc.fUploadByteCount;
                }
                GR_GL(TexSubImage2D(GL_TEXTURE_2D, 0, desc.fWidth, desc.fHeight,
                                    extraW, extraH, glDesc.fUploadFormat,
                                    glDesc.fUploadType, texels.get()));
            }

        } else {
            GR_GL(TexImage2D(GL_TEXTURE_2D, 0, internalFormat, glDesc.fAllocWidth,
                             glDesc.fAllocHeight, 0, glDesc.fUploadFormat,
                             glDesc.fUploadType, srcData));
            GrGL_RestoreResetRowLength();
        }
    }

    glDesc.fOrientation = GrGLTexture::kTopDown_Orientation;

    GrGLRenderTarget::GLRenderTargetIDs rtIDs;
    rtIDs.fStencilRenderbufferID = 0;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fRTFBOID = 0;
    rtIDs.fTexFBOID = 0;
    rtIDs.fOwnIDs = true;
    GLenum msColorRenderbufferFormat = -1;

    if (renderTarget) {
#if GR_COLLECT_STATS
        ++fStats.fRenderTargetCreateCnt;
#endif
        bool failed = true;
        GLenum status;
        GLint err;

        // If need have both RT flag and srcData we have
        // to invert the data before uploading because FBO
        // will be rendered bottom up
        GrAssert(NULL == srcData);
        glDesc.fOrientation =  GrGLTexture::kBottomUp_Orientation;

        GR_GLEXT(fExts, GenFramebuffers(1, &rtIDs.fTexFBOID));
        GrAssert(rtIDs.fTexFBOID);

        // If we are using multisampling and any extension other than the IMG
        // one we will create two FBOs. We render to one and then resolve to
        // the texture bound to the other. The IMG extension does an implicit
        // resolve.
        if (samples > 1 && kIMG_MSFBO != fMSFBOType && kNone_MSFBO != fMSFBOType) {
            GR_GLEXT(fExts, GenFramebuffers(1, &rtIDs.fRTFBOID));
            GrAssert(0 != rtIDs.fRTFBOID);
            GR_GLEXT(fExts, GenRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
            GrAssert(0 != rtIDs.fMSColorRenderbufferID);
            if (!fboInternalFormat(desc.fFormat, &msColorRenderbufferFormat)) {
                GR_GLEXT(fExts,
                         DeleteRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
                GR_GL(DeleteTextures(1, &glDesc.fTextureID));
                GR_GLEXT(fExts, DeleteFramebuffers(1, &rtIDs.fTexFBOID));
                GR_GLEXT(fExts, DeleteFramebuffers(1, &rtIDs.fRTFBOID));
                return return_null_texture();
            }
        } else {
            rtIDs.fRTFBOID = rtIDs.fTexFBOID;
        }
        int attempts = 1;
        if (!(kNoPathRendering_TextureFlag & desc.fFlags)) {
            GR_GLEXT(fExts, GenRenderbuffers(1, &rtIDs.fStencilRenderbufferID));
            GrAssert(0 != rtIDs.fStencilRenderbufferID);
            attempts = GR_ARRAY_COUNT(gStencilFormats);
        }

        // someone suggested that some systems might require
        // unbinding the texture before we call FramebufferTexture2D
        // (seems unlikely)
        GR_GL(BindTexture(GL_TEXTURE_2D, 0));

        err = ~GL_NO_ERROR;
        for (int i = 0; i < attempts; ++i) {
            if (rtIDs.fStencilRenderbufferID) {
                GR_GLEXT(fExts, BindRenderbuffer(GR_RENDERBUFFER,
                                                 rtIDs.fStencilRenderbufferID));
                if (samples > 1) {
                    GR_GLEXT_NO_ERR(fExts, RenderbufferStorageMultisample(
                                                GR_RENDERBUFFER,
                                                samples,
                                                gStencilFormats[i].fEnum,
                                                glDesc.fAllocWidth,
                                                glDesc.fAllocHeight));
                } else {
                    GR_GLEXT_NO_ERR(fExts, RenderbufferStorage(
                                                GR_RENDERBUFFER,
                                                gStencilFormats[i].fEnum,
                                                glDesc.fAllocWidth,
                                                glDesc.fAllocHeight));
                }
                err = glGetError();
                if (err != GL_NO_ERROR) {
                    continue;
                }
            }
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GrAssert(samples > 1);
                GR_GLEXT(fExts, BindRenderbuffer(GR_RENDERBUFFER,
                                                 rtIDs.fMSColorRenderbufferID));
                GR_GLEXT_NO_ERR(fExts, RenderbufferStorageMultisample(
                                                   GR_RENDERBUFFER,
                                                   samples,
                                                   msColorRenderbufferFormat,
                                                   glDesc.fAllocWidth,
                                                   glDesc.fAllocHeight));
                err = glGetError();
                if (err != GL_NO_ERROR) {
                    continue;
                }
            }
            GR_GLEXT(fExts, BindFramebuffer(GR_FRAMEBUFFER, rtIDs.fTexFBOID));

#if GR_COLLECT_STATS
            ++fStats.fRenderTargetChngCnt;
#endif
            if (kIMG_MSFBO == fMSFBOType && samples > 1) {
                GR_GLEXT(fExts, FramebufferTexture2DMultisample(
                                                         GR_FRAMEBUFFER,
                                                         GR_COLOR_ATTACHMENT0,
                                                         GL_TEXTURE_2D,
                                                         glDesc.fTextureID,
                                                         0,
                                                         samples));

            } else {
                GR_GLEXT(fExts, FramebufferTexture2D(GR_FRAMEBUFFER,
                                                     GR_COLOR_ATTACHMENT0,
                                                     GL_TEXTURE_2D,
                                                     glDesc.fTextureID, 0));
            }
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GLenum status = GR_GLEXT(fExts,
                                         CheckFramebufferStatus(GR_FRAMEBUFFER));
                if (status != GR_FRAMEBUFFER_COMPLETE) {
                    GrPrintf("-- glCheckFramebufferStatus %x %d %d\n",
                             status, desc.fWidth, desc.fHeight);
                    continue;
                }
                GR_GLEXT(fExts, BindFramebuffer(GR_FRAMEBUFFER, rtIDs.fRTFBOID));
            #if GR_COLLECT_STATS
                ++fStats.fRenderTargetChngCnt;
            #endif
                GR_GLEXT(fExts, FramebufferRenderbuffer(GR_FRAMEBUFFER,
                                                 GR_COLOR_ATTACHMENT0,
                                                 GR_RENDERBUFFER,
                                                 rtIDs.fMSColorRenderbufferID));

            }
            if (rtIDs.fStencilRenderbufferID) {
                // bind the stencil to rt fbo if present, othewise the tex fbo
                GR_GLEXT(fExts, FramebufferRenderbuffer(GR_FRAMEBUFFER,
                                                 GR_STENCIL_ATTACHMENT,
                                                 GR_RENDERBUFFER,
                                                 rtIDs.fStencilRenderbufferID));
            }
            status = GR_GLEXT(fExts, CheckFramebufferStatus(GR_FRAMEBUFFER));

#if GR_SUPPORT_GLDESKTOP
            // On some implementations you have to be bound as DEPTH_STENCIL.
            // (Even binding to DEPTH and STENCIL separately with the same
            // buffer doesn't work.)
            if (rtIDs.fStencilRenderbufferID &&
                status != GR_FRAMEBUFFER_COMPLETE) {
                GR_GLEXT(fExts, FramebufferRenderbuffer(GR_FRAMEBUFFER,
                                                        GR_STENCIL_ATTACHMENT,
                                                        GR_RENDERBUFFER,
                                                        0));
                GR_GLEXT(fExts,
                         FramebufferRenderbuffer(GR_FRAMEBUFFER,
                                                 GR_DEPTH_STENCIL_ATTACHMENT,
                                                 GR_RENDERBUFFER,
                                                 rtIDs.fStencilRenderbufferID));
                status = GR_GLEXT(fExts, CheckFramebufferStatus(GR_FRAMEBUFFER));
            }
#endif
            if (status != GR_FRAMEBUFFER_COMPLETE) {
                GrPrintf("-- glCheckFramebufferStatus %x %d %d\n",
                         status, desc.fWidth, desc.fHeight);
#if GR_SUPPORT_GLDESKTOP
                if (rtIDs.fStencilRenderbufferID) {
                    GR_GLEXT(fExts, FramebufferRenderbuffer(GR_FRAMEBUFFER,
                                                     GR_DEPTH_STENCIL_ATTACHMENT,
                                                     GR_RENDERBUFFER,
                                                     0));
                }
#endif
                continue;
            }
            // we're successful!
            failed = false;
            if (rtIDs.fStencilRenderbufferID) {
                if (UNKNOWN_BITS == gStencilFormats[i].fBits) {
                    GR_GL_GetIntegerv(GL_STENCIL_BITS, (GLint*)&glDesc.fStencilBits);
                } else {
                    glDesc.fStencilBits = gStencilFormats[i].fBits;
                }
            }
            break;
        }
        if (failed) {
            if (rtIDs.fStencilRenderbufferID) {
                GR_GLEXT(fExts,
                         DeleteRenderbuffers(1, &rtIDs.fStencilRenderbufferID));
            }
            if (rtIDs.fMSColorRenderbufferID) {
                GR_GLEXT(fExts,
                         DeleteRenderbuffers(1, &rtIDs.fMSColorRenderbufferID));
            }
            if (rtIDs.fRTFBOID != rtIDs.fTexFBOID) {
                GR_GLEXT(fExts, DeleteFramebuffers(1, &rtIDs.fRTFBOID));
            }
            if (rtIDs.fTexFBOID) {
                GR_GLEXT(fExts, DeleteFramebuffers(1, &rtIDs.fTexFBOID));
            }
            GR_GL(DeleteTextures(1, &glDesc.fTextureID));
            return return_null_texture();
        }
    }
#ifdef TRACE_TEXTURE_CREATION
    GrPrintf("--- new texture [%d] size=(%d %d) bpp=%d\n",
             tex->fTextureID, width, height, tex->fUploadByteCount);
#endif
    GrGLTexture* tex = new GrGLTexture(glDesc, rtIDs, DEFAULT_PARAMS, this);

    if (0 != rtIDs.fTexFBOID) {
        GrRenderTarget* rt = tex->asRenderTarget();
        // We've messed with FBO state but may not have set the correct viewport
        // so just dirty the rendertarget state to force a resend.
        fHWDrawState.fRenderTarget = NULL;

        // clear the new stencil buffer if we have one
        if (!(desc.fFlags & kNoPathRendering_TextureFlag)) {
            GrRenderTarget* rtSave = fCurrDrawState.fRenderTarget;
            fCurrDrawState.fRenderTarget = rt;
            eraseStencil(0, ~0);
            fCurrDrawState.fRenderTarget = rtSave;
        }
    }
    return tex;
}

GrVertexBuffer* GrGpuGL::createVertexBufferHelper(uint32_t size, bool dynamic) {
    GLuint id;
    GR_GL(GenBuffers(1, &id));
    if (id) {
        GR_GL(BindBuffer(GL_ARRAY_BUFFER, id));
        fHWGeometryState.fArrayPtrsDirty = true;
        GrGLClearErr();
        // make sure driver can allocate memory for this buffer
        GR_GL_NO_ERR(BufferData(GL_ARRAY_BUFFER, size, NULL,
                                dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        if (glGetError() != GL_NO_ERROR) {
            GR_GL(DeleteBuffers(1, &id));
            // deleting bound buffer does implicit bind to 0
            fHWGeometryState.fVertexBuffer = NULL;
            return NULL;
        }
        GrGLVertexBuffer* vertexBuffer = new GrGLVertexBuffer(id, this,
                                                              size, dynamic);
        fHWGeometryState.fVertexBuffer = vertexBuffer;
        return vertexBuffer;
    }
    return NULL;
}

GrIndexBuffer* GrGpuGL::createIndexBufferHelper(uint32_t size, bool dynamic) {
    GLuint id;
    GR_GL(GenBuffers(1, &id));
    if (id) {
        GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
        GrGLClearErr();
        // make sure driver can allocate memory for this buffer
        GR_GL_NO_ERR(BufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL,
                                dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        if (glGetError() != GL_NO_ERROR) {
            GR_GL(DeleteBuffers(1, &id));
            // deleting bound buffer does implicit bind to 0
            fHWGeometryState.fIndexBuffer = NULL;
            return NULL;
        }
        GrIndexBuffer* indexBuffer = new GrGLIndexBuffer(id, this,
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
            GR_GL(Enable(GL_SCISSOR_TEST));
            fHWBounds.fScissorEnabled = true;
        }
    } else {
        if (fHWBounds.fScissorEnabled) {
            GR_GL(Disable(GL_SCISSOR_TEST));
            fHWBounds.fScissorEnabled = false;
        }
    }
}

void GrGpuGL::eraseColorHelper(GrColor color) {
    if (NULL == fCurrDrawState.fRenderTarget) {
        return;
    }
    flushRenderTarget();
    if (fHWBounds.fScissorEnabled) {
        GR_GL(Disable(GL_SCISSOR_TEST));
        fHWBounds.fScissorEnabled = false;
    }
    GR_GL(ColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE));
    fHWDrawState.fFlagBits &= ~kNoColorWrites_StateBit;
    GR_GL(ClearColor(GrColorUnpackR(color)/255.f,
                     GrColorUnpackG(color)/255.f,
                     GrColorUnpackB(color)/255.f,
                     GrColorUnpackA(color)/255.f));
    GR_GL(Clear(GL_COLOR_BUFFER_BIT));
}

void GrGpuGL::eraseStencil(uint32_t value, uint32_t mask) {
    if (NULL == fCurrDrawState.fRenderTarget) {
        return;
    }
    flushRenderTarget();
    if (fHWBounds.fScissorEnabled) {
        GR_GL(Disable(GL_SCISSOR_TEST));
        fHWBounds.fScissorEnabled = false;
    }
    GR_GL(StencilMask(mask));
    GR_GL(ClearStencil(value));
    GR_GL(Clear(GL_STENCIL_BUFFER_BIT));
    fHWDrawState.fStencilSettings.invalidate();
}

void GrGpuGL::eraseStencilClip(const GrIRect& rect) {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
#if 0
    GLint stencilBitCount = fCurrDrawState.fRenderTarget->stencilBits();
    GrAssert(stencilBitCount > 0);
    GLint clipStencilMask  = (1 << (stencilBitCount - 1));
#else
    // we could just clear the clip bit but when we go through
    // angle a partial stencil mask will cause clears to be
    // turned into draws. Our contract on GrDrawTarget says that
    // changing the clip between stencil passes may or may not
    // zero the client's clip bits. So we just clear the whole thing.
    static const GLint clipStencilMask  = ~0;
#endif
    flushRenderTarget();
    flushScissor(&rect);
    GR_GL(StencilMask(clipStencilMask));
    GR_GL(ClearStencil(0));
    GR_GL(Clear(GL_STENCIL_BUFFER_BIT));
    fHWDrawState.fStencilSettings.invalidate();
}

void GrGpuGL::forceRenderTargetFlushHelper() {
    flushRenderTarget();
}

bool GrGpuGL::readPixelsHelper(int left, int top, int width, int height,
                               GrTexture::PixelConfig config, void* buffer) {
    GLenum internalFormat;  // we don't use this for glReadPixels
    GLenum format;
    GLenum type;
    if (!this->canBeTexture(config, &internalFormat, &format, &type)) {
        return false;
    }

    if (NULL == fCurrDrawState.fRenderTarget) {
        return false;
    }
    flushRenderTarget();

    const GrGLIRect& glvp = ((GrGLRenderTarget*)fCurrDrawState.fRenderTarget)->getViewport();

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height);
    GR_GL(ReadPixels(readRect.fLeft, readRect.fBottom,
                     readRect.fWidth, readRect.fHeight,
                     format, type, buffer));

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom
    {
        size_t stride = width * GrTexture::BytesPerPixel(config);
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

void GrGpuGL::flushRenderTarget() {

    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    if (fHWDrawState.fRenderTarget != fCurrDrawState.fRenderTarget) {
        GrGLRenderTarget* rt = (GrGLRenderTarget*)fCurrDrawState.fRenderTarget;
        GR_GLEXT(fExts, BindFramebuffer(GR_FRAMEBUFFER, rt->renderFBOID()));
    #if GR_COLLECT_STATS
        ++fStats.fRenderTargetChngCnt;
    #endif
        rt->setDirty(true);
    #if GR_DEBUG
        GLenum status = GR_GLEXT(fExts, CheckFramebufferStatus(GR_FRAMEBUFFER));
        if (status != GR_FRAMEBUFFER_COMPLETE) {
            GrPrintf("-- glCheckFramebufferStatus %x\n", status);
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
}

GLenum gPrimitiveType2GLMode[] = {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN,
    GL_POINTS,
    GL_LINES,
    GL_LINE_STRIP
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

void GrGpuGL::drawIndexedHelper(GrPrimitiveType type,
                                uint32_t startVertex,
                                uint32_t startIndex,
                                uint32_t vertexCount,
                                uint32_t indexCount) {
    GrAssert((size_t)type < GR_ARRAY_COUNT(gPrimitiveType2GLMode));

    GLvoid* indices = (GLvoid*)(sizeof(uint16_t) * startIndex);

    GrAssert(NULL != fHWGeometryState.fIndexBuffer);
    GrAssert(NULL != fHWGeometryState.fVertexBuffer);

    // our setupGeometry better have adjusted this to zero since
    // DrawElements always draws from the begining of the arrays for idx 0.
    GrAssert(0 == startVertex);

    GR_GL(DrawElements(gPrimitiveType2GLMode[type], indexCount,
                       GL_UNSIGNED_SHORT, indices));
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

void GrGpuGL::drawNonIndexedHelper(GrPrimitiveType type,
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

void GrGpuGL::resolveTextureRenderTarget(GrGLTexture* texture) {
    GrGLRenderTarget* rt = (GrGLRenderTarget*) texture->asRenderTarget();

    if (NULL != rt && rt->needsResolve()) {
        GrAssert(kNone_MSFBO != fMSFBOType);
        GrAssert(rt->textureFBOID() != rt->renderFBOID());
        GR_GLEXT(fExts, BindFramebuffer(GR_READ_FRAMEBUFFER,
                                        rt->renderFBOID()));
        GR_GLEXT(fExts, BindFramebuffer(GR_DRAW_FRAMEBUFFER,
                                        rt->textureFBOID()));
    #if GR_COLLECT_STATS
        ++fStats.fRenderTargetChngCnt;
    #endif
        // make sure we go through set render target
        fHWDrawState.fRenderTarget = NULL;

        GLint left = 0;
        GLint right = texture->width();
        // we will have rendered to the top of the FBO.
        GLint top = texture->allocHeight();
        GLint bottom = texture->allocHeight() - texture->height();
        if (kApple_MSFBO == fMSFBOType) {
            GR_GL(Enable(GL_SCISSOR_TEST));
            GR_GL(Scissor(left, bottom, right-left, top-bottom));
            GR_GLEXT(fExts, ResolveMultisampleFramebuffer());
            fHWBounds.fScissorRect.invalidate();
            fHWBounds.fScissorEnabled = true;
        } else {
            GR_GLEXT(fExts, BlitFramebuffer(left, bottom, right, top,
                                     left, bottom, right, top,
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST));
        }
        rt->setDirty(false);

    }
}

static const GLenum grToGLStencilFunc[] = {
    GL_ALWAYS,           // kAlways_StencilFunc
    GL_NEVER,            // kNever_StencilFunc
    GL_GREATER,          // kGreater_StencilFunc
    GL_GEQUAL,           // kGEqual_StencilFunc
    GL_LESS,             // kLess_StencilFunc
    GL_LEQUAL,           // kLEqual_StencilFunc,
    GL_EQUAL,            // kEqual_StencilFunc,
    GL_NOTEQUAL,         // kNotEqual_StencilFunc,
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

static const GLenum grToGLStencilOp[] = {
    GL_KEEP,        // kKeep_StencilOp
    GL_REPLACE,     // kReplace_StencilOp
    GL_INCR_WRAP,   // kIncWrap_StencilOp
    GL_INCR,        // kIncClamp_StencilOp
    GL_DECR_WRAP,   // kDecWrap_StencilOp
    GL_DECR,        // kDecClamp_StencilOp
    GL_ZERO,        // kZero_StencilOp
    GL_INVERT,      // kInvert_StencilOp
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
            GR_GL(Disable(GL_STENCIL_TEST));
        } else {
            GR_GL(Enable(GL_STENCIL_TEST));
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
            GLuint clipStencilMask = 1 << (stencilBits - 1);
            GLuint userStencilMask = clipStencilMask - 1;

            unsigned int frontRef  = settings->fFrontFuncRef;
            unsigned int frontMask = settings->fFrontFuncMask;
            unsigned int frontWriteMask = settings->fFrontWriteMask;
            GLenum frontFunc;

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
                     settings->fFrontFailOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fFrontPassOp >= 0 &&
                     settings->fFrontPassOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fBackFailOp >= 0 &&
                     settings->fBackFailOp < GR_ARRAY_COUNT(grToGLStencilOp));
            GrAssert(settings->fBackPassOp >= 0 &&
                     settings->fBackPassOp < GR_ARRAY_COUNT(grToGLStencilOp));
            if (fTwoSidedStencilSupport) {
                GLenum backFunc;

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

                GR_GL(StencilFuncSeparate(GL_FRONT, frontFunc, frontRef, frontMask));
                GR_GL(StencilMaskSeparate(GL_FRONT, frontWriteMask));
                GR_GL(StencilFuncSeparate(GL_BACK, backFunc, backRef, backMask));
                GR_GL(StencilMaskSeparate(GL_BACK, backWriteMask));
                GR_GL(StencilOpSeparate(GL_FRONT, grToGLStencilOp[settings->fFrontFailOp],
                                                  grToGLStencilOp[settings->fFrontPassOp],
                                                  grToGLStencilOp[settings->fFrontPassOp]));

                GR_GL(StencilOpSeparate(GL_BACK,  grToGLStencilOp[settings->fBackFailOp],
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

bool GrGpuGL::flushGLStateCommon(GrPrimitiveType type) {

    // GrGpu::setupClipAndFlushState should have already checked this
    // and bailed if not true.
    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    for (int s = 0; s < kNumStages; ++s) {
        bool usingTexture = VertexUsesStage(s, fGeometrySrc.fVertexLayout);

        // bind texture and set sampler state
        if (usingTexture) {
            GrGLTexture* nextTexture = (GrGLTexture*)fCurrDrawState.fTextures[s];

            if (NULL != nextTexture) {
                // if we created a rt/tex and rendered to it without using a
                // texture and now we're texuring from the rt it will still be
                // the last bound texture, but it needs resolving. So keep this
                // out of the "last != next" check.
                resolveTextureRenderTarget(nextTexture);

                if (fHWDrawState.fTextures[s] != nextTexture) {
                    setTextureUnit(s);
                    GR_GL(BindTexture(GL_TEXTURE_2D, nextTexture->textureID()));
                #if GR_COLLECT_STATS
                    ++fStats.fTextureChngCnt;
                #endif
                    //GrPrintf("---- bindtexture %d\n", nextTexture->textureID());
                    fHWDrawState.fTextures[s] = nextTexture;
                }

                const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
                const GrGLTexture::TexParams& oldTexParams =
                                                    nextTexture->getTexParams();
                GrGLTexture::TexParams newTexParams;

                newTexParams.fFilter = sampler.isFilter() ? GL_LINEAR :
                                                            GL_NEAREST;
                newTexParams.fWrapS =
                            GrGLTexture::gWrapMode2GLWrap[sampler.getWrapX()];
                newTexParams.fWrapT =
                            GrGLTexture::gWrapMode2GLWrap[sampler.getWrapY()];

                if (newTexParams.fFilter != oldTexParams.fFilter) {
                    setTextureUnit(s);
                    GR_GL(TexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER,
                                        newTexParams.fFilter));
                    GR_GL(TexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MIN_FILTER,
                                        newTexParams.fFilter));
                }
                if (newTexParams.fWrapS != oldTexParams.fWrapS) {
                    setTextureUnit(s);
                    GR_GL(TexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_WRAP_S,
                                        newTexParams.fWrapS));
                }
                if (newTexParams.fWrapT != oldTexParams.fWrapT) {
                    setTextureUnit(s);
                    GR_GL(TexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_WRAP_T,
                                        newTexParams.fWrapT));
                }
                nextTexture->setTexParams(newTexParams);

                // The texture matrix has to compensate for texture width/height
                // and NPOT-embedded-in-POT
                fDirtyFlags.fTextureChangedMask |= (1 << s);
            } else {
                GrAssert(!"Rendering with texture vert flag set but no texture");
                return false;
            }
        }
    }

    flushRenderTarget();

    if ((fCurrDrawState.fFlagBits & kDither_StateBit) !=
        (fHWDrawState.fFlagBits & kDither_StateBit)) {
        if (fCurrDrawState.fFlagBits & kDither_StateBit) {
            GR_GL(Enable(GL_DITHER));
        } else {
            GR_GL(Disable(GL_DITHER));
        }
    }

    if ((fCurrDrawState.fFlagBits & kNoColorWrites_StateBit) !=
        (fHWDrawState.fFlagBits & kNoColorWrites_StateBit)) {
        GLenum mask;
        if (fCurrDrawState.fFlagBits & kNoColorWrites_StateBit) {
            mask = GL_FALSE;
        } else {
            mask = GL_TRUE;
        }
        GR_GL(ColorMask(mask, mask, mask, mask));
    }

#if GR_SUPPORT_GLDESKTOP
    // ES doesn't support toggling GL_MULTISAMPLE and doesn't have
    // smooth lines.
    if (fDirtyFlags.fRenderTargetChanged ||
        (fCurrDrawState.fFlagBits & kAntialias_StateBit) !=
        (fHWDrawState.fFlagBits & kAntialias_StateBit)) {
        GLint msaa = 0;
        // only perform query if we know MSAA is supported.
        // calling on non-MSAA target caused a crash in one environment,
        // though I don't think it should.
        if (fAASamples[kHigh_AALevel]) {
            GR_GL_GetIntegerv(GL_SAMPLE_BUFFERS, &msaa);
        }
        if (fCurrDrawState.fFlagBits & kAntialias_StateBit) {
            if (msaa) {
                GR_GL(Enable(GL_MULTISAMPLE));
            } else {
                GR_GL(Enable(GL_LINE_SMOOTH));
            }
        } else {
            if (msaa) {
                GR_GL(Disable(GL_MULTISAMPLE));
            }
            GR_GL(Disable(GL_LINE_SMOOTH));
        }
    }
#endif

    bool blendOff = canDisableBlend();
    if (fHWBlendDisabled != blendOff) {
        if (blendOff) {
            GR_GL(Disable(GL_BLEND));
        } else {
            GR_GL(Enable(GL_BLEND));
        }
        fHWBlendDisabled = blendOff;
    }

    if (!blendOff) {
        if (fHWDrawState.fSrcBlend != fCurrDrawState.fSrcBlend ||
              fHWDrawState.fDstBlend != fCurrDrawState.fDstBlend) {
            GR_GL(BlendFunc(gXfermodeCoeff2Blend[fCurrDrawState.fSrcBlend],
                            gXfermodeCoeff2Blend[fCurrDrawState.fDstBlend]));
            fHWDrawState.fSrcBlend = fCurrDrawState.fSrcBlend;
            fHWDrawState.fDstBlend = fCurrDrawState.fDstBlend;
        }
    }

    if (fHWDrawState.fDrawFace != fCurrDrawState.fDrawFace) {
        switch (fCurrDrawState.fDrawFace) {
            case kCCW_DrawFace:
                glEnable(GL_CULL_FACE);
                GR_GL(CullFace(GL_BACK));
                break;
            case kCW_DrawFace:
                GR_GL(Enable(GL_CULL_FACE));
                GR_GL(CullFace(GL_FRONT));
                break;
            case kBoth_DrawFace:
                GR_GL(Disable(GL_CULL_FACE));
                break;
            default:
                GrCrash("Unknown draw face.");
        }
        fHWDrawState.fDrawFace = fCurrDrawState.fDrawFace;
    }

#if GR_DEBUG
    // check for circular rendering
    for (int s = 0; s < kNumStages; ++s) {
        GrAssert(!VertexUsesStage(s, fGeometrySrc.fVertexLayout) ||
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
    GrAssert(!(kBuffer_GeometrySrcType == fGeometrySrc.fVertexSrc &&
               buffer == fGeometrySrc.fVertexBuffer));

    if (fHWGeometryState.fVertexBuffer == buffer) {
        // deleting bound buffer does implied bind to 0
        fHWGeometryState.fVertexBuffer = NULL;
        fHWGeometryState.fArrayPtrsDirty = true;
    }
}

void GrGpuGL::notifyIndexBufferBind(const GrGLIndexBuffer* buffer) {
    fGeometrySrc.fIndexBuffer = buffer;
}

void GrGpuGL::notifyIndexBufferDelete(const GrGLIndexBuffer* buffer) {
    GrAssert(!(kBuffer_GeometrySrcType == fGeometrySrc.fIndexSrc &&
               buffer == fGeometrySrc.fIndexBuffer));

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

bool GrGpuGL::canBeTexture(GrTexture::PixelConfig config,
                           GLenum* internalFormat,
                           GLenum* format,
                           GLenum* type) {
    switch (config) {
        case GrTexture::kRGBA_8888_PixelConfig:
        case GrTexture::kRGBX_8888_PixelConfig: // todo: can we tell it our X?
            *format = GR_GL_32BPP_COLOR_FORMAT;
#if GR_SUPPORT_GLES
            // according to GL_EXT_texture_format_BGRA8888 the *internal*
            // format for a BGRA is BGRA not RGBA (as on desktop)
            *internalFormat = GR_GL_32BPP_COLOR_FORMAT;
#else
            *internalFormat = GL_RGBA;
#endif
            *type = GL_UNSIGNED_BYTE;
            break;
        case GrTexture::kRGB_565_PixelConfig:
            *format = GL_RGB;
            *internalFormat = GL_RGB;
            *type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case GrTexture::kRGBA_4444_PixelConfig:
            *format = GL_RGBA;
            *internalFormat = GL_RGBA;
            *type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case GrTexture::kIndex_8_PixelConfig:
            if (this->supports8BitPalette()) {
                *format = GR_PALETTE8_RGBA8;
                *internalFormat = GR_PALETTE8_RGBA8;
                *type = GL_UNSIGNED_BYTE;   // unused I think
            } else {
                return false;
            }
            break;
        case GrTexture::kAlpha_8_PixelConfig:
            *format = GL_ALPHA;
            *internalFormat = GL_ALPHA;
            *type = GL_UNSIGNED_BYTE;
            break;
        default:
            return false;
    }
    return true;
}

void GrGpuGL::setTextureUnit(int unit) {
    GrAssert(unit >= 0 && unit < kNumStages);
    if (fActiveTextureUnitIdx != unit) {
        GR_GL(ActiveTexture(GL_TEXTURE0 + unit));
        fActiveTextureUnitIdx = unit;
    }
}

void GrGpuGL::setSpareTextureUnit() {
    if (fActiveTextureUnitIdx != (GL_TEXTURE0 + SPARE_TEX_UNIT)) {
        GR_GL(ActiveTexture(GL_TEXTURE0 + SPARE_TEX_UNIT));
        fActiveTextureUnitIdx = SPARE_TEX_UNIT;
    }
}

/* On ES the internalFormat and format must match for TexImage and we use
   GL_RGB, GL_RGBA for color formats. We also generally like having the driver
   decide the internalFormat. However, on ES internalFormat for
   RenderBufferStorage* has to be a specific format (not a base format like
   GL_RGBA).
 */
bool GrGpuGL::fboInternalFormat(GrTexture::PixelConfig config, GLenum* format) {
    switch (config) {
        case GrTexture::kRGBA_8888_PixelConfig:
        case GrTexture::kRGBX_8888_PixelConfig:
            if (fRGBA8Renderbuffer) {
                *format = GR_RGBA8;
                return true;
            } else {
                return false;
            }
#if GR_SUPPORT_GLES // ES2 supports 565. ES1 supports it with FBO extension
                    // desktop GL has no such internal format
        case GrTexture::kRGB_565_PixelConfig:
            *format = GR_RGB565;
            return true;
#endif
        case GrTexture::kRGBA_4444_PixelConfig:
            *format = GL_RGBA4;
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

    GrGLVertexBuffer* vbuf;
    switch (fGeometrySrc.fVertexSrc) {
    case kBuffer_GeometrySrcType:
        *extraVertexOffset = 0;
        vbuf = (GrGLVertexBuffer*) fGeometrySrc.fVertexBuffer;
        break;
    case kArray_GeometrySrcType:
    case kReserved_GeometrySrcType:
        finalizeReservedVertices();
        *extraVertexOffset = fCurrPoolStartVertex;
        vbuf = (GrGLVertexBuffer*) fCurrPoolVertexBuffer;
        break;
    default:
        vbuf = NULL; // suppress warning
        GrCrash("Unknown geometry src type!");
    }

    GrAssert(NULL != vbuf);
    GrAssert(!vbuf->isLocked());
    if (fHWGeometryState.fVertexBuffer != vbuf) {
        GR_GL(BindBuffer(GL_ARRAY_BUFFER, vbuf->bufferID()));
        fHWGeometryState.fArrayPtrsDirty = true;
        fHWGeometryState.fVertexBuffer = vbuf;
    }

    if (indexed) {
        GrAssert(NULL != extraIndexOffset);

        GrGLIndexBuffer* ibuf;
        switch (fGeometrySrc.fIndexSrc) {
        case kBuffer_GeometrySrcType:
            *extraIndexOffset = 0;
            ibuf = (GrGLIndexBuffer*)fGeometrySrc.fIndexBuffer;
            break;
        case kArray_GeometrySrcType:
        case kReserved_GeometrySrcType:
            finalizeReservedIndices();
            *extraIndexOffset = fCurrPoolStartIndex;
            ibuf = (GrGLIndexBuffer*) fCurrPoolIndexBuffer;
            break;
        default:
            ibuf = NULL; // suppress warning
            GrCrash("Unknown geometry src type!");
        }

        GrAssert(NULL != ibuf);
        GrAssert(!ibuf->isLocked());
        if (fHWGeometryState.fIndexBuffer != ibuf) {
            GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf->bufferID()));
            fHWGeometryState.fIndexBuffer = ibuf;
        }
    }
}
