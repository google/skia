/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuGL.h"
#include "GrGLStencilBuffer.h"
#include "GrGLPath.h"
#include "GrGLShaderBuilder.h"
#include "GrTemplates.h"
#include "GrTypes.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"

static const GrGLuint GR_MAX_GLUINT = ~0U;
static const GrGLint  GR_INVAL_GLINT = ~0;

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glInterface(), RET, X)


#define SKIP_CACHE_CHECK    true

#if GR_GL_CHECK_ALLOC_WITH_GET_ERROR
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)   GrGLClearErr(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL_NOERRCHECK(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_GET_ERROR(iface)
#else
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_NO_ERROR
#endif


///////////////////////////////////////////////////////////////////////////////

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
    GR_STATIC_ASSERT(kTotalGrBlendCoeffCount ==
                     GR_ARRAY_COUNT(gCoeffReferencesBlendConst));

    GR_STATIC_ASSERT(0 == kZero_GrBlendCoeff);
    GR_STATIC_ASSERT(1 == kOne_GrBlendCoeff);
    GR_STATIC_ASSERT(2 == kSC_GrBlendCoeff);
    GR_STATIC_ASSERT(3 == kISC_GrBlendCoeff);
    GR_STATIC_ASSERT(4 == kDC_GrBlendCoeff);
    GR_STATIC_ASSERT(5 == kIDC_GrBlendCoeff);
    GR_STATIC_ASSERT(6 == kSA_GrBlendCoeff);
    GR_STATIC_ASSERT(7 == kISA_GrBlendCoeff);
    GR_STATIC_ASSERT(8 == kDA_GrBlendCoeff);
    GR_STATIC_ASSERT(9 == kIDA_GrBlendCoeff);
    GR_STATIC_ASSERT(10 == kConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(11 == kIConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(12 == kConstA_GrBlendCoeff);
    GR_STATIC_ASSERT(13 == kIConstA_GrBlendCoeff);

    GR_STATIC_ASSERT(14 == kS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(15 == kIS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(16 == kS2A_GrBlendCoeff);
    GR_STATIC_ASSERT(17 == kIS2A_GrBlendCoeff);

    // assertion for gXfermodeCoeff2Blend have to be in GrGpu scope
    GR_STATIC_ASSERT(kTotalGrBlendCoeffCount ==
                     GR_ARRAY_COUNT(gXfermodeCoeff2Blend));
}

///////////////////////////////////////////////////////////////////////////////

static bool gPrintStartupSpew;

GrGpuGL::GrGpuGL(const GrGLContext& ctx, GrContext* context)
    : GrGpu(context)
    , fGLContext(ctx) {

    SkASSERT(ctx.isInitialized());

    fCaps.reset(SkRef(ctx.info().caps()));

    fHWBoundTextures.reset(ctx.info().caps()->maxFragmentTextureUnits());
    fHWTexGenSettings.reset(ctx.info().caps()->maxFixedFunctionTextureCoords());

    GrGLClearErr(fGLContext.interface());

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
        GrPrintf("------ EXTENSIONS\n");
        ctx.info().extensions().print();
        GrPrintf("\n");
        ctx.info().caps()->print();
    }

    fProgramCache = SkNEW_ARGS(ProgramCache, (this));

    SkASSERT(this->glCaps().maxVertexAttributes() >= GrDrawState::kMaxVertexAttribCnt);

    fLastSuccessfulStencilFmtIdx = 0;
    fHWProgramID = 0;
}

GrGpuGL::~GrGpuGL() {
    if (0 != fHWProgramID) {
        // detach the current program so there is no confusion on OpenGL's part
        // that we want it to be deleted
        SkASSERT(fHWProgramID == fCurrentProgram->programID());
        GL_CALL(UseProgram(0));
    }

    delete fProgramCache;

    // This must be called by before the GrDrawTarget destructor
    this->releaseGeometry();
    // This subclass must do this before the base class destructor runs
    // since we will unref the GrGLInterface.
    this->releaseResources();
}

///////////////////////////////////////////////////////////////////////////////


GrPixelConfig GrGpuGL::preferredReadPixelsConfig(GrPixelConfig readConfig,
                                                 GrPixelConfig surfaceConfig) const {
    if (GR_GL_RGBA_8888_PIXEL_OPS_SLOW && kRGBA_8888_GrPixelConfig == readConfig) {
        return kBGRA_8888_GrPixelConfig;
    } else if (fGLContext.info().isMesa() &&
               GrBytesPerPixel(readConfig) == 4 &&
               GrPixelConfigSwapRAndB(readConfig) == surfaceConfig) {
        // Mesa 3D takes a slow path on when reading back  BGRA from an RGBA surface and vice-versa.
        // Perhaps this should be guarded by some compiletime or runtime check.
        return surfaceConfig;
    } else if (readConfig == kBGRA_8888_GrPixelConfig &&
               !this->glCaps().readPixelsSupported(this->glInterface(),
                                                   GR_GL_BGRA, GR_GL_UNSIGNED_BYTE)) {
        return kRGBA_8888_GrPixelConfig;
    } else {
        return readConfig;
    }
}

GrPixelConfig GrGpuGL::preferredWritePixelsConfig(GrPixelConfig writeConfig,
                                                  GrPixelConfig surfaceConfig) const {
    if (GR_GL_RGBA_8888_PIXEL_OPS_SLOW && kRGBA_8888_GrPixelConfig == writeConfig) {
        return kBGRA_8888_GrPixelConfig;
    } else {
        return writeConfig;
    }
}

bool GrGpuGL::canWriteTexturePixels(const GrTexture* texture, GrPixelConfig srcConfig) const {
    if (kIndex_8_GrPixelConfig == srcConfig || kIndex_8_GrPixelConfig == texture->config()) {
        return false;
    }
    if (srcConfig != texture->config() && kES_GrGLBinding == this->glBinding()) {
        // In general ES2 requires the internal format of the texture and the format of the src
        // pixels to match. However, It may or may not be possible to upload BGRA data to a RGBA
        // texture. It depends upon which extension added BGRA. The Apple extension allows it
        // (BGRA's internal format is RGBA) while the EXT extension does not (BGRA is its own
        // internal format).
        if (this->glCaps().bgraFormatSupport() &&
            !this->glCaps().bgraIsInternalFormat() &&
            kBGRA_8888_GrPixelConfig == srcConfig &&
            kRGBA_8888_GrPixelConfig == texture->config()) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool GrGpuGL::fullReadPixelsIsFasterThanPartial() const {
    return SkToBool(GR_GL_FULL_READPIXELS_FASTER_THAN_PARTIAL);
}

void GrGpuGL::onResetContext(uint32_t resetBits) {
    // we don't use the zb at all
    if (resetBits & kMisc_GrGLBackendState) {
        GL_CALL(Disable(GR_GL_DEPTH_TEST));
        GL_CALL(DepthMask(GR_GL_FALSE));

        fHWDrawFace = GrDrawState::kInvalid_DrawFace;
        fHWDitherEnabled = kUnknown_TriState;

        if (kDesktop_GrGLBinding == this->glBinding()) {
            // Desktop-only state that we never change
            if (!this->glCaps().isCoreProfile()) {
                GL_CALL(Disable(GR_GL_POINT_SMOOTH));
                GL_CALL(Disable(GR_GL_LINE_SMOOTH));
                GL_CALL(Disable(GR_GL_POLYGON_SMOOTH));
                GL_CALL(Disable(GR_GL_POLYGON_STIPPLE));
                GL_CALL(Disable(GR_GL_COLOR_LOGIC_OP));
                GL_CALL(Disable(GR_GL_INDEX_LOGIC_OP));
            }
            // The windows NVIDIA driver has GL_ARB_imaging in the extension string when using a
            // core profile. This seems like a bug since the core spec removes any mention of
            // GL_ARB_imaging.
            if (this->glCaps().imagingSupport() && !this->glCaps().isCoreProfile()) {
                GL_CALL(Disable(GR_GL_COLOR_TABLE));
            }
            GL_CALL(Disable(GR_GL_POLYGON_OFFSET_FILL));
            // Since ES doesn't support glPointSize at all we always use the VS to
            // set the point size
            GL_CALL(Enable(GR_GL_VERTEX_PROGRAM_POINT_SIZE));

            // We should set glPolygonMode(FRONT_AND_BACK,FILL) here, too. It isn't
            // currently part of our gl interface. There are probably others as
            // well.
        }
        fHWWriteToColor = kUnknown_TriState;
        // we only ever use lines in hairline mode
        GL_CALL(LineWidth(1));
    }

    if (resetBits & kAA_GrGLBackendState) {
        fHWAAState.invalidate();
    }

    fHWActiveTextureUnitIdx = -1; // invalid

    if (resetBits & kTextureBinding_GrGLBackendState) {
        for (int s = 0; s < fHWBoundTextures.count(); ++s) {
            fHWBoundTextures[s] = NULL;
        }
    }

    if (resetBits & kBlend_GrGLBackendState) {
        fHWBlendState.invalidate();
    }

    if (resetBits & kView_GrGLBackendState) {
        fHWScissorSettings.invalidate();
        fHWViewport.invalidate();
    }

    if (resetBits & kStencil_GrGLBackendState) {
        fHWStencilSettings.invalidate();
        fHWStencilTestEnabled = kUnknown_TriState;
    }

    // Vertex
    if (resetBits & kVertex_GrGLBackendState) {
        fHWGeometryState.invalidate();
    }

    if (resetBits & kRenderTarget_GrGLBackendState) {
        fHWBoundRenderTarget = NULL;
    }

    if (resetBits & (kFixedFunction_GrGLBackendState | kPathRendering_GrGLBackendState)) {
        if (this->glCaps().fixedFunctionSupport()) {
            fHWProjectionMatrixState.invalidate();
            // we don't use the model view matrix.
            GL_CALL(MatrixMode(GR_GL_MODELVIEW));
            GL_CALL(LoadIdentity());

            for (int i = 0; i < this->glCaps().maxFixedFunctionTextureCoords(); ++i) {
                GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + i));
                GL_CALL(Disable(GR_GL_TEXTURE_GEN_S));
                GL_CALL(Disable(GR_GL_TEXTURE_GEN_T));
                GL_CALL(Disable(GR_GL_TEXTURE_GEN_Q));
                GL_CALL(Disable(GR_GL_TEXTURE_GEN_R));
                if (this->caps()->pathRenderingSupport()) {
                    GL_CALL(PathTexGen(GR_GL_TEXTURE0 + i, GR_GL_NONE, 0, NULL));
                }
                fHWTexGenSettings[i].fMode = GR_GL_NONE;
                fHWTexGenSettings[i].fNumComponents = 0;
            }
            fHWActiveTexGenSets = 0;
        }
        if (this->caps()->pathRenderingSupport()) {
            fHWPathStencilSettings.invalidate();
        }
    }

    // we assume these values
    if (resetBits & kPixelStore_GrGLBackendState) {
        if (this->glCaps().unpackRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().packRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().unpackFlipYSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
        }
        if (this->glCaps().packFlipYSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, GR_GL_FALSE));
        }
    }

    if (resetBits & kProgram_GrGLBackendState) {
        fHWProgramID = 0;
        fSharedGLProgramState.invalidate();
    }
}

namespace {

GrSurfaceOrigin resolve_origin(GrSurfaceOrigin origin, bool renderTarget) {
    // By default, GrRenderTargets are GL's normal orientation so that they
    // can be drawn to by the outside world without the client having
    // to render upside down.
    if (kDefault_GrSurfaceOrigin == origin) {
        return renderTarget ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
    } else {
        return origin;
    }
}

}

GrTexture* GrGpuGL::onWrapBackendTexture(const GrBackendTextureDesc& desc) {
    if (!this->configToGLFormats(desc.fConfig, false, NULL, NULL, NULL)) {
        return NULL;
    }

    if (0 == desc.fTextureHandle) {
        return NULL;
    }

    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return NULL;
    }

    GrGLTexture::Desc glTexDesc;
    // next line relies on GrBackendTextureDesc's flags matching GrTexture's
    glTexDesc.fFlags = (GrTextureFlags) desc.fFlags;
    glTexDesc.fWidth = desc.fWidth;
    glTexDesc.fHeight = desc.fHeight;
    glTexDesc.fConfig = desc.fConfig;
    glTexDesc.fSampleCnt = desc.fSampleCnt;
    glTexDesc.fTextureID = static_cast<GrGLuint>(desc.fTextureHandle);
    glTexDesc.fIsWrapped = true;
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrBackendTextureFlag);
    // FIXME:  this should be calling resolve_origin(), but Chrome code is currently
    // assuming the old behaviour, which is that backend textures are always
    // BottomLeft, even for non-RT's.  Once Chrome is fixed, change this to:
    // glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    if (kDefault_GrSurfaceOrigin == desc.fOrigin) {
        glTexDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    } else {
        glTexDesc.fOrigin = desc.fOrigin;
    }

    GrGLTexture* texture = NULL;
    if (renderTarget) {
        GrGLRenderTarget::Desc glRTDesc;
        glRTDesc.fRTFBOID = 0;
        glRTDesc.fTexFBOID = 0;
        glRTDesc.fMSColorRenderbufferID = 0;
        glRTDesc.fConfig = desc.fConfig;
        glRTDesc.fSampleCnt = desc.fSampleCnt;
        glRTDesc.fOrigin = glTexDesc.fOrigin;
        glRTDesc.fCheckAllocation = false;
        if (!this->createRenderTargetObjects(glTexDesc.fWidth,
                                             glTexDesc.fHeight,
                                             glTexDesc.fTextureID,
                                             &glRTDesc)) {
            return NULL;
        }
        texture = SkNEW_ARGS(GrGLTexture, (this, glTexDesc, glRTDesc));
    } else {
        texture = SkNEW_ARGS(GrGLTexture, (this, glTexDesc));
    }
    if (NULL == texture) {
        return NULL;
    }

    return texture;
}

