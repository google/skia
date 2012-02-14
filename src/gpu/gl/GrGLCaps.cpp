
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLCaps.h"
#include "GrGLContextInfo.h"

GrGLCaps::GrGLCaps() {
    this->reset();
}

void GrGLCaps::reset() {
    fVerifiedColorConfigs.reset();
    fStencilFormats.reset();
    fStencilVerifiedColorConfigs.reset();
    fMSFBOType = kNone_MSFBOType;
    fMaxFragmentUniformVectors = 0;
    fRGBA8RenderbufferSupport = false;
    fBGRAFormatSupport = false;
    fBGRAIsInternalFormat = false;
    fTextureSwizzleSupport = false;
    fUnpackRowLengthSupport = false;
    fUnpackFlipYSupport = false;
    fPackRowLengthSupport = false;
    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fTexStorageSupport = false;
}

GrGLCaps::GrGLCaps(const GrGLCaps& caps) {
    *this = caps;
}

GrGLCaps& GrGLCaps::operator = (const GrGLCaps& caps) {
    fVerifiedColorConfigs = caps.fVerifiedColorConfigs;
    fStencilFormats = caps.fStencilFormats;
    fStencilVerifiedColorConfigs = caps.fStencilVerifiedColorConfigs;
    fMaxFragmentUniformVectors = caps.fMaxFragmentUniformVectors;
    fMSFBOType = caps.fMSFBOType;
    fRGBA8RenderbufferSupport = caps.fRGBA8RenderbufferSupport;
    fBGRAFormatSupport = caps.fBGRAFormatSupport;
    fBGRAIsInternalFormat = caps.fBGRAIsInternalFormat;
    fTextureSwizzleSupport = caps.fTextureSwizzleSupport;
    fUnpackRowLengthSupport = caps.fUnpackRowLengthSupport;
    fUnpackFlipYSupport = caps.fUnpackFlipYSupport;
    fPackRowLengthSupport = caps.fPackRowLengthSupport;
    fPackFlipYSupport = caps.fPackFlipYSupport;
    fTextureUsageSupport = caps.fTextureUsageSupport;
    fTexStorageSupport = caps.fTexStorageSupport;

    return *this;
}

void GrGLCaps::init(const GrGLContextInfo& ctxInfo) {

    this->reset();
    if (!ctxInfo.isInitialized()) {
        return;
    }

    const GrGLInterface* gli = ctxInfo.interface();
    GrGLBinding binding = ctxInfo.binding();
    GrGLVersion version = ctxInfo.version();

    if (kES2_GrGLBinding == binding) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fMaxFragmentUniformVectors);
    } else {
        GrAssert(kDesktop_GrGLBinding == binding);
        GrGLint max;
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fMaxFragmentUniformVectors = max / 4;
    }

    if (kDesktop_GrGLBinding == binding) {
        fRGBA8RenderbufferSupport = true;
    } else {
        fRGBA8RenderbufferSupport = ctxInfo.hasExtension("GL_OES_rgb8_rgba8") ||
                                    ctxInfo.hasExtension("GL_ARM_rgba8");
    }

    if (kDesktop_GrGLBinding == binding) {
        fBGRAFormatSupport = version >= GR_GL_VER(1,2) ||
                             ctxInfo.hasExtension("GL_EXT_bgra");
    } else {
        bool hasBGRAExt = false;
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
            fBGRAIsInternalFormat = true;
        }
        GrAssert(fBGRAFormatSupport ||
                 kSkia8888_PM_GrPixelConfig != kBGRA_8888_PM_GrPixelConfig);
    }

    if (kDesktop_GrGLBinding == binding) {
        fTextureSwizzleSupport = version >= GR_GL_VER(3,3) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_swizzle");
    } else {
        fTextureSwizzleSupport = false;
    }

    if (kDesktop_GrGLBinding == binding) {
        fUnpackRowLengthSupport = true;
        fUnpackFlipYSupport = false;
        fPackRowLengthSupport = true;
        fPackFlipYSupport = false;
    } else {
        fUnpackRowLengthSupport =ctxInfo.hasExtension("GL_EXT_unpack_subimage");
        fUnpackFlipYSupport = ctxInfo.hasExtension("GL_CHROMIUM_flipy");
        // no extension for pack row length
        fPackRowLengthSupport = false;
        fPackFlipYSupport =
            ctxInfo.hasExtension("GL_ANGLE_pack_reverse_row_order");
    }

    fTextureUsageSupport = (kES2_GrGLBinding == binding) &&
                            ctxInfo.hasExtension("GL_ANGLE_texture_usage");

    // Tex storage is in desktop 4.2 and can be an extension to desktop or ES.
    fTexStorageSupport = (kDesktop_GrGLBinding == binding &&
                          version >= GR_GL_VER(4,2)) ||
                         ctxInfo.hasExtension("GL_ARB_texture_storage") ||
                         ctxInfo.hasExtension("GL_EXT_texture_storage");

    this->initFSAASupport(ctxInfo);
    this->initStencilFormats(ctxInfo);
}

