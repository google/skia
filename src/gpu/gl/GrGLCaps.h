/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLCaps_DEFINED
#define GrGLCaps_DEFINED

#include "SkTArray.h"
#include "SkTDArray.h"
#include "GrGLStencilBuffer.h"

class GrGLContextInfo;

/**
 * Stores some capabilities of a GL context. Most are determined by the GL
 * version and the extensions string. It also tracks formats that have passed
 * the FBO completeness test.
 */
class GrGLCaps {
public:
    typedef GrGLStencilBuffer::Format StencilFormat;

    /**
     * Represents a supported multisampling/coverage-sampling mode.
     */
    struct MSAACoverageMode {
        // "Coverage samples" includes samples that actually have color, depth,
        // stencil, ... as well as those that don't (coverage only). All samples
        // are coverage samples. (We're using the word "coverage sample" to
        // match the NV extension language.)
        int fCoverageSampleCnt;

        // Color samples are samples that store data values (color, stencil,
        // depth) rather than just representing coverage. They are a subset
        // of coverage samples. (Again the wording was chosen to match the
        // extension.)
        int fColorSampleCnt;
    };

    /**
     * The type of MSAA for FBOs supported. Different extensions have different
     * semantics of how / when a resolve is performed.
     */
    enum MSFBOType {
        /**
         * no support for MSAA FBOs
         */
        kNone_MSFBOType = 0,
        /**
         * GL3.0-style MSAA FBO (GL_ARB_framebuffer_object)
         */
        kDesktopARB_MSFBOType,
        /**
         * earlier GL_EXT_framebuffer* extensions
         */
        kDesktopEXT_MSFBOType,
        /**
         * GL_APPLE_framebuffer_multisample ES extension
         */
        kAppleES_MSFBOType,
    };

    enum CoverageAAType {
        /**
         * No coverage sample support
         */
        kNone_CoverageAAType,

        /**
         * GL_NV_framebuffer_multisample_coverage
         */
        kNVDesktop_CoverageAAType,
    };

    /**
     * Creates a GrGLCaps that advertises no support for any extensions,
     * formats, etc. Call init to initialize from a GrGLContextInfo.
     */
    GrGLCaps();

    GrGLCaps(const GrGLCaps& caps);

    GrGLCaps& operator = (const GrGLCaps& caps);

    /**
     * Resets the caps such that nothing is supported.
     */
    void reset();

    /**
     * Initializes the GrGLCaps to the set of features supported in the current
     * OpenGL context accessible via ctxInfo.
     */
    void init(const GrGLContextInfo& ctxInfo);

    /**
     * Call to note that a color config has been verified as a valid color
     * attachment. This may save future calls to glCheckFramebufferStatus
     * using isConfigVerifiedColorAttachment().
     */
    void markConfigAsValidColorAttachment(GrPixelConfig config) {
        fVerifiedColorConfigs.markVerified(config);
    }

    /**
     * Call to check whether a config has been verified as a valid color
     * attachment.
     */
    bool isConfigVerifiedColorAttachment(GrPixelConfig config) const {
        return fVerifiedColorConfigs.isVerified(config);
    }

    /**
     * Call to note that a color config / stencil format pair passed
     * FBO status check. We may skip calling glCheckFramebufferStatus for
     * this combination in the future using
     * isColorConfigAndStencilFormatVerified().
     */
    void markColorConfigAndStencilFormatAsVerified(
                    GrPixelConfig config,
                    const GrGLStencilBuffer::Format& format);

    /**
     * Call to check whether color config / stencil format pair has already
     * passed FBO status check.
     */
    bool isColorConfigAndStencilFormatVerified(
                    GrPixelConfig config,
                    const GrGLStencilBuffer::Format& format) const;

    /**
     * Reports the type of MSAA FBO support.
     */
    MSFBOType msFBOType() const { return fMSFBOType; }

    /**
     * Reports the maximum number of samples supported.
     */
    int maxSampleCount() const { return fMaxSampleCount; }

    /**
     * Reports the type of coverage sample AA support.
     */
    CoverageAAType coverageAAType() const { return fCoverageAAType; }

    /**
     * Chooses a supported coverage mode based on a desired sample count. The
     * desired sample count is rounded up the next supported coverage sample
     * count unless a it is larger than the max in which case it is rounded
     * down. Once a coverage sample count is decided, the supported mode with
     * the fewest color samples is chosen.
     */
    const MSAACoverageMode& getMSAACoverageMode(int desiredSampleCount) const;

    /**
     * Prints the caps info using GrPrintf.
     */
    void print() const;

    /**
     * Gets an array of legal stencil formats. These formats are not guaranteed
     * to be supported by the driver but are legal GLenum names given the GL
     * version and extensions supported.
     */
    const SkTArray<StencilFormat, true>& stencilFormats() const {
        return fStencilFormats;
    }