GrRenderTarget* GrGpuGL::onWrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    GrGLRenderTarget::Desc glDesc;
    glDesc.fConfig = desc.fConfig;
    glDesc.fRTFBOID = static_cast<GrGLuint>(desc.fRenderTargetHandle);
    glDesc.fMSColorRenderbufferID = 0;
    glDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    glDesc.fSampleCnt = desc.fSampleCnt;
    glDesc.fIsWrapped = true;
    glDesc.fCheckAllocation = false;

    glDesc.fOrigin = resolve_origin(desc.fOrigin, true);
    GrGLIRect viewport;
    viewport.fLeft   = 0;
    viewport.fBottom = 0;
    viewport.fWidth  = desc.fWidth;
    viewport.fHeight = desc.fHeight;

    GrRenderTarget* tgt = SkNEW_ARGS(GrGLRenderTarget,
                                     (this, glDesc, viewport));
    if (desc.fStencilBits) {
        GrGLStencilBuffer::Format format;
        format.fInternalFormat = GrGLStencilBuffer::kUnknownInternalFormat;
        format.fPacked = false;
        format.fStencilBits = desc.fStencilBits;
        format.fTotalBits = desc.fStencilBits;
        static const bool kIsSBWrapped = false;
        GrGLStencilBuffer* sb = SkNEW_ARGS(GrGLStencilBuffer,
                                           (this,
                                            kIsSBWrapped,
                                            0,
                                            desc.fWidth,
                                            desc.fHeight,
                                            desc.fSampleCnt,
                                            format));
        tgt->setStencilBuffer(sb);
        sb->unref();
    }
    return tgt;
}

////////////////////////////////////////////////////////////////////////////////

bool GrGpuGL::onWriteTexturePixels(GrTexture* texture,
                                   int left, int top, int width, int height,
                                   GrPixelConfig config, const void* buffer,
                                   size_t rowBytes) {
    if (NULL == buffer) {
        return false;
    }
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, glTex->textureID()));
    GrGLTexture::Desc desc;
    desc.fFlags = glTex->desc().fFlags;
    desc.fWidth = glTex->width();
    desc.fHeight = glTex->height();
    desc.fConfig = glTex->config();
    desc.fSampleCnt = glTex->desc().fSampleCnt;
    desc.fTextureID = glTex->textureID();
    desc.fOrigin = glTex->origin();

    if (this->uploadTexData(desc, false,
                            left, top, width, height,
                            config, buffer, rowBytes)) {
        texture->dirtyMipMaps(true);
        return true;
    } else {
        return false;
    }
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

    SkIRect subRect = SkIRect::MakeXYWH(*left, *top, *width, *height);
    SkIRect bounds = SkIRect::MakeWH(surfaceWidth, surfaceHeight);

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

GrGLenum check_alloc_error(const GrTextureDesc& desc, const GrGLInterface* interface) {
    if (SkToBool(desc.fFlags & kCheckAllocation_GrTextureFlagBit)) {
        return GR_GL_GET_ERROR(interface);
    } else {
        return CHECK_ALLOC_ERROR(interface);
    }
}

}

bool GrGpuGL::uploadTexData(const GrGLTexture::Desc& desc,
                            bool isNewTexture,
                            int left, int top, int width, int height,
                            GrPixelConfig dataConfig,
                            const void* data,
                            size_t rowBytes) {
    SkASSERT(NULL != data || isNewTexture);

    size_t bpp = GrBytesPerPixel(dataConfig);
    if (!adjust_pixel_ops_params(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                 &width, &height, &data, &rowBytes)) {
        return false;
    }
    size_t trimRowBytes = width * bpp;

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    // paletted textures cannot be partially updated
    bool useTexStorage = isNewTexture &&
                         desc.fConfig != kIndex_8_GrPixelConfig &&
                         this->glCaps().texStorageSupport();

    if (useTexStorage && kDesktop_GrGLBinding == this->glBinding()) {
        // 565 is not a sized internal format on desktop GL. So on desktop with
        // 565 we always use an unsized internal format to let the system pick
        // the best sized format to convert the 565 data to. Since TexStorage
        // only allows sized internal formats we will instead use TexImage2D.
        useTexStorage = desc.fConfig != kRGB_565_GrPixelConfig;
    }

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum externalType;
    // glTexStorage requires sized internal formats on both desktop and ES. ES2 requires an unsized
    // format for glTexImage, unlike ES3 and desktop. However, we allow the driver to decide the
    // size of the internal format whenever possible and so only use a sized internal format when
    // using texture storage.
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
        if (kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
            if (this->glCaps().unpackFlipYSupport()) {
                glFlipY = true;
            } else {
                swFlipY = true;
            }
        }
        if (this->glCaps().unpackRowLengthSupport() && !swFlipY) {
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
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        if (useTexStorage) {
            // We never resize  or change formats of textures. We don't use
            // mipmaps currently.
            GL_ALLOC_CALL(this->glInterface(),
                          TexStorage2D(GR_GL_TEXTURE_2D,
                                       1, // levels
                                       internalFormat,
                                       desc.fWidth, desc.fHeight));
        } else {
            if (GR_GL_PALETTE8_RGBA8 == internalFormat) {
                GrGLsizei imageSize = desc.fWidth * desc.fHeight +
                                      kGrColorTableSize;
                GL_ALLOC_CALL(this->glInterface(),
                              CompressedTexImage2D(GR_GL_TEXTURE_2D,
                                                   0, // level
                                                   internalFormat,
                                                   desc.fWidth, desc.fHeight,
                                                   0, // border
                                                   imageSize,
                                                   data));
            } else {
                GL_ALLOC_CALL(this->glInterface(),
                              TexImage2D(GR_GL_TEXTURE_2D,
                                         0, // level
                                         internalFormat,
                                         desc.fWidth, desc.fHeight,
                                         0, // border
                                         externalFormat, externalType,
                                         data));
            }
        }
        GrGLenum error = check_alloc_error(desc, this->glInterface());
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
        SkASSERT(this->glCaps().unpackRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
    if (glFlipY) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
    }
    return succeeded;
}

static bool renderbuffer_storage_msaa(GrGLContext& ctx,
                                      int sampleCount,
                                      GrGLenum format,
                                      int width, int height) {
    CLEAR_ERROR_BEFORE_ALLOC(ctx.interface());
    SkASSERT(GrGLCaps::kNone_MSFBOType != ctx.info().caps()->msFBOType());
#if GR_GL_IGNORE_ES3_MSAA
        GL_ALLOC_CALL(ctx.interface(),
                      RenderbufferStorageMultisample(GR_GL_RENDERBUFFER,
                                                     sampleCount,
                                                     format,
                                                     width, height));
#else
    switch (ctx.info().caps()->msFBOType()) {
        case GrGLCaps::kDesktop_ARB_MSFBOType:
        case GrGLCaps::kDesktop_EXT_MSFBOType:
        case GrGLCaps::kES_3_0_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisample(GR_GL_RENDERBUFFER,
                                                            sampleCount,
                                                            format,
                                                            width, height));
            break;
        case GrGLCaps::kES_Apple_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisampleES2APPLE(GR_GL_RENDERBUFFER,
                                                                    sampleCount,
                                                                    format,
                                                                    width, height));
            break;
        case GrGLCaps::kES_EXT_MsToTexture_MSFBOType:
        case GrGLCaps::kES_IMG_MsToTexture_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisampleES2EXT(GR_GL_RENDERBUFFER,
                                                                sampleCount,
                                                                format,
                                                                width, height));
            break;
        case GrGLCaps::kNone_MSFBOType:
            GrCrash("Shouldn't be here if we don't support multisampled renderbuffers.");
            break;
    }
#endif
    return (GR_GL_NO_ERROR == CHECK_ALLOC_ERROR(ctx.interface()));;
}

