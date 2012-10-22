/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLCaps.h"
#include "GrGLContextInfo.h"
#include "SkTSearch.h"

GrGLCaps::GrGLCaps() {
    this->reset();
}

void GrGLCaps::reset() {
    fVerifiedColorConfigs.reset();
    fStencilFormats.reset();
    fStencilVerifiedColorConfigs.reset();
    fMSFBOType = kNone_MSFBOType;
    fMaxSampleCount = 0;
    fCoverageAAType = kNone_CoverageAAType;
    fMaxFragmentUniformVectors = 0;
    fMaxVertexAttributes = 0;
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
    fTextureRedSupport = false;
    fImagingSupport = false;
    fTwoFormatLimit = false;
}

GrGLCaps::GrGLCaps(const GrGLCaps& caps) {
    *this = caps;
}

GrGLCaps& GrGLCaps::operator = (const GrGLCaps& caps) {
    fVerifiedColorConfigs = caps.fVerifiedColorConfigs;
    fStencilFormats = caps.fStencilFormats;
    fStencilVerifiedColorConfigs = caps.fStencilVerifiedColorConfigs;
    fMaxFragmentUniformVectors = caps.fMaxFragmentUniformVectors;
    fMaxVertexAttributes = caps.fMaxVertexAttributes;
    fMSFBOType = caps.fMSFBOType;
    fMaxSampleCount = caps.fMaxSampleCount;
    fCoverageAAType = caps.fCoverageAAType;
    fMSAACoverageModes = caps.fMSAACoverageModes;
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
    fTextureRedSupport = caps.fTextureRedSupport;
    fImagingSupport = caps.fImagingSupport;
    fTwoFormatLimit = caps.fTwoFormatLimit;

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
    GR_GL_GetIntegerv(gli, GR_GL_MAX_VERTEX_ATTRIBS, &fMaxVertexAttributes);

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
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
            fBGRAIsInternalFormat = true;
        }
        GrAssert(fBGRAFormatSupport ||
                 kSkia8888_GrPixelConfig != kBGRA_8888_GrPixelConfig);
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

    // ARB_texture_rg is part of OpenGL 3.0
    if (kDesktop_GrGLBinding == binding) {
        fTextureRedSupport = version >= GR_GL_VER(3,0) ||
                             ctxInfo.hasExtension("GL_ARB_texture_rg");
    } else {
        fTextureRedSupport = ctxInfo.hasExtension("GL_EXT_texture_rg");
    }

    fImagingSupport = kDesktop_GrGLBinding == binding &&
                      ctxInfo.hasExtension("GL_ARB_imaging");

    // ES 2 only guarantees RGBA/uchar + one other format/type combo for
    // ReadPixels. The other format has to checked at run-time since it
    // can change based on which render target is bound
    fTwoFormatLimit = kES2_GrGLBinding == binding;

    this->initFSAASupport(ctxInfo);
    this->initStencilFormats(ctxInfo);
}

bool GrGLCaps::readPixelsSupported(const GrGLInterface* intf,
                                   GrGLenum format,
                                   GrGLenum type) const {
    if (GR_GL_RGBA == format && GR_GL_UNSIGNED_BYTE == type) {
        // ES 2 guarantees this format is supported
        return true;
    }

    if (!fTwoFormatLimit) {
        // not limited by ES 2's constraints
        return true;
    }

    GrGLint otherFormat = GR_GL_RGBA;
    GrGLint otherType = GR_GL_UNSIGNED_BYTE;

    // The other supported format/type combo supported for ReadPixels
    // can change based on which render target is bound
    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                      &otherFormat);

    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_TYPE,
                      &otherType);

    return (GrGLenum)otherFormat == format && (GrGLenum)otherType == type;
}

namespace {
int coverage_mode_compare(const GrGLCaps::MSAACoverageMode* left,
                          const GrGLCaps::MSAACoverageMode* right) {
    if (left->fCoverageSampleCnt < right->fCoverageSampleCnt) {
        return -1;
    } else if (right->fCoverageSampleCnt < left->fCoverageSampleCnt) {
        return 1;
    } else if (left->fColorSampleCnt < right->fColorSampleCnt) {
        return -1;
    } else if (right->fColorSampleCnt < left->fColorSampleCnt) {
        return 1;
    }
    return 0;
}
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
        // TODO: We could populate fMSAACoverageModes using GetInternalformativ
        // on GL 4.2+. It's format-specific, though. See also
        // http://code.google.com/p/skia/issues/detail?id=470 about using actual
        // rather than requested sample counts in cache key.
        if (ctxInfo.hasExtension("GL_NV_framebuffer_multisample_coverage")) {
            fCoverageAAType = kNVDesktop_CoverageAAType;
            GrGLint count;
            GR_GL_GetIntegerv(ctxInfo.interface(),
                              GR_GL_MAX_MULTISAMPLE_COVERAGE_MODES,
                              &count);
            fMSAACoverageModes.setCount(count);
            GR_GL_GetIntegerv(ctxInfo.interface(),
                              GR_GL_MULTISAMPLE_COVERAGE_MODES,
                              (int*)&fMSAACoverageModes[0]);
            // The NV driver seems to return the modes already sorted but the
            // spec doesn't require this. So we sort.
            qsort(&fMSAACoverageModes[0],
                    count,
                    sizeof(MSAACoverageMode),
                    SkCastForQSort(coverage_mode_compare));
        }
    }
    if (kNone_MSFBOType != fMSFBOType) {
        GR_GL_GetIntegerv(ctxInfo.interface(),
                          GR_GL_MAX_SAMPLES,
                          &fMaxSampleCount);
    }
}

const GrGLCaps::MSAACoverageMode& GrGLCaps::getMSAACoverageMode(
                                            int desiredSampleCount) const {
    static const MSAACoverageMode kNoneMode = {0, 0};
    if (0 == fMSAACoverageModes.count()) {
        return kNoneMode;
    } else {
        GrAssert(kNone_CoverageAAType != fCoverageAAType);
        int max = (fMSAACoverageModes.end() - 1)->fCoverageSampleCnt;
        desiredSampleCount = GrMin(desiredSampleCount, max);
        MSAACoverageMode desiredMode = {desiredSampleCount, 0};
        int idx = SkTSearch<MSAACoverageMode>(&fMSAACoverageModes[0],
                                              fMSAACoverageModes.count(),
                                              desiredMode,
                                              sizeof(MSAACoverageMode),
                                              &coverage_mode_compare);
        if (idx < 0) {
            idx = ~idx;
        }
        GrAssert(idx >= 0 && idx < fMSAACoverageModes.count());
        return fMSAACoverageModes[idx];
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
    //  gS     = {GR_GL_STENCIL_INDEX,    kUnknownBitCount, kUnknownBitCount, false},
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
    GrPrintf("Two Format Limit: %s\n", (fTwoFormatLimit ? "YES": "NO"));
}