    /// The maximum number of fragment uniform vectors (GLES has min. 16).
    int maxFragmentUniformVectors() const { return fMaxFragmentUniformVectors; }

    // maximum number of attribute values per vertex
    int maxVertexAttributes() const { return fMaxVertexAttributes; }

    /// ES requires an extension to support RGBA8 in RenderBufferStorage
    bool rgba8RenderbufferSupport() const { return fRGBA8RenderbufferSupport; }

    /// Is GL_BGRA supported
    bool bgraFormatSupport() const { return fBGRAFormatSupport; }

    /**
     * Depending on the ES extensions present the BGRA external format may
     * correspond either a BGRA or RGBA internalFormat. On desktop GL it is
     * RGBA.
     */
    bool bgraIsInternalFormat() const { return fBGRAIsInternalFormat; }

    /// GL_ARB_texture_swizzle support
    bool textureSwizzleSupport() const { return fTextureSwizzleSupport; }

    /// Is there support for GL_UNPACK_ROW_LENGTH
    bool unpackRowLengthSupport() const { return fUnpackRowLengthSupport; }

    /// Is there support for GL_UNPACK_FLIP_Y
    bool unpackFlipYSupport() const { return fUnpackFlipYSupport; }

    /// Is there support for GL_PACK_ROW_LENGTH
    bool packRowLengthSupport() const { return fPackRowLengthSupport; }

    /// Is there support for GL_PACK_REVERSE_ROW_ORDER
    bool packFlipYSupport() const { return fPackFlipYSupport; }

    /// Is there support for texture parameter GL_TEXTURE_USAGE
    bool textureUsageSupport() const { return fTextureUsageSupport; }

    /// Is there support for glTexStorage
    bool texStorageSupport() const { return fTexStorageSupport; }

    /// Is there support for GL_RED and GL_R8
    bool textureRedSupport() const { return fTextureRedSupport; }

    /// Is GL_ARB_IMAGING supported
    bool imagingSupport() const { return fImagingSupport; }

    // Does ReadPixels support the provided format/type combo?
    bool readPixelsSupported(const GrGLInterface* intf,
                             GrGLenum format,
                             GrGLenum type) const;

private:
    /**
     * Maintains a bit per GrPixelConfig. It is used to avoid redundantly
     * performing glCheckFrameBufferStatus for the same config.
     */
    struct VerifiedColorConfigs {
        VerifiedColorConfigs() {
            this->reset();
        }

        void reset() {
            for (int i = 0; i < kNumUints; ++i) {
                fVerifiedColorConfigs[i] = 0;
            }
        }

        static const int kNumUints = (kGrPixelConfigCount  + 31) / 32;
        uint32_t fVerifiedColorConfigs[kNumUints];

        void markVerified(GrPixelConfig config) {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
                return;
#endif
            int u32Idx = config / 32;
            int bitIdx = config % 32;
            fVerifiedColorConfigs[u32Idx] |= 1 << bitIdx;
        }

        bool isVerified(GrPixelConfig config) const {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
            return false;
#endif
            int u32Idx = config / 32;
            int bitIdx = config % 32;
            return SkToBool(fVerifiedColorConfigs[u32Idx] & (1 << bitIdx));
        }
    };

    void initFSAASupport(const GrGLContextInfo& ctxInfo);
    void initStencilFormats(const GrGLContextInfo& ctxInfo);

    // tracks configs that have been verified to pass the FBO completeness when
    // used as a color attachment
    VerifiedColorConfigs fVerifiedColorConfigs;

    SkTArray<StencilFormat, true> fStencilFormats;
    // tracks configs that have been verified to pass the FBO completeness when
    // used as a color attachment when a particular stencil format is used
    // as a stencil attachment.
    SkTArray<VerifiedColorConfigs, true> fStencilVerifiedColorConfigs;

    int fMaxFragmentUniformVectors;
    int fMaxVertexAttributes;

    MSFBOType fMSFBOType;
    int fMaxSampleCount;
    CoverageAAType fCoverageAAType;
    SkTDArray<MSAACoverageMode> fMSAACoverageModes;

    bool fRGBA8RenderbufferSupport : 1;
    bool fBGRAFormatSupport : 1;
    bool fBGRAIsInternalFormat : 1;
    bool fTextureSwizzleSupport : 1;
    bool fUnpackRowLengthSupport : 1;
    bool fUnpackFlipYSupport : 1;
    bool fPackRowLengthSupport : 1;
    bool fPackFlipYSupport : 1;
    bool fTextureUsageSupport : 1;
    bool fTexStorageSupport : 1;
    bool fTextureRedSupport : 1;
    bool fImagingSupport  : 1;
    bool fTwoFormatLimit : 1;
};

#endif