bool GrGpuGL::createRenderTargetObjects(int width, int height,
                                        GrGLuint texID,
                                        GrGLRenderTarget::Desc* desc) {
    desc->fMSColorRenderbufferID = 0;
    desc->fRTFBOID = 0;
    desc->fTexFBOID = 0;
    desc->fIsWrapped = false;

    GrGLenum status;

    GrGLenum msColorFormat = 0; // suppress warning

    if (desc->fSampleCnt > 0 && GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType()) {
        goto FAILED;
    }

    GL_CALL(GenFramebuffers(1, &desc->fTexFBOID));
    if (!desc->fTexFBOID) {
        goto FAILED;
    }


    // If we are using multisampling we will create two FBOS. We render to one and then resolve to
    // the texture bound to the other. The exception is the IMG multisample extension. With this
    // extension the texture is multisampled when rendered to and then auto-resolves it when it is
    // rendered from.
    if (desc->fSampleCnt > 0 && this->glCaps().usesMSAARenderBuffers()) {
        GL_CALL(GenFramebuffers(1, &desc->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &desc->fMSColorRenderbufferID));
        if (!desc->fRTFBOID ||
            !desc->fMSColorRenderbufferID ||
            !this->configToGLFormats(desc->fConfig,
                                     // ES2 and ES3 require sized internal formats for rb storage.
                                     kES_GrGLBinding == this->glBinding(),
                                     &msColorFormat,
                                     NULL,
                                     NULL)) {
            goto FAILED;
        }
    } else {
        desc->fRTFBOID = desc->fTexFBOID;
    }

    // below here we may bind the FBO
    fHWBoundRenderTarget = NULL;
    if (desc->fRTFBOID != desc->fTexFBOID) {
        SkASSERT(desc->fSampleCnt > 0);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER,
                               desc->fMSColorRenderbufferID));
        if (!renderbuffer_storage_msaa(fGLContext,
                                       desc->fSampleCnt,
                                       msColorFormat,
                                       width, height)) {
            goto FAILED;
        }
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, desc->fRTFBOID));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                      GR_GL_COLOR_ATTACHMENT0,
                                      GR_GL_RENDERBUFFER,
                                      desc->fMSColorRenderbufferID));
        if (desc->fCheckAllocation ||
            !this->glCaps().isConfigVerifiedColorAttachment(desc->fConfig)) {
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                goto FAILED;
            }
            fGLContext.info().caps()->markConfigAsValidColorAttachment(desc->fConfig);
        }
    }
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, desc->fTexFBOID));

    if (this->glCaps().usesImplicitMSAAResolve() && desc->fSampleCnt > 0) {
        GL_CALL(FramebufferTexture2DMultisample(GR_GL_FRAMEBUFFER,
                                                GR_GL_COLOR_ATTACHMENT0,
                                                GR_GL_TEXTURE_2D,
                                                texID, 0, desc->fSampleCnt));
    } else {
        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     GR_GL_TEXTURE_2D,
                                     texID, 0));
    }
    if (desc->fCheckAllocation ||
        !this->glCaps().isConfigVerifiedColorAttachment(desc->fConfig)) {
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            goto FAILED;
        }
        fGLContext.info().caps()->markConfigAsValidColorAttachment(desc->fConfig);
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
//    SkDEBUGFAIL("null texture");
    return NULL;
}

#if 0 && defined(SK_DEBUG)
static size_t as_size_t(int x) {
    return x;
}
#endif

GrTexture* GrGpuGL::onCreateTexture(const GrTextureDesc& desc,
                                    const void* srcData,
                                    size_t rowBytes) {

    GrGLTexture::Desc glTexDesc;
    GrGLRenderTarget::Desc  glRTDesc;

    // Attempt to catch un- or wrongly initialized sample counts;
    SkASSERT(desc.fSampleCnt >= 0 && desc.fSampleCnt <= 64);
    // We fail if the MSAA was requested and is not available.
    if (GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType() && desc.fSampleCnt) {
        //GrPrintf("MSAA RT requested but not supported on this platform.");
        return return_null_texture();
    }
    // If the sample count exceeds the max then we clamp it.
    glTexDesc.fSampleCnt = GrMin(desc.fSampleCnt, this->caps()->maxSampleCount());

    glTexDesc.fFlags  = desc.fFlags;
    glTexDesc.fWidth  = desc.fWidth;
    glTexDesc.fHeight = desc.fHeight;
    glTexDesc.fConfig = desc.fConfig;
    glTexDesc.fIsWrapped = false;

    glRTDesc.fMSColorRenderbufferID = 0;
    glRTDesc.fRTFBOID = 0;
    glRTDesc.fTexFBOID = 0;
    glRTDesc.fIsWrapped = false;
    glRTDesc.fConfig = glTexDesc.fConfig;
    glRTDesc.fCheckAllocation = SkToBool(desc.fFlags & kCheckAllocation_GrTextureFlagBit);

    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrTextureFlagBit);

    glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    glRTDesc.fOrigin = glTexDesc.fOrigin;

    glRTDesc.fSampleCnt = glTexDesc.fSampleCnt;
    if (GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType() &&
        desc.fSampleCnt) {
        //GrPrintf("MSAA RT requested but not supported on this platform.");
        return return_null_texture();
    }

    if (renderTarget) {
        int maxRTSize = this->caps()->maxRenderTargetSize();
        if (glTexDesc.fWidth > maxRTSize || glTexDesc.fHeight > maxRTSize) {
            return return_null_texture();
        }
    } else {
        int maxSize = this->caps()->maxTextureSize();
        if (glTexDesc.fWidth > maxSize || glTexDesc.fHeight > maxSize) {
            return return_null_texture();
        }
    }

    GL_CALL(GenTextures(1, &glTexDesc.fTextureID));

    if (!glTexDesc.fTextureID) {
        return return_null_texture();
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, glTexDesc.fTextureID));

    if (renderTarget && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_USAGE,
                              GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTexture::TexParams initialTexParams;
    // we only set a subset here so invalidate first
    initialTexParams.invalidate();
    initialTexParams.fMinFilter = GR_GL_NEAREST;
    initialTexParams.fMagFilter = GR_GL_NEAREST;
    initialTexParams.fWrapS = GR_GL_CLAMP_TO_EDGE;
    initialTexParams.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MAG_FILTER,
                          initialTexParams.fMagFilter));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                          GR_GL_TEXTURE_MIN_FILTER,
                          initialTexParams.fMinFilter));
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
        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, 0));

        if (!this->createRenderTargetObjects(glTexDesc.fWidth,
                                             glTexDesc.fHeight,
                                             glTexDesc.fTextureID,
                                             &glRTDesc)) {
            GL_CALL(DeleteTextures(1, &glTexDesc.fTextureID));
            return return_null_texture();
        }
        tex = SkNEW_ARGS(GrGLTexture, (this, glTexDesc, glRTDesc));
    } else {
        tex = SkNEW_ARGS(GrGLTexture, (this, glTexDesc));
    }
    tex->setCachedTexParams(initialTexParams, this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    GrPrintf("--- new texture [%d] size=(%d %d) config=%d\n",
             glTexDesc.fTextureID, desc.fWidth, desc.fHeight, desc.fConfig);
#endif
    return tex;
}

namespace {

const GrGLuint kUnknownBitCount = GrGLStencilBuffer::kUnknownBitCount;

void inline get_stencil_rb_sizes(const GrGLInterface* gl,
                                 GrGLStencilBuffer::Format* format) {

    // we shouldn't ever know one size and not the other
    SkASSERT((kUnknownBitCount == format->fStencilBits) ==
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
    // SBs for a client's standalone RT (that is a RT that isn't also a texture).
    SkASSERT(rt->asTexture());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numSamples();
    GrGLuint sbID;
    GL_CALL(GenRenderbuffers(1, &sbID));
    if (!sbID) {
        return false;
    }

    int stencilFmtCnt = this->glCaps().stencilFormats().count();
    for (int i = 0; i < stencilFmtCnt; ++i) {
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, sbID));
        // we start with the last stencil format that succeeded in hopes
        // that we won't go through this loop more than once after the
        // first (painful) stencil creation.
        int sIdx = (i + fLastSuccessfulStencilFmtIdx) % stencilFmtCnt;
        const GrGLCaps::StencilFormat& sFmt =
                this->glCaps().stencilFormats()[sIdx];
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        // we do this "if" so that we don't call the multisample
        // version on a GL that doesn't have an MSAA extension.
        bool created;
        if (samples > 0) {
            created = renderbuffer_storage_msaa(fGLContext,
                                                samples,
                                                sFmt.fInternalFormat,
                                                width, height);
        } else {
            GL_ALLOC_CALL(this->glInterface(),
                          RenderbufferStorage(GR_GL_RENDERBUFFER,
                                              sFmt.fInternalFormat,
                                              width, height));
            created =
                (GR_GL_NO_ERROR == check_alloc_error(rt->desc(), this->glInterface()));
        }
        if (created) {
            // After sized formats we attempt an unsized format and take
            // whatever sizes GL gives us. In that case we query for the size.
            GrGLStencilBuffer::Format format = sFmt;
            get_stencil_rb_sizes(this->glInterface(), &format);
            static const bool kIsWrapped = false;
            SkAutoTUnref<GrStencilBuffer> sb(SkNEW_ARGS(GrGLStencilBuffer,
                                                  (this, kIsWrapped, sbID, width, height,
                                                  samples, format)));
            if (this->attachStencilBufferToRenderTarget(sb, rt)) {
                fLastSuccessfulStencilFmtIdx = sIdx;
                sb->transferToCache();
                rt->setStencilBuffer(sb);
                return true;
           }
           sb->abandon(); // otherwise we lose sbID
        }
    }
    GL_CALL(DeleteRenderbuffers(1, &sbID));
    return false;
}