void GrGLCaps::initFSAASupport(const GrGLContextInfo& ctxInfo) {

    fMSFBOType = kNone_MSFBOType;
    if (kDesktop_GrGLBinding != ctxInfo.binding()) {
       if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
           // chrome's extension is equivalent to the EXT msaa
           // and fbo_blit extensions.
           fMSFBOType = kDesktopEXT_MSFBOType;
       } else if (ctxInfo.hasExtension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kAppleES_MSFBOType;
        }
    } else {
        if ((ctxInfo.version() >= GR_GL_VER(3,0)) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fMSFBOType = GrGLCaps::kDesktopARB_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_EXT_framebuffer_multisample") &&
                   ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = GrGLCaps::kDesktopEXT_MSFBOType;
        }
    }
}

namespace {
const GrGLuint kUnknownBitCount = GrGLStencilBuffer::kUnknownBitCount;
}

void GrGLCaps::initStencilFormats(const GrGLContextInfo& ctxInfo) {

    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    // these consts are in order of most preferred to least preferred
    // we don't bother with GL_STENCIL_INDEX1 or GL_DEPTH32F_STENCIL8

    static const StencilFormat
                  // internal Format      stencil bits      total bits        packed?
        gS8    = {GR_GL_STENCIL_INDEX8,   8,                8,                false},
        gS16   = {GR_GL_STENCIL_INDEX16,  16,               16,               false},
        gD24S8 = {GR_GL_DEPTH24_STENCIL8, 8,                32,               true },
        gS4    = {GR_GL_STENCIL_INDEX4,   4,                4,                false},
        gS     = {GR_GL_STENCIL_INDEX,    kUnknownBitCount, kUnknownBitCount, false},
        gDS    = {GR_GL_DEPTH_STENCIL,    kUnknownBitCount, kUnknownBitCount, true };

    if (kDesktop_GrGLBinding == ctxInfo.binding()) {
        bool supportsPackedDS =
            ctxInfo.version() >= GR_GL_VER(3,0) || 
            ctxInfo.hasExtension("GL_EXT_packed_depth_stencil") ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object");

        // S1 thru S16 formats are in GL 3.0+, EXT_FBO, and ARB_FBO since we
        // require FBO support we can expect these are legal formats and don't
        // check. These also all support the unsized GL_STENCIL_INDEX.
        fStencilFormats.push_back() = gS8;
        fStencilFormats.push_back() = gS16;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gD24S8;
        }
        fStencilFormats.push_back() = gS4;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gDS;
        }
    } else {
        // ES2 has STENCIL_INDEX8 without extensions but requires extensions
        // for other formats.
        // ES doesn't support using the unsized format.

        fStencilFormats.push_back() = gS8;
        //fStencilFormats.push_back() = gS16;
        if (ctxInfo.hasExtension("GL_OES_packed_depth_stencil")) {
            fStencilFormats.push_back() = gD24S8;
        }
        if (ctxInfo.hasExtension("GL_OES_stencil4")) {
            fStencilFormats.push_back() = gS4;
        }
    }
    GrAssert(0 == fStencilVerifiedColorConfigs.count());
    fStencilVerifiedColorConfigs.push_back_n(fStencilFormats.count());
}

void GrGLCaps::markColorConfigAndStencilFormatAsVerified(
                                    GrPixelConfig config,
                                    const GrGLStencilBuffer::Format& format) {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
    return;
#endif
    GrAssert((unsigned)config < kGrPixelConfigCount);
    GrAssert(fStencilFormats.count() == fStencilVerifiedColorConfigs.count());
    int count = fStencilFormats.count();
    // we expect a really small number of possible formats so linear search
    // should be OK
    GrAssert(count < 16);
    for (int i = 0; i < count; ++i) {
        if (format.fInternalFormat ==
            fStencilFormats[i].fInternalFormat) {
            fStencilVerifiedColorConfigs[i].markVerified(config);
            return;
        }
    }
    GrCrash("Why are we seeing a stencil format that "
            "GrGLCaps doesn't know about.");
}

bool GrGLCaps::isColorConfigAndStencilFormatVerified(
                                GrPixelConfig config,
                                const GrGLStencilBuffer::Format& format) const {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
    return false;
#endif
    GrAssert((unsigned)config < kGrPixelConfigCount);
    int count = fStencilFormats.count();
    // we expect a really small number of possible formats so linear search
    // should be OK
    GrAssert(count < 16);
    for (int i = 0; i < count; ++i) {
        if (format.fInternalFormat ==
            fStencilFormats[i].fInternalFormat) {
            return fStencilVerifiedColorConfigs[i].isVerified(config);
        }
    }
    GrCrash("Why are we seeing a stencil format that "
            "GLCaps doesn't know about.");
    return false;
}

void GrGLCaps::print() const {
    for (int i = 0; i < fStencilFormats.count(); ++i) {
        GrPrintf("Stencil Format %d, stencil bits: %02d, total bits: %02d\n",
                 i,
                 fStencilFormats[i].fStencilBits,
                 fStencilFormats[i].fTotalBits);
    }

    GR_STATIC_ASSERT(0 == kNone_MSFBOType);
    GR_STATIC_ASSERT(1 == kDesktopARB_MSFBOType);
    GR_STATIC_ASSERT(2 == kDesktopEXT_MSFBOType);
    GR_STATIC_ASSERT(3 == kAppleES_MSFBOType);
    static const char* gMSFBOExtStr[] = {
        "None",
        "ARB",
        "EXT",
        "Apple",
    };
    GrPrintf("MSAA Type: %s\n", gMSFBOExtStr[fMSFBOType]);
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