bool GrGpuGL::attachStencilBufferToRenderTarget(GrStencilBuffer* sb, GrRenderTarget* rt) {
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
#ifdef SK_DEBUG
            GrGLenum status;
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            SkASSERT(GR_GL_FRAMEBUFFER_COMPLETE == status);
#endif
        }
        return true;
    } else {
        GrGLStencilBuffer* glsb = static_cast<GrGLStencilBuffer*>(sb);
        GrGLuint rb = glsb->renderbufferID();

        fHWBoundRenderTarget = NULL;
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
        if (!this->glCaps().isColorConfigAndStencilFormatVerified(rt->config(), glsb->format())) {
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
                fGLContext.info().caps()->markColorConfigAndStencilFormatAsVerified(
                    rt->config(),
                    glsb->format());
            }
        }
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBuffer* GrGpuGL::onCreateVertexBuffer(size_t size, bool dynamic) {
    GrGLVertexBuffer::Desc desc;
    desc.fDynamic = dynamic;
    desc.fSizeInBytes = size;
    desc.fIsWrapped = false;

    if (this->glCaps().useNonVBOVertexAndIndexDynamicData() && desc.fDynamic) {
        desc.fID = 0;
        GrGLVertexBuffer* vertexBuffer = SkNEW_ARGS(GrGLVertexBuffer, (this, desc));
        return vertexBuffer;
    } else {
        GL_CALL(GenBuffers(1, &desc.fID));
        if (desc.fID) {
            fHWGeometryState.setVertexBufferID(this, desc.fID);
            CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
            // make sure driver can allocate memory for this buffer
            GL_ALLOC_CALL(this->glInterface(),
                          BufferData(GR_GL_ARRAY_BUFFER,
                                     desc.fSizeInBytes,
                                     NULL,   // data ptr
                                     desc.fDynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
            if (CHECK_ALLOC_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
                GL_CALL(DeleteBuffers(1, &desc.fID));
                this->notifyVertexBufferDelete(desc.fID);
                return NULL;
            }
            GrGLVertexBuffer* vertexBuffer = SkNEW_ARGS(GrGLVertexBuffer, (this, desc));
            return vertexBuffer;
        }
        return NULL;
    }
}

GrIndexBuffer* GrGpuGL::onCreateIndexBuffer(size_t size, bool dynamic) {
    GrGLIndexBuffer::Desc desc;
    desc.fDynamic = dynamic;
    desc.fSizeInBytes = size;
    desc.fIsWrapped = false;

    if (this->glCaps().useNonVBOVertexAndIndexDynamicData() && desc.fDynamic) {
        desc.fID = 0;
        GrIndexBuffer* indexBuffer = SkNEW_ARGS(GrGLIndexBuffer, (this, desc));
        return indexBuffer;
    } else {
        GL_CALL(GenBuffers(1, &desc.fID));
        if (desc.fID) {
            fHWGeometryState.setIndexBufferIDOnDefaultVertexArray(this, desc.fID);
            CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
            // make sure driver can allocate memory for this buffer
            GL_ALLOC_CALL(this->glInterface(),
                          BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                                     desc.fSizeInBytes,
                                     NULL,  // data ptr
                                     desc.fDynamic ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW));
            if (CHECK_ALLOC_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
                GL_CALL(DeleteBuffers(1, &desc.fID));
                this->notifyIndexBufferDelete(desc.fID);
                return NULL;
            }
            GrIndexBuffer* indexBuffer = SkNEW_ARGS(GrGLIndexBuffer, (this, desc));
            return indexBuffer;
        }
        return NULL;
    }
}

GrPath* GrGpuGL::onCreatePath(const SkPath& inPath, const SkStrokeRec& stroke) {
    SkASSERT(this->caps()->pathRenderingSupport());
    return SkNEW_ARGS(GrGLPath, (this, inPath, stroke));
}

void GrGpuGL::flushScissor() {
    if (fScissorState.fEnabled) {
        // Only access the RT if scissoring is being enabled. We can call this before performing
        // a glBitframebuffer for a surface->surface copy, which requires no RT to be bound to the
        // GrDrawState.
        const GrDrawState& drawState = this->getDrawState();
        const GrGLRenderTarget* rt =
            static_cast<const GrGLRenderTarget*>(drawState.getRenderTarget());

        SkASSERT(NULL != rt);
        const GrGLIRect& vp = rt->getViewport();
        GrGLIRect scissor;
        scissor.setRelativeTo(vp,
                              fScissorState.fRect.fLeft,
                              fScissorState.fRect.fTop,
                              fScissorState.fRect.width(),
                              fScissorState.fRect.height(),
                              rt->origin());
        // if the scissor fully contains the viewport then we fall through and
        // disable the scissor test.
        if (!scissor.contains(vp)) {
            if (fHWScissorSettings.fRect != scissor) {
                scissor.pushToGLScissor(this->glInterface());
                fHWScissorSettings.fRect = scissor;
            }
            if (kYes_TriState != fHWScissorSettings.fEnabled) {
                GL_CALL(Enable(GR_GL_SCISSOR_TEST));
                fHWScissorSettings.fEnabled = kYes_TriState;
            }
            return;
        }
    }
    if (kNo_TriState != fHWScissorSettings.fEnabled) {
        GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        fHWScissorSettings.fEnabled = kNo_TriState;
        return;
    }
}

void GrGpuGL::onClear(const SkIRect* rect, GrColor color) {
    const GrDrawState& drawState = this->getDrawState();
    const GrRenderTarget* rt = drawState.getRenderTarget();
    // parent class should never let us get here with no RT
    SkASSERT(NULL != rt);

    SkIRect clippedRect;
    if (NULL != rect) {
        // flushScissor expects rect to be clipped to the target.
        clippedRect = *rect;
        SkIRect rtRect = SkIRect::MakeWH(rt->width(), rt->height());
        if (clippedRect.intersect(rtRect)) {
            rect = &clippedRect;
        } else {
            return;
        }
    }
    this->flushRenderTarget(rect);
    GrAutoTRestore<ScissorState> asr(&fScissorState);
    fScissorState.fEnabled = (NULL != rect);
    if (fScissorState.fEnabled) {
        fScissorState.fRect = *rect;
    }
    this->flushScissor();

    GrGLfloat r, g, b, a;
    static const GrGLfloat scale255 = 1.f / 255.f;
    a = GrColorUnpackA(color) * scale255;
    GrGLfloat scaleRGB = scale255;
    r = GrColorUnpackR(color) * scaleRGB;
    g = GrColorUnpackG(color) * scaleRGB;
    b = GrColorUnpackB(color) * scaleRGB;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWWriteToColor = kYes_TriState;
    GL_CALL(ClearColor(r, g, b, a));
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGpuGL::clearStencil() {
    if (NULL == this->getDrawState().getRenderTarget()) {
        return;
    }

    this->flushRenderTarget(&SkIRect::EmptyIRect());

    GrAutoTRestore<ScissorState> asr(&fScissorState);
    fScissorState.fEnabled = false;
    this->flushScissor();

    GL_CALL(StencilMask(0xffffffff));
    GL_CALL(ClearStencil(0));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

void GrGpuGL::clearStencilClip(const SkIRect& rect, bool insideClip) {
    const GrDrawState& drawState = this->getDrawState();
    const GrRenderTarget* rt = drawState.getRenderTarget();
    SkASSERT(NULL != rt);

    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(NULL != rt->getStencilBuffer());
    GrGLint stencilBitCount =  rt->getStencilBuffer()->bits();
#if 0
    SkASSERT(stencilBitCount > 0);
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
    this->flushRenderTarget(&SkIRect::EmptyIRect());

    GrAutoTRestore<ScissorState> asr(&fScissorState);
    fScissorState.fEnabled = true;
    fScissorState.fRect = rect;
    this->flushScissor();

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

void GrGpuGL::onForceRenderTargetFlush() {
    this->flushRenderTarget(&SkIRect::EmptyIRect());
}

bool GrGpuGL::readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                        int left, int top,
                                        int width, int height,
                                        GrPixelConfig config,
                                        size_t rowBytes) const {
    // If this rendertarget is aready TopLeft, we don't need to flip.
    if (kTopLeft_GrSurfaceOrigin == renderTarget->origin()) {
        return false;
    }

    // if GL can do the flip then we'll never pay for it.
    if (this->glCaps().packFlipYSupport()) {
        return false;
    }

    // If we have to do memcpy to handle non-trim rowBytes then we
    // get the flip for free. Otherwise it costs.
    if (this->glCaps().packRowLengthSupport()) {
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
                           size_t rowBytes) {
    GrGLenum format;
    GrGLenum type;
    bool flipY = kBottomLeft_GrSurfaceOrigin == target->origin();
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
            this->flushRenderTarget(&SkIRect::EmptyIRect());
            break;
        case GrGLRenderTarget::kCanResolve_ResolveType:
            this->onResolveRenderTarget(tgt);
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
    readRect.setRelativeTo(glvp, left, top, width, height, target->origin());

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
        if (this->glCaps().packRowLengthSupport()) {
            SkASSERT(!(rowBytes % sizeof(GrColor)));
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, rowBytes / sizeof(GrColor)));
            readDstRowBytes = rowBytes;
        } else {
            scratch.reset(tightRowBytes * height);
            readDst = scratch.get();
        }
    }
    if (flipY && this->glCaps().packFlipYSupport()) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 1));
    }
    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom,
                       readRect.fWidth, readRect.fHeight,
                       format, type, readDst));
    if (readDstRowBytes != tightRowBytes) {
        SkASSERT(this->glCaps().packRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }
    if (flipY && this->glCaps().packFlipYSupport()) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 0));
        flipY = false;
    }

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom. We must preserve the padding contents. Note
    // that the above readPixels did not overwrite the padding.
    if (readDst == buffer) {
        SkASSERT(rowBytes == readDstRowBytes);
        if (flipY) {
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
        SkASSERT(readDst != buffer);        SkASSERT(rowBytes != tightRowBytes);
        // copy from readDst to buffer while flipping y
        // const int halfY = height >> 1;
        const char* src = reinterpret_cast<const char*>(readDst);
        char* dst = reinterpret_cast<char*>(buffer);
        if (flipY) {
            dst += (height-1) * rowBytes;
        }
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, tightRowBytes);
            src += readDstRowBytes;
            if (!flipY) {
                dst += rowBytes;
            } else {
                dst -= rowBytes;
            }
        }
    }
    return true;
}

void GrGpuGL::flushRenderTarget(const SkIRect* bound) {

    GrGLRenderTarget* rt =
        static_cast<GrGLRenderTarget*>(this->drawState()->getRenderTarget());
    SkASSERT(NULL != rt);

    if (fHWBoundRenderTarget != rt) {
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, rt->renderFBOID()));
#ifdef SK_DEBUG
        GrGLenum status;
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            GrPrintf("GrGpuGL::flushRenderTarget glCheckFramebufferStatus %x\n", status);
        }
#endif
        fHWBoundRenderTarget = rt;
        const GrGLIRect& vp = rt->getViewport();
        if (fHWViewport != vp) {
            vp.pushToGLViewport(this->glInterface());
            fHWViewport = vp;
        }
    }
    if (NULL == bound || !bound->isEmpty()) {
        rt->flagAsNeedingResolve(bound);
    }

    GrTexture *texture = rt->asTexture();
    if (texture) {
        texture->dirtyMipMaps(true);
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
    #if defined(SK_BUILD_FOR_MAC)
        #include <AGL/agl.h>
    #elif defined(SK_BUILD_FOR_WIN32)
        #include <gl/GL.h>
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

void GrGpuGL::onGpuDraw(const DrawInfo& info) {
    size_t indexOffsetInBytes;
    this->setupGeometry(info, &indexOffsetInBytes);

    SkASSERT((size_t)info.primitiveType() < GR_ARRAY_COUNT(gPrimitiveType2GLMode));

    if (info.isIndexed()) {
        GrGLvoid* indices =
            reinterpret_cast<GrGLvoid*>(indexOffsetInBytes + sizeof(uint16_t) * info.startIndex());
        // info.startVertex() was accounted for by setupGeometry.
        GL_CALL(DrawElements(gPrimitiveType2GLMode[info.primitiveType()],
                             info.indexCount(),
                             GR_GL_UNSIGNED_SHORT,
                             indices));
    } else {
        // Pass 0 for parameter first. We have to adjust glVertexAttribPointer() to account for
        // startVertex in the DrawElements case. So we always rely on setupGeometry to have
        // accounted for startVertex.
        GL_CALL(DrawArrays(gPrimitiveType2GLMode[info.primitiveType()], 0, info.vertexCount()));
    }
#if SWAP_PER_DRAW
    glFlush();
    #if defined(SK_BUILD_FOR_MAC)
        aglSwapBuffers(aglGetCurrentContext());
        int set_a_break_pt_here = 9;
        aglSwapBuffers(aglGetCurrentContext());
    #elif defined(SK_BUILD_FOR_WIN32)
        SwapBuf();
        int set_a_break_pt_here = 9;
        SwapBuf();
    #endif
#endif
}

static GrGLenum gr_stencil_op_to_gl_path_rendering_fill_mode(GrStencilOp op) {
    switch (op) {
        default:
            GrCrash("Unexpected path fill.");
            /* fallthrough */;
        case kIncClamp_StencilOp:
            return GR_GL_COUNT_UP;
        case kInvert_StencilOp:
            return GR_GL_INVERT;
    }
}

void GrGpuGL::onGpuStencilPath(const GrPath* path, SkPath::FillType fill) {
    SkASSERT(this->caps()->pathRenderingSupport());

    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();
    SkASSERT(NULL != this->drawState()->getRenderTarget());
    SkASSERT(NULL != this->drawState()->getRenderTarget()->getStencilBuffer());

    flushPathStencilSettings(fill);

    // Decide how to manipulate the stencil buffer based on the fill rule.
    SkASSERT(!fHWPathStencilSettings.isTwoSided());

    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);
    GL_CALL(StencilFillPath(id, fillMode, writeMask));
}

void GrGpuGL::onGpuDrawPath(const GrPath* path, SkPath::FillType fill) {
    SkASSERT(this->caps()->pathRenderingSupport());

    GrGLuint id = static_cast<const GrGLPath*>(path)->pathID();
    SkASSERT(NULL != this->drawState()->getRenderTarget());
    SkASSERT(NULL != this->drawState()->getRenderTarget()->getStencilBuffer());
    SkASSERT(!fCurrentProgram->hasVertexShader());

    flushPathStencilSettings(fill);
    const SkStrokeRec& stroke = path->getStroke();

    SkPath::FillType nonInvertedFill = SkPath::ConvertToNonInverseFillType(fill);
    SkASSERT(!fHWPathStencilSettings.isTwoSided());
    GrGLenum fillMode =
        gr_stencil_op_to_gl_path_rendering_fill_mode(fHWPathStencilSettings.passOp(GrStencilSettings::kFront_Face));
    GrGLint writeMask = fHWPathStencilSettings.writeMask(GrStencilSettings::kFront_Face);

    if (stroke.isFillStyle() || SkStrokeRec::kStrokeAndFill_Style == stroke.getStyle()) {
        GL_CALL(StencilFillPath(id, fillMode, writeMask));
    }
    if (stroke.needToApply()) {
        GL_CALL(StencilStrokePath(id, 0xffff, writeMask));
    }

    if (nonInvertedFill == fill) {
        if (stroke.needToApply()) {
            GL_CALL(CoverStrokePath(id, GR_GL_BOUNDING_BOX));
        } else {
            GL_CALL(CoverFillPath(id, GR_GL_BOUNDING_BOX));
        }
    } else {
        GrDrawState* drawState = this->drawState();
        GrDrawState::AutoViewMatrixRestore avmr;
        SkRect bounds = SkRect::MakeLTRB(0, 0,
                                         SkIntToScalar(drawState->getRenderTarget()->width()),
                                         SkIntToScalar(drawState->getRenderTarget()->height()));
        SkMatrix vmi;
        // mapRect through persp matrix may not be correct
        if (!drawState->getViewMatrix().hasPerspective() && drawState->getViewInverse(&vmi)) {
            vmi.mapRect(&bounds);
            // theoretically could set bloat = 0, instead leave it because of matrix inversion
            // precision.
            SkScalar bloat = drawState->getViewMatrix().getMaxStretch() * SK_ScalarHalf;
            bounds.outset(bloat, bloat);
        } else {
            avmr.setIdentity(drawState);
        }

        this->drawSimpleRect(bounds, NULL);
    }
}

void GrGpuGL::onResolveRenderTarget(GrRenderTarget* target) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(target);
    if (rt->needsResolve()) {
        // Some extensions automatically resolves the texture when it is read.
        if (this->glCaps().usesMSAARenderBuffers()) {
            SkASSERT(rt->textureFBOID() != rt->renderFBOID());
            GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, rt->renderFBOID()));
            GL_CALL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, rt->textureFBOID()));
            // make sure we go through flushRenderTarget() since we've modified
            // the bound DRAW FBO ID.
            fHWBoundRenderTarget = NULL;
            const GrGLIRect& vp = rt->getViewport();
            const SkIRect dirtyRect = rt->getResolveRect();
            GrGLIRect r;
            r.setRelativeTo(vp, dirtyRect.fLeft, dirtyRect.fTop,
                            dirtyRect.width(), dirtyRect.height(), target->origin());

            GrAutoTRestore<ScissorState> asr;
            if (GrGLCaps::kES_Apple_MSFBOType == this->glCaps().msFBOType()) {
                // Apple's extension uses the scissor as the blit bounds.
                asr.reset(&fScissorState);
                fScissorState.fEnabled = true;
                fScissorState.fRect = dirtyRect;
                this->flushScissor();
                GL_CALL(ResolveMultisampleFramebuffer());
            } else {
                if (GrGLCaps::kDesktop_EXT_MSFBOType == this->glCaps().msFBOType()) {
                    // this respects the scissor during the blit, so disable it.
                    asr.reset(&fScissorState);
                    fScissorState.fEnabled = false;
                    this->flushScissor();
                }
                int right = r.fLeft + r.fWidth;
                int top = r.fBottom + r.fHeight;
                GL_CALL(BlitFramebuffer(r.fLeft, r.fBottom, right, top,
                                        r.fLeft, r.fBottom, right, top,
                                        GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
            }
        }
        rt->flagAsResolved();
    }
}

namespace {

GrGLenum gr_to_gl_stencil_func(GrStencilFunc basicFunc) {
    static const GrGLenum gTable[] = {
        GR_GL_ALWAYS,           // kAlways_StencilFunc
        GR_GL_NEVER,            // kNever_StencilFunc
        GR_GL_GREATER,          // kGreater_StencilFunc
        GR_GL_GEQUAL,           // kGEqual_StencilFunc
        GR_GL_LESS,             // kLess_StencilFunc
        GR_GL_LEQUAL,           // kLEqual_StencilFunc,
        GR_GL_EQUAL,            // kEqual_StencilFunc,
        GR_GL_NOTEQUAL,         // kNotEqual_StencilFunc,
    };
    GR_STATIC_ASSERT(GR_ARRAY_COUNT(gTable) == kBasicStencilFuncCount);
    GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
    GR_STATIC_ASSERT(1 == kNever_StencilFunc);
    GR_STATIC_ASSERT(2 == kGreater_StencilFunc);
    GR_STATIC_ASSERT(3 == kGEqual_StencilFunc);
    GR_STATIC_ASSERT(4 == kLess_StencilFunc);
    GR_STATIC_ASSERT(5 == kLEqual_StencilFunc);
    GR_STATIC_ASSERT(6 == kEqual_StencilFunc);
    GR_STATIC_ASSERT(7 == kNotEqual_StencilFunc);
    SkASSERT((unsigned) basicFunc < kBasicStencilFuncCount);

    return gTable[basicFunc];
}

GrGLenum gr_to_gl_stencil_op(GrStencilOp op) {
    static const GrGLenum gTable[] = {
        GR_GL_KEEP,        // kKeep_StencilOp
        GR_GL_REPLACE,     // kReplace_StencilOp
        GR_GL_INCR_WRAP,   // kIncWrap_StencilOp
        GR_GL_INCR,        // kIncClamp_StencilOp
        GR_GL_DECR_WRAP,   // kDecWrap_StencilOp
        GR_GL_DECR,        // kDecClamp_StencilOp
        GR_GL_ZERO,        // kZero_StencilOp
        GR_GL_INVERT,      // kInvert_StencilOp
    };
    GR_STATIC_ASSERT(GR_ARRAY_COUNT(gTable) == kStencilOpCount);
    GR_STATIC_ASSERT(0 == kKeep_StencilOp);
    GR_STATIC_ASSERT(1 == kReplace_StencilOp);
    GR_STATIC_ASSERT(2 == kIncWrap_StencilOp);
    GR_STATIC_ASSERT(3 == kIncClamp_StencilOp);
    GR_STATIC_ASSERT(4 == kDecWrap_StencilOp);
    GR_STATIC_ASSERT(5 == kDecClamp_StencilOp);
    GR_STATIC_ASSERT(6 == kZero_StencilOp);
    GR_STATIC_ASSERT(7 == kInvert_StencilOp);
    SkASSERT((unsigned) op < kStencilOpCount);
    return gTable[op];
}

void set_gl_stencil(const GrGLInterface* gl,
                    const GrStencilSettings& settings,
                    GrGLenum glFace,
                    GrStencilSettings::Face grFace) {
    GrGLenum glFunc = gr_to_gl_stencil_func(settings.func(grFace));
    GrGLenum glFailOp = gr_to_gl_stencil_op(settings.failOp(grFace));
    GrGLenum glPassOp = gr_to_gl_stencil_op(settings.passOp(grFace));

    GrGLint ref = settings.funcRef(grFace);
    GrGLint mask = settings.funcMask(grFace);
    GrGLint writeMask = settings.writeMask(grFace);

    if (GR_GL_FRONT_AND_BACK == glFace) {
        // we call the combined func just in case separate stencil is not
        // supported.
        GR_GL_CALL(gl, StencilFunc(glFunc, ref, mask));
        GR_GL_CALL(gl, StencilMask(writeMask));
        GR_GL_CALL(gl, StencilOp(glFailOp, glPassOp, glPassOp));
    } else {
        GR_GL_CALL(gl, StencilFuncSeparate(glFace, glFunc, ref, mask));
        GR_GL_CALL(gl, StencilMaskSeparate(glFace, writeMask));
        GR_GL_CALL(gl, StencilOpSeparate(glFace, glFailOp, glPassOp, glPassOp));
    }
}
}

void GrGpuGL::flushStencil(DrawType type) {
    if (kStencilPath_DrawType != type && fHWStencilSettings != fStencilSettings) {
        if (fStencilSettings.isDisabled()) {
            if (kNo_TriState != fHWStencilTestEnabled) {
                GL_CALL(Disable(GR_GL_STENCIL_TEST));
                fHWStencilTestEnabled = kNo_TriState;
            }
        } else {
            if (kYes_TriState != fHWStencilTestEnabled) {
                GL_CALL(Enable(GR_GL_STENCIL_TEST));
                fHWStencilTestEnabled = kYes_TriState;
            }
        }
        if (!fStencilSettings.isDisabled()) {
            if (this->caps()->twoSidedStencilSupport()) {
                set_gl_stencil(this->glInterface(),
                               fStencilSettings,
                               GR_GL_FRONT,
                               GrStencilSettings::kFront_Face);
                set_gl_stencil(this->glInterface(),
                               fStencilSettings,
                               GR_GL_BACK,
                               GrStencilSettings::kBack_Face);
            } else {
                set_gl_stencil(this->glInterface(),
                               fStencilSettings,
                               GR_GL_FRONT_AND_BACK,
                               GrStencilSettings::kFront_Face);
            }
        }
        fHWStencilSettings = fStencilSettings;
    }
}

void GrGpuGL::flushAAState(DrawType type) {
// At least some ATI linux drivers will render GL_LINES incorrectly when MSAA state is enabled but
// the target is not multisampled. Single pixel wide lines are rendered thicker than 1 pixel wide.
#if 0
    // Replace RT_HAS_MSAA with this definition once this driver bug is no longer a relevant concern
    #define RT_HAS_MSAA rt->isMultisampled()
#else
    #define RT_HAS_MSAA (rt->isMultisampled() || kDrawLines_DrawType == type)
#endif

    const GrRenderTarget* rt = this->getDrawState().getRenderTarget();
    if (kDesktop_GrGLBinding == this->glBinding()) {
        // ES doesn't support toggling GL_MULTISAMPLE and doesn't have
        // smooth lines.
        // we prefer smooth lines over multisampled lines
        bool smoothLines = false;

        if (kDrawLines_DrawType == type) {
            smoothLines = this->willUseHWAALines();
            if (smoothLines) {
                if (kYes_TriState != fHWAAState.fSmoothLineEnabled) {
                    GL_CALL(Enable(GR_GL_LINE_SMOOTH));
                    fHWAAState.fSmoothLineEnabled = kYes_TriState;
                    // must disable msaa to use line smoothing
                    if (RT_HAS_MSAA &&
                        kNo_TriState != fHWAAState.fMSAAEnabled) {
                        GL_CALL(Disable(GR_GL_MULTISAMPLE));
                        fHWAAState.fMSAAEnabled = kNo_TriState;
                    }
                }
            } else {
                if (kNo_TriState != fHWAAState.fSmoothLineEnabled) {
                    GL_CALL(Disable(GR_GL_LINE_SMOOTH));
                    fHWAAState.fSmoothLineEnabled = kNo_TriState;
                }
            }
        }
        if (!smoothLines && RT_HAS_MSAA) {
            // FIXME: GL_NV_pr doesn't seem to like MSAA disabled. The paths
            // convex hulls of each segment appear to get filled.
            bool enableMSAA = kStencilPath_DrawType == type ||
                              this->getDrawState().isHWAntialiasState();
            if (enableMSAA) {
                if (kYes_TriState != fHWAAState.fMSAAEnabled) {
                    GL_CALL(Enable(GR_GL_MULTISAMPLE));
                    fHWAAState.fMSAAEnabled = kYes_TriState;
                }
            } else {
                if (kNo_TriState != fHWAAState.fMSAAEnabled) {
                    GL_CALL(Disable(GR_GL_MULTISAMPLE));
                    fHWAAState.fMSAAEnabled = kNo_TriState;
                }
            }
        }
    }
}

void GrGpuGL::flushPathStencilSettings(SkPath::FillType fill) {
    GrStencilSettings pathStencilSettings;
    this->getPathStencilSettingsForFillType(fill, &pathStencilSettings);
    if (fHWPathStencilSettings != pathStencilSettings) {
        // Just the func, ref, and mask is set here. The op and write mask are params to the call
        // that draws the path to the SB (glStencilFillPath)
        GrGLenum func =
            gr_to_gl_stencil_func(pathStencilSettings.func(GrStencilSettings::kFront_Face));
        GL_CALL(PathStencilFunc(func,
                                pathStencilSettings.funcRef(GrStencilSettings::kFront_Face),
                                pathStencilSettings.funcMask(GrStencilSettings::kFront_Face)));

        fHWPathStencilSettings = pathStencilSettings;
    }
}

void GrGpuGL::flushBlend(bool isLines,
                         GrBlendCoeff srcCoeff,
                         GrBlendCoeff dstCoeff) {
    if (isLines && this->willUseHWAALines()) {
        if (kYes_TriState != fHWBlendState.fEnabled) {
            GL_CALL(Enable(GR_GL_BLEND));
            fHWBlendState.fEnabled = kYes_TriState;
        }
        if (kSA_GrBlendCoeff != fHWBlendState.fSrcCoeff ||
            kISA_GrBlendCoeff != fHWBlendState.fDstCoeff) {
            GL_CALL(BlendFunc(gXfermodeCoeff2Blend[kSA_GrBlendCoeff],
                              gXfermodeCoeff2Blend[kISA_GrBlendCoeff]));
            fHWBlendState.fSrcCoeff = kSA_GrBlendCoeff;
            fHWBlendState.fDstCoeff = kISA_GrBlendCoeff;
        }
    } else {
        // any optimization to disable blending should
        // have already been applied and tweaked the coeffs
        // to (1, 0).
        bool blendOff = kOne_GrBlendCoeff == srcCoeff &&
                        kZero_GrBlendCoeff == dstCoeff;
        if (blendOff) {
            if (kNo_TriState != fHWBlendState.fEnabled) {
                GL_CALL(Disable(GR_GL_BLEND));
                fHWBlendState.fEnabled = kNo_TriState;
            }
        } else {
            if (kYes_TriState != fHWBlendState.fEnabled) {
                GL_CALL(Enable(GR_GL_BLEND));
                fHWBlendState.fEnabled = kYes_TriState;
            }
            if (fHWBlendState.fSrcCoeff != srcCoeff ||
                fHWBlendState.fDstCoeff != dstCoeff) {
                GL_CALL(BlendFunc(gXfermodeCoeff2Blend[srcCoeff],
                                  gXfermodeCoeff2Blend[dstCoeff]));
                fHWBlendState.fSrcCoeff = srcCoeff;
                fHWBlendState.fDstCoeff = dstCoeff;
            }
            GrColor blendConst = this->getDrawState().getBlendConstant();
            if ((BlendCoeffReferencesConstant(srcCoeff) ||
                 BlendCoeffReferencesConstant(dstCoeff)) &&
                (!fHWBlendState.fConstColorValid ||
                 fHWBlendState.fConstColor != blendConst)) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(blendConst, c);
                GL_CALL(BlendColor(c[0], c[1], c[2], c[3]));
                fHWBlendState.fConstColor = blendConst;
                fHWBlendState.fConstColorValid = true;
            }
        }
    }
}

static inline GrGLenum tile_to_gl_wrap(SkShader::TileMode tm) {
    static const GrGLenum gWrapModes[] = {
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_REPEAT,
        GR_GL_MIRRORED_REPEAT
    };
    GR_STATIC_ASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gWrapModes));
    GR_STATIC_ASSERT(0 == SkShader::kClamp_TileMode);
    GR_STATIC_ASSERT(1 == SkShader::kRepeat_TileMode);
    GR_STATIC_ASSERT(2 == SkShader::kMirror_TileMode);
    return gWrapModes[tm];
}

void GrGpuGL::bindTexture(int unitIdx, const GrTextureParams& params, GrGLTexture* texture) {
    SkASSERT(NULL != texture);

    // If we created a rt/tex and rendered to it without using a texture and now we're texturing
    // from the rt it will still be the last bound texture, but it needs resolving. So keep this
    // out of the "last != next" check.
    GrGLRenderTarget* texRT =  static_cast<GrGLRenderTarget*>(texture->asRenderTarget());
    if (NULL != texRT) {
        this->onResolveRenderTarget(texRT);
    }

    if (fHWBoundTextures[unitIdx] != texture) {
        this->setTextureUnit(unitIdx);
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, texture->textureID()));
        fHWBoundTextures[unitIdx] = texture;
    }

    ResetTimestamp timestamp;
    const GrGLTexture::TexParams& oldTexParams = texture->getCachedTexParams(&timestamp);
    bool setAll = timestamp < this->getResetTimestamp();
    GrGLTexture::TexParams newTexParams;

    static GrGLenum glMinFilterModes[] = {
        GR_GL_NEAREST,
        GR_GL_LINEAR,
        GR_GL_LINEAR_MIPMAP_LINEAR
    };
    static GrGLenum glMagFilterModes[] = {
        GR_GL_NEAREST,
        GR_GL_LINEAR,
        GR_GL_LINEAR
    };
    newTexParams.fMinFilter = glMinFilterModes[params.filterMode()];
    newTexParams.fMagFilter = glMagFilterModes[params.filterMode()];

#ifndef SKIA_IGNORE_GPU_MIPMAPS
    if (params.filterMode() == GrTextureParams::kMipMap_FilterMode &&
        texture->mipMapsAreDirty()) {
//        GL_CALL(Hint(GR_GL_GENERATE_MIPMAP_HINT,GR_GL_NICEST));
        GL_CALL(GenerateMipmap(GR_GL_TEXTURE_2D));
        texture->dirtyMipMaps(false);
    }
#endif

    newTexParams.fWrapS = tile_to_gl_wrap(params.getTileModeX());
    newTexParams.fWrapT = tile_to_gl_wrap(params.getTileModeY());
    memcpy(newTexParams.fSwizzleRGBA,
           GrGLShaderBuilder::GetTexParamSwizzle(texture->config(), this->glCaps()),
           sizeof(newTexParams.fSwizzleRGBA));
    if (setAll || newTexParams.fMagFilter != oldTexParams.fMagFilter) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_MAG_FILTER,
                              newTexParams.fMagFilter));
    }
    if (setAll || newTexParams.fMinFilter != oldTexParams.fMinFilter) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_MIN_FILTER,
                              newTexParams.fMinFilter));
    }
    if (setAll || newTexParams.fWrapS != oldTexParams.fWrapS) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_WRAP_S,
                              newTexParams.fWrapS));
    }
    if (setAll || newTexParams.fWrapT != oldTexParams.fWrapT) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_WRAP_T,
                              newTexParams.fWrapT));
    }
    if (this->glCaps().textureSwizzleSupport() &&
        (setAll || memcmp(newTexParams.fSwizzleRGBA,
                          oldTexParams.fSwizzleRGBA,
                          sizeof(newTexParams.fSwizzleRGBA)))) {
        this->setTextureUnit(unitIdx);
        if (this->glBinding() == kES_GrGLBinding) {
            // ES3 added swizzle support but not GL_TEXTURE_SWIZZLE_RGBA.
            const GrGLenum* swizzle = newTexParams.fSwizzleRGBA;
            GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_SWIZZLE_R, swizzle[0]));
            GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_SWIZZLE_G, swizzle[1]));
            GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_SWIZZLE_B, swizzle[2]));
            GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_SWIZZLE_A, swizzle[3]));
        } else {
            GR_STATIC_ASSERT(sizeof(newTexParams.fSwizzleRGBA[0]) == sizeof(GrGLint));
            const GrGLint* swizzle = reinterpret_cast<const GrGLint*>(newTexParams.fSwizzleRGBA);
            GL_CALL(TexParameteriv(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_SWIZZLE_RGBA, swizzle));
        }
    }
    texture->setCachedTexParams(newTexParams, this->getResetTimestamp());
}

void GrGpuGL::setProjectionMatrix(const SkMatrix& matrix,
                                  const SkISize& renderTargetSize,
                                  GrSurfaceOrigin renderTargetOrigin) {

    SkASSERT(this->glCaps().fixedFunctionSupport());

    if (renderTargetOrigin == fHWProjectionMatrixState.fRenderTargetOrigin &&
        renderTargetSize == fHWProjectionMatrixState.fRenderTargetSize &&
        matrix.cheapEqualTo(fHWProjectionMatrixState.fViewMatrix)) {
        return;
    }

    fHWProjectionMatrixState.fViewMatrix = matrix;
    fHWProjectionMatrixState.fRenderTargetSize = renderTargetSize;
    fHWProjectionMatrixState.fRenderTargetOrigin = renderTargetOrigin;

    GrGLfloat glMatrix[4 * 4];
    fHWProjectionMatrixState.getGLMatrix<4>(glMatrix);
    GL_CALL(MatrixMode(GR_GL_PROJECTION));
    GL_CALL(LoadMatrixf(glMatrix));
}

void GrGpuGL::enableTexGen(int unitIdx,
                           TexGenComponents components,
                           const GrGLfloat* coefficients) {

    SkASSERT(this->glCaps().fixedFunctionSupport());
    SkASSERT(this->caps()->pathRenderingSupport());
    SkASSERT(components >= kS_TexGenComponents && components <= kSTR_TexGenComponents);

    if (GR_GL_OBJECT_LINEAR == fHWTexGenSettings[unitIdx].fMode &&
        components == fHWTexGenSettings[unitIdx].fNumComponents &&
        !memcmp(coefficients, fHWTexGenSettings[unitIdx].fCoefficients,
                3 * components * sizeof(GrGLfloat))) {
        return;
    }

    this->setTextureUnit(unitIdx);

    if (GR_GL_OBJECT_LINEAR != fHWTexGenSettings[unitIdx].fMode) {
        for (int i = 0; i < 4; i++) {
            GL_CALL(TexGeni(GR_GL_S + i, GR_GL_TEXTURE_GEN_MODE, GR_GL_OBJECT_LINEAR));
        }
        fHWTexGenSettings[unitIdx].fMode = GR_GL_OBJECT_LINEAR;
    }

    for (int i = fHWTexGenSettings[unitIdx].fNumComponents; i < components; i++) {
        GL_CALL(Enable(GR_GL_TEXTURE_GEN_S + i));
    }
    for (int i = components; i < fHWTexGenSettings[unitIdx].fNumComponents; i++) {
        GL_CALL(Disable(GR_GL_TEXTURE_GEN_S + i));
    }
    fHWTexGenSettings[unitIdx].fNumComponents = components;

    for (int i = 0; i < components; i++) {
        GrGLfloat plane[] = {coefficients[0 + 3 * i],
                             coefficients[1 + 3 * i],
                             0,
                             coefficients[2 + 3 * i]};
        GL_CALL(TexGenfv(GR_GL_S + i, GR_GL_OBJECT_PLANE, plane));
    }

    GL_CALL(PathTexGen(GR_GL_TEXTURE0 + unitIdx,
                       GR_GL_OBJECT_LINEAR,
                       components,
                       coefficients));

    memcpy(fHWTexGenSettings[unitIdx].fCoefficients, coefficients,
           3 * components * sizeof(GrGLfloat));

    fHWActiveTexGenSets = SkTMax(fHWActiveTexGenSets, unitIdx);
}

void GrGpuGL::enableTexGen(int unitIdx, TexGenComponents components, const SkMatrix& matrix) {

    GrGLfloat coefficients[3 * 3];
    SkASSERT(this->glCaps().fixedFunctionSupport());
    SkASSERT(components >= kS_TexGenComponents && components <= kSTR_TexGenComponents);

    coefficients[0] = SkScalarToFloat(matrix[SkMatrix::kMScaleX]);
    coefficients[1] = SkScalarToFloat(matrix[SkMatrix::kMSkewX]);
    coefficients[2] = SkScalarToFloat(matrix[SkMatrix::kMTransX]);

    if (components >= kST_TexGenComponents) {
        coefficients[3] = SkScalarToFloat(matrix[SkMatrix::kMSkewY]);
        coefficients[4] = SkScalarToFloat(matrix[SkMatrix::kMScaleY]);
        coefficients[5] = SkScalarToFloat(matrix[SkMatrix::kMTransY]);
    }

    if (components >= kSTR_TexGenComponents) {
        coefficients[6] = SkScalarToFloat(matrix[SkMatrix::kMPersp0]);
        coefficients[7] = SkScalarToFloat(matrix[SkMatrix::kMPersp1]);
        coefficients[8] = SkScalarToFloat(matrix[SkMatrix::kMPersp2]);
    }

    enableTexGen(unitIdx, components, coefficients);
}

void GrGpuGL::disableUnusedTexGen(int numUsedTexCoordSets) {

    SkASSERT(this->glCaps().fixedFunctionSupport());

    for (int i = numUsedTexCoordSets; i < fHWActiveTexGenSets; i++) {
        if (0 == fHWTexGenSettings[i].fNumComponents) {
            continue;
        }

        this->setTextureUnit(i);
        for (int j = 0; j < fHWTexGenSettings[i].fNumComponents; j++) {
            GL_CALL(Disable(GR_GL_TEXTURE_GEN_S + j));
        }
        GL_CALL(PathTexGen(GR_GL_TEXTURE0 + i, GR_GL_NONE, 0, NULL));
        fHWTexGenSettings[i].fNumComponents = 0;
    }

    fHWActiveTexGenSets = SkTMin(fHWActiveTexGenSets, numUsedTexCoordSets);
}

void GrGpuGL::flushMiscFixedFunctionState() {

    const GrDrawState& drawState = this->getDrawState();

    if (drawState.isDitherState()) {
        if (kYes_TriState != fHWDitherEnabled) {
            GL_CALL(Enable(GR_GL_DITHER));
            fHWDitherEnabled = kYes_TriState;
        }
    } else {
        if (kNo_TriState != fHWDitherEnabled) {
            GL_CALL(Disable(GR_GL_DITHER));
            fHWDitherEnabled = kNo_TriState;
        }
    }

    if (drawState.isColorWriteDisabled()) {
        if (kNo_TriState != fHWWriteToColor) {
            GL_CALL(ColorMask(GR_GL_FALSE, GR_GL_FALSE,
                              GR_GL_FALSE, GR_GL_FALSE));
            fHWWriteToColor = kNo_TriState;
        }
    } else {
        if (kYes_TriState != fHWWriteToColor) {
            GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
            fHWWriteToColor = kYes_TriState;
        }
    }

    if (fHWDrawFace != drawState.getDrawFace()) {
        switch (this->getDrawState().getDrawFace()) {
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
        fHWDrawFace = drawState.getDrawFace();
    }
}

void GrGpuGL::notifyRenderTargetDelete(GrRenderTarget* renderTarget) {
    SkASSERT(NULL != renderTarget);
    if (fHWBoundRenderTarget == renderTarget) {
        fHWBoundRenderTarget = NULL;
    }
}

void GrGpuGL::notifyTextureDelete(GrGLTexture* texture) {
    for (int s = 0; s < fHWBoundTextures.count(); ++s) {
        if (fHWBoundTextures[s] == texture) {
            // deleting bound texture does implied bind to 0
            fHWBoundTextures[s] = NULL;
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
        case kRGBA_8888_GrPixelConfig:
            *internalFormat = GR_GL_RGBA;
            *externalFormat = GR_GL_RGBA;
            if (getSizedInternalFormat) {
                *internalFormat = GR_GL_RGBA8;
            } else {
                *internalFormat = GR_GL_RGBA;
            }
            *externalType = GR_GL_UNSIGNED_BYTE;
            break;
        case kBGRA_8888_GrPixelConfig:
            if (!this->glCaps().bgraFormatSupport()) {
                return false;
            }
            if (this->glCaps().bgraIsInternalFormat()) {
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
            if (this->caps()->eightBitPaletteSupport()) {
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
            if (this->glCaps().textureRedSupport()) {
                *internalFormat = GR_GL_RED;
                *externalFormat = GR_GL_RED;
                if (getSizedInternalFormat) {
                    *internalFormat = GR_GL_R8;
                } else {
                    *internalFormat = GR_GL_RED;
                }
                *externalType = GR_GL_UNSIGNED_BYTE;
            } else {
                *internalFormat = GR_GL_ALPHA;
                *externalFormat = GR_GL_ALPHA;
                if (getSizedInternalFormat) {
                    *internalFormat = GR_GL_ALPHA8;
                } else {
                    *internalFormat = GR_GL_ALPHA;
                }
                *externalType = GR_GL_UNSIGNED_BYTE;
            }
            break;
        default:
            return false;
    }
    return true;
}

void GrGpuGL::setTextureUnit(int unit) {
    SkASSERT(unit >= 0 && unit < fHWBoundTextures.count());
    if (unit != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + unit));
        fHWActiveTextureUnitIdx = unit;
    }
}

void GrGpuGL::setScratchTextureUnit() {
    // Bind the last texture unit since it is the least likely to be used by GrGLProgram.
    int lastUnitIdx = fHWBoundTextures.count() - 1;
    if (lastUnitIdx != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + lastUnitIdx));
        fHWActiveTextureUnitIdx = lastUnitIdx;
    }
    // clear out the this field so that if a program does use this unit it will rebind the correct
    // texture.
    fHWBoundTextures[lastUnitIdx] = NULL;
}

namespace {
// Determines whether glBlitFramebuffer could be used between src and dst.
inline bool can_blit_framebuffer(const GrSurface* dst,
                                 const GrSurface* src,
                                 const GrGpuGL* gpu,
                                 bool* wouldNeedTempFBO = NULL) {
    if (gpu->glCaps().isConfigRenderable(dst->config(), dst->desc().fSampleCnt > 0) &&
        gpu->glCaps().isConfigRenderable(src->config(), src->desc().fSampleCnt > 0) &&
        gpu->glCaps().usesMSAARenderBuffers()) {
        // ES3 doesn't allow framebuffer blits when the src has MSAA and the configs don't match
        // or the rects are not the same (not just the same size but have the same edges).
        if (GrGLCaps::kES_3_0_MSFBOType == gpu->glCaps().msFBOType() &&
            (src->desc().fSampleCnt > 0 || src->config() != dst->config())) {
           return false;
        }
        if (NULL != wouldNeedTempFBO) {
            *wouldNeedTempFBO = NULL == dst->asRenderTarget() || NULL == src->asRenderTarget();
        }
        return true;
    } else {
        return false;
    }
}

inline bool can_copy_texsubimage(const GrSurface* dst,
                                 const GrSurface* src,
                                 const GrGpuGL* gpu,
                                 bool* wouldNeedTempFBO = NULL) {
    // Table 3.9 of the ES2 spec indicates the supported formats with CopyTexSubImage
    // and BGRA isn't in the spec. There doesn't appear to be any extension that adds it. Perhaps
    // many drivers would allow it to work, but ANGLE does not.
    if (kES_GrGLBinding == gpu->glBinding() && gpu->glCaps().bgraIsInternalFormat() &&
        (kBGRA_8888_GrPixelConfig == dst->config() || kBGRA_8888_GrPixelConfig == src->config())) {
        return false;
    }
    const GrGLRenderTarget* dstRT = static_cast<const GrGLRenderTarget*>(dst->asRenderTarget());
    // If dst is multisampled (and uses an extension where there is a separate MSAA renderbuffer)
    // then we don't want to copy to the texture but to the MSAA buffer.
    if (NULL != dstRT && dstRT->renderFBOID() != dstRT->textureFBOID()) {
        return false;
    }
    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    // If the src is multisampled (and uses an extension where there is a separate MSAA
    // renderbuffer) then it is an invalid operation to call CopyTexSubImage
    if (NULL != srcRT && srcRT->renderFBOID() != srcRT->textureFBOID()) {
        return false;
    }
    if (gpu->glCaps().isConfigRenderable(src->config(), src->desc().fSampleCnt > 0) &&
        NULL != dst->asTexture() &&
        dst->origin() == src->origin() &&
        kIndex_8_GrPixelConfig != src->config()) {
        if (NULL != wouldNeedTempFBO) {
            *wouldNeedTempFBO = NULL == src->asRenderTarget();
        }
        return true;
    } else {
        return false;
    }
}

// If a temporary FBO was created, its non-zero ID is returned. The viewport that the copy rect is
// relative to is output.
inline GrGLuint bind_surface_as_fbo(const GrGLInterface* gl,
                                    GrSurface* surface,
                                    GrGLenum fboTarget,
                                    GrGLIRect* viewport) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    GrGLuint tempFBOID;
    if (NULL == rt) {
        SkASSERT(NULL != surface->asTexture());
        GrGLuint texID = static_cast<GrGLTexture*>(surface->asTexture())->textureID();
        GR_GL_CALL(gl, GenFramebuffers(1, &tempFBOID));
        GR_GL_CALL(gl, BindFramebuffer(fboTarget, tempFBOID));
        GR_GL_CALL(gl, FramebufferTexture2D(fboTarget,
                                            GR_GL_COLOR_ATTACHMENT0,
                                            GR_GL_TEXTURE_2D,
                                            texID,
                                            0));
        viewport->fLeft = 0;
        viewport->fBottom = 0;
        viewport->fWidth = surface->width();
        viewport->fHeight = surface->height();
    } else {
        tempFBOID = 0;
        GR_GL_CALL(gl, BindFramebuffer(fboTarget, rt->renderFBOID()));
        *viewport = rt->getViewport();
    }
    return tempFBOID;
}

}

void GrGpuGL::initCopySurfaceDstDesc(const GrSurface* src, GrTextureDesc* desc) {
    // Check for format issues with glCopyTexSubImage2D
    if (kES_GrGLBinding == this->glBinding() && this->glCaps().bgraIsInternalFormat() &&
        kBGRA_8888_GrPixelConfig == src->config()) {
        // glCopyTexSubImage2D doesn't work with this config. We'll want to make it a render target
        // in order to call glBlitFramebuffer or to copy to it by rendering.
        INHERITED::initCopySurfaceDstDesc(src, desc);
        return;
    } else if (NULL == src->asRenderTarget()) {
        // We don't want to have to create an FBO just to use glCopyTexSubImage2D. Let the base
        // class handle it by rendering.
        INHERITED::initCopySurfaceDstDesc(src, desc);
        return;
    }

    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    if (NULL != srcRT && srcRT->renderFBOID() != srcRT->textureFBOID()) {
        // It's illegal to call CopyTexSubImage2D on a MSAA renderbuffer.
        INHERITED::initCopySurfaceDstDesc(src, desc);
    } else {
        desc->fConfig = src->config();
        desc->fOrigin = src->origin();
        desc->fFlags = kNone_GrTextureFlags;
    }
}

bool GrGpuGL::onCopySurface(GrSurface* dst,
                            GrSurface* src,
                            const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    bool inheritedCouldCopy = INHERITED::onCanCopySurface(dst, src, srcRect, dstPoint);
    bool copied = false;
    bool wouldNeedTempFBO = false;
    if (can_copy_texsubimage(dst, src, this, &wouldNeedTempFBO) &&
        (!wouldNeedTempFBO || !inheritedCouldCopy)) {
        GrGLuint srcFBO;
        GrGLIRect srcVP;
        srcFBO = bind_surface_as_fbo(this->glInterface(), src, GR_GL_FRAMEBUFFER, &srcVP);
        GrGLTexture* dstTex = static_cast<GrGLTexture*>(dst->asTexture());
        SkASSERT(NULL != dstTex);
        // We modified the bound FBO
        fHWBoundRenderTarget = NULL;
        GrGLIRect srcGLRect;
        srcGLRect.setRelativeTo(srcVP,
                                srcRect.fLeft,
                                srcRect.fTop,
                                srcRect.width(),
                                srcRect.height(),
                                src->origin());

        this->setScratchTextureUnit();
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, dstTex->textureID()));
        GrGLint dstY;
        if (kBottomLeft_GrSurfaceOrigin == dst->origin()) {
            dstY = dst->height() - (dstPoint.fY + srcGLRect.fHeight);
        } else {
            dstY = dstPoint.fY;
        }
        GL_CALL(CopyTexSubImage2D(GR_GL_TEXTURE_2D, 0,
                                  dstPoint.fX, dstY,
                                  srcGLRect.fLeft, srcGLRect.fBottom,
                                  srcGLRect.fWidth, srcGLRect.fHeight));
        copied = true;
        if (srcFBO) {
            GL_CALL(DeleteFramebuffers(1, &srcFBO));
        }
    } else if (can_blit_framebuffer(dst, src, this, &wouldNeedTempFBO) &&
               (!wouldNeedTempFBO || !inheritedCouldCopy)) {
        SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                            srcRect.width(), srcRect.height());
        bool selfOverlap = false;
        if (dst->isSameAs(src)) {
            selfOverlap = SkIRect::IntersectsNoEmptyCheck(dstRect, srcRect);
        }

        if (!selfOverlap) {
            GrGLuint dstFBO;
            GrGLuint srcFBO;
            GrGLIRect dstVP;
            GrGLIRect srcVP;
            dstFBO = bind_surface_as_fbo(this->glInterface(), dst, GR_GL_DRAW_FRAMEBUFFER, &dstVP);
            srcFBO = bind_surface_as_fbo(this->glInterface(), src, GR_GL_READ_FRAMEBUFFER, &srcVP);
            // We modified the bound FBO
            fHWBoundRenderTarget = NULL;
            GrGLIRect srcGLRect;
            GrGLIRect dstGLRect;
            srcGLRect.setRelativeTo(srcVP,
                                    srcRect.fLeft,
                                    srcRect.fTop,
                                    srcRect.width(),
                                    srcRect.height(),
                                    src->origin());
            dstGLRect.setRelativeTo(dstVP,
                                    dstRect.fLeft,
                                    dstRect.fTop,
                                    dstRect.width(),
                                    dstRect.height(),
                                    dst->origin());

            GrAutoTRestore<ScissorState> asr;
            if (GrGLCaps::kDesktop_EXT_MSFBOType == this->glCaps().msFBOType()) {
                // The EXT version applies the scissor during the blit, so disable it.
                asr.reset(&fScissorState);
                fScissorState.fEnabled = false;
                this->flushScissor();
            }
            GrGLint srcY0;
            GrGLint srcY1;
            // Does the blit need to y-mirror or not?
            if (src->origin() == dst->origin()) {
                srcY0 = srcGLRect.fBottom;
                srcY1 = srcGLRect.fBottom + srcGLRect.fHeight;
            } else {
                srcY0 = srcGLRect.fBottom + srcGLRect.fHeight;
                srcY1 = srcGLRect.fBottom;
            }
            GL_CALL(BlitFramebuffer(srcGLRect.fLeft,
                                    srcY0,
                                    srcGLRect.fLeft + srcGLRect.fWidth,
                                    srcY1,
                                    dstGLRect.fLeft,
                                    dstGLRect.fBottom,
                                    dstGLRect.fLeft + dstGLRect.fWidth,
                                    dstGLRect.fBottom + dstGLRect.fHeight,
                                    GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
            if (dstFBO) {
                GL_CALL(DeleteFramebuffers(1, &dstFBO));
            }
            if (srcFBO) {
                GL_CALL(DeleteFramebuffers(1, &srcFBO));
            }
            copied = true;
        }
    }
    if (!copied && inheritedCouldCopy) {
        copied = INHERITED::onCopySurface(dst, src, srcRect, dstPoint);
        SkASSERT(copied);
    }
    return copied;
}

bool GrGpuGL::onCanCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) {
    // This mirrors the logic in onCopySurface.
    if (can_copy_texsubimage(dst, src, this)) {
        return true;
    }
    if (can_blit_framebuffer(dst, src, this)) {
        if (dst->isSameAs(src)) {
            SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                                srcRect.width(), srcRect.height());
            if(!SkIRect::IntersectsNoEmptyCheck(dstRect, srcRect)) {
                return true;
            }
        } else {
            return true;
        }
    }
    return INHERITED::onCanCopySurface(dst, src, srcRect, dstPoint);
}


///////////////////////////////////////////////////////////////////////////////

GrGLAttribArrayState* GrGpuGL::HWGeometryState::bindArrayAndBuffersToDraw(
                                                GrGpuGL* gpu,
                                                const GrGLVertexBuffer* vbuffer,
                                                const GrGLIndexBuffer* ibuffer) {
    SkASSERT(NULL != vbuffer);
    GrGLAttribArrayState* attribState;

    // We use a vertex array if we're on a core profile and the verts are in a VBO.
    if (gpu->glCaps().isCoreProfile() && !vbuffer->isCPUBacked()) {
        if (NULL == fVBOVertexArray || !fVBOVertexArray->isValid()) {
            SkSafeUnref(fVBOVertexArray);
            GrGLuint arrayID;
            GR_GL_CALL(gpu->glInterface(), GenVertexArrays(1, &arrayID));
            int attrCount = gpu->glCaps().maxVertexAttributes();
            fVBOVertexArray = SkNEW_ARGS(GrGLVertexArray, (gpu, arrayID, attrCount));
        }
        attribState = fVBOVertexArray->bindWithIndexBuffer(ibuffer);
    } else {
        if (NULL != ibuffer) {
            this->setIndexBufferIDOnDefaultVertexArray(gpu, ibuffer->bufferID());
        } else {
            this->setVertexArrayID(gpu, 0);
        }
        int attrCount = gpu->glCaps().maxVertexAttributes();
        if (fDefaultVertexArrayAttribState.count() != attrCount) {
            fDefaultVertexArrayAttribState.resize(attrCount);
        }
        attribState = &fDefaultVertexArrayAttribState;
    }
    return attribState;
}
