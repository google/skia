/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGpu_DEFINED
#define GrGLGpu_DEFINED

#include "GrGLContext.h"
#include "GrGLIRect.h"
#include "GrGLIndexBuffer.h"
#include "GrGLPathRendering.h"
#include "GrGLProgram.h"
#include "GrGLRenderTarget.h"
#include "GrGLStencilBuffer.h"
#include "GrGLTexture.h"
#include "GrGLVertexArray.h"
#include "GrGLVertexBuffer.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrXferProcessor.h"
#include "SkTypes.h"

class GrPipeline;

#ifdef SK_DEVELOPER
#define PROGRAM_CACHE_STATS
#endif

class GrGLGpu : public GrGpu {
public:
    GrGLGpu(const GrGLContext& ctx, GrContext* context);
    ~GrGLGpu() SK_OVERRIDE;

    void contextAbandoned() SK_OVERRIDE;

    const GrGLContext& glContext() const { return fGLContext; }

    const GrGLInterface* glInterface() const { return fGLContext.interface(); }
    const GrGLContextInfo& ctxInfo() const { return fGLContext; }
    GrGLStandard glStandard() const { return fGLContext.standard(); }
    GrGLVersion glVersion() const { return fGLContext.version(); }
    GrGLSLGeneration glslGeneration() const { return fGLContext.glslGeneration(); }
    const GrGLCaps& glCaps() const { return *fGLContext.caps(); }

    GrGLPathRendering* glPathRendering() {
        SkASSERT(glCaps().pathRenderingSupport());
        return static_cast<GrGLPathRendering*>(pathRendering());
    }

    void discard(GrRenderTarget*) SK_OVERRIDE;

    // Used by GrGLProgram and GrGLPathTexGenProgramEffects to configure OpenGL
    // state.
    void bindTexture(int unitIdx, const GrTextureParams& params, GrGLTexture* texture);

    // GrGpu overrides
    GrPixelConfig preferredReadPixelsConfig(GrPixelConfig readConfig,
                                            GrPixelConfig surfaceConfig) const SK_OVERRIDE;
    GrPixelConfig preferredWritePixelsConfig(GrPixelConfig writeConfig,
                                             GrPixelConfig surfaceConfig) const SK_OVERRIDE;
    bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const SK_OVERRIDE;
    bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                   int left, int top,
                                   int width, int height,
                                   GrPixelConfig config,
                                   size_t rowBytes) const SK_OVERRIDE;
    bool fullReadPixelsIsFasterThanPartial() const SK_OVERRIDE;

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) SK_OVERRIDE;

    // These functions should be used to bind GL objects. They track the GL state and skip redundant
    // bindings. Making the equivalent glBind calls directly will confuse the state tracking.
    void bindVertexArray(GrGLuint id) {
        fHWGeometryState.setVertexArrayID(this, id);
    }
    void bindIndexBufferAndDefaultVertexArray(GrGLuint id) {
        fHWGeometryState.setIndexBufferIDOnDefaultVertexArray(this, id);
    }
    void bindVertexBuffer(GrGLuint id) {
        fHWGeometryState.setVertexBufferID(this, id);
    }

    // These callbacks update state tracking when GL objects are deleted. They are called from
    // GrGLResource onRelease functions.
    void notifyVertexArrayDelete(GrGLuint id) {
        fHWGeometryState.notifyVertexArrayDelete(id);
    }
    void notifyVertexBufferDelete(GrGLuint id) {
        fHWGeometryState.notifyVertexBufferDelete(id);
    }
    void notifyIndexBufferDelete(GrGLuint id) {
        fHWGeometryState.notifyIndexBufferDelete(id);
    }

    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) SK_OVERRIDE;

    bool canCopySurface(const GrSurface* dst,
                        const GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) SK_OVERRIDE;

    void buildProgramDesc(GrProgramDesc*,
                          const GrPrimitiveProcessor&,
                          const GrPipeline&,
                          const GrBatchTracker&) const SK_OVERRIDE;

private:
    // GrGpu overrides
    void onResetContext(uint32_t resetBits) SK_OVERRIDE;

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, bool budgeted, const void* srcData,
                               size_t rowBytes) SK_OVERRIDE;
    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, bool budgeted,
                                         const void* srcData) SK_OVERRIDE;
    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) SK_OVERRIDE;
    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) SK_OVERRIDE;
    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) SK_OVERRIDE;
    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) SK_OVERRIDE;
    bool createStencilBufferForRenderTarget(GrRenderTarget* rt, int width, int height) SK_OVERRIDE;
    bool attachStencilBufferToRenderTarget(GrStencilBuffer* sb, GrRenderTarget* rt) SK_OVERRIDE;

    void onClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                 bool canIgnoreRect) SK_OVERRIDE;

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) SK_OVERRIDE;

    bool onReadPixels(GrRenderTarget* target,
                      int left, int top,
                      int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) SK_OVERRIDE;

    bool onWriteTexturePixels(GrTexture* texture,
                              int left, int top, int width, int height,
                              GrPixelConfig config, const void* buffer,
                              size_t rowBytes) SK_OVERRIDE;

    void onResolveRenderTarget(GrRenderTarget* target) SK_OVERRIDE;

    void onDraw(const DrawArgs&, const GrDrawTarget::DrawInfo&) SK_OVERRIDE;
    void onStencilPath(const GrPath*, const StencilPathState&) SK_OVERRIDE;
    void onDrawPath(const DrawArgs&, const GrPath*, const GrStencilSettings&) SK_OVERRIDE;
    void onDrawPaths(const DrawArgs&,
                     const GrPathRange*,
                     const void* indices,
                     GrDrawTarget::PathIndexType,
                     const float transformValues[],
                     GrDrawTarget::PathTransformType,
                     int count,
                     const GrStencilSettings&) SK_OVERRIDE;

    void clearStencil(GrRenderTarget*) SK_OVERRIDE;

    // GrDrawTarget overrides
    void didAddGpuTraceMarker() SK_OVERRIDE;
    void didRemoveGpuTraceMarker() SK_OVERRIDE;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    // Flushes state from GrPipeline to GL. Returns false if the state couldn't be set.
    // TODO we only have need to know if this is a line draw for flushing AA state on some buggy
    // hardware.  Evaluate if this is really necessary anymore
    bool flushGLState(const DrawArgs&, bool isLineDraw);

    // Sets up vertex attribute pointers and strides. On return indexOffsetInBytes gives the offset
    // an into the index buffer. It does not account for drawInfo.startIndex() but rather the start
    // index is relative to the returned offset.
    void setupGeometry(const GrPrimitiveProcessor&,
                       const GrDrawTarget::DrawInfo& info,
                       size_t* indexOffsetInBytes);

    // Subclasses should call this to flush the blend state.
    void flushBlend(const GrXferProcessor::BlendInfo& blendInfo);

    bool hasExtension(const char* ext) const { return fGLContext.hasExtension(ext); }

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

    class ProgramCache : public ::SkNoncopyable {
    public:
        ProgramCache(GrGLGpu* gpu);
        ~ProgramCache();

        void abandon();
        GrGLProgram* getProgram(const DrawArgs&);

    private:
        enum {
            // We may actually have kMaxEntries+1 shaders in the GL context because we create a new
            // shader before evicting from the cache.
            kMaxEntries = 128,
            kHashBits = 6,
        };

        struct Entry;

        struct ProgDescLess;

        // binary search for entry matching desc. returns index into fEntries that matches desc or ~
        // of the index of where it should be inserted.
        int search(const GrProgramDesc& desc) const;

        // sorted array of all the entries
        Entry*                      fEntries[kMaxEntries];
        // hash table based on lowest kHashBits bits of the program key. Used to avoid binary
        // searching fEntries.
        Entry*                      fHashTable[1 << kHashBits];

        int                         fCount;
        unsigned int                fCurrLRUStamp;
        GrGLGpu*                    fGpu;
#ifdef PROGRAM_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
        int                         fHashMisses; // cache hit but hash table missed
#endif
    };

    void flushDither(bool dither);
    void flushColorWrite(bool writeColor);
    void flushDrawFace(GrPipelineBuilder::DrawFace face);

    // flushes the scissor. see the note on flushBoundTextureAndParams about
    // flushing the scissor after that function is called.
    void flushScissor(const GrScissorState&,
                      const GrGLIRect& rtViewport,
                      GrSurfaceOrigin rtOrigin);

    // disables the scissor
    void disableScissor();

    void initFSAASupport();

    // determines valid stencil formats
    void initStencilFormats();

    // sets a texture unit to use for texture operations other than binding a texture to a program.
    // ensures that such operations don't negatively interact with tracking bound textures.
    void setScratchTextureUnit();

    // Binds the render target, sets the viewport, tracks dirty are for resolve, and tracks whether
    // mip maps need rebuilding. bounds is region that may be modified by the draw. NULL means whole
    // target. Can be an empty rect.
    void prepareToDrawToRenderTarget(GrGLRenderTarget*, const SkIRect* bounds);

    // On older GLs there may not be separate FBO bindings for draw and read. In that case these
    // alias each other.
    enum FBOBinding {
        kDraw_FBOBinding, // drawing or dst of blit
        kRead_FBOBinding, // src of blit, read pixels.

        kLast_FBOBinding = kRead_FBOBinding
    };
    static const int kFBOBindingCnt = kLast_FBOBinding + 1;

    // binds the FBO and returns the GL enum of the framebuffer target it was bound to.
    GrGLenum bindFBO(FBOBinding, const GrGLFBO*);
    // This version invokes a workaround for a bug in Chromium. It should be called before
    // attachments are changed on a FBO.
    GrGLenum bindFBOForAddingAttachments(const GrGLFBO*);

    void setViewport(const GrGLIRect& viewport);

    void flushStencil(const GrStencilSettings&);
    void flushHWAAState(GrRenderTarget* rt, bool useHWAA, bool isLineDraw);

    bool configToGLFormats(GrPixelConfig config,
                           bool getSizedInternal,
                           GrGLenum* internalFormat,
                           GrGLenum* externalFormat,
                           GrGLenum* externalType);
    // helper for onCreateTexture and writeTexturePixels
    bool uploadTexData(const GrSurfaceDesc& desc,
                       bool isNewTexture,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const void* data,
                       size_t rowBytes);

    // helper for onCreateCompressedTexture. If width and height are
    // set to -1, then this function will use desc.fWidth and desc.fHeight
    // for the size of the data. The isNewTexture flag should be set to true
    // whenever a new texture needs to be created. Otherwise, we assume that
    // the texture is already in GPU memory and that it's going to be updated
    // with new data.
    bool uploadCompressedTexData(const GrSurfaceDesc& desc,
                                 const void* data,
                                 bool isNewTexture = true,
                                 int left = 0, int top = 0,
                                 int width = -1, int height = -1);

    bool createRenderTargetObjects(const GrSurfaceDesc&, bool budgeted, GrGLuint texID, 
                                   GrGLRenderTarget::IDDesc*);

    static const FBOBinding kInvalidFBOBinding = static_cast<FBOBinding>(-1);

    // Binds a surface as an FBO. A temporary FBO ID may be used if the surface is not already
    // a render target. Afterwards unbindSurfaceAsFBOForCopy must be called with the value returned.
    FBOBinding bindSurfaceAsFBOForCopy(GrSurface*, FBOBinding, GrGLIRect* viewport);

    // Must be matched with bindSurfaceAsFBOForCopy.
    void unbindSurfaceAsFBOForCopy(FBOBinding);

    GrGLContext fGLContext;

    // GL program-related state
    ProgramCache*               fProgramCache;
    SkAutoTUnref<GrGLProgram>   fCurrentProgram;

    ///////////////////////////////////////////////////////////////////////////
    ///@name Caching of GL State
    ///@{
    int                         fHWActiveTextureUnitIdx;
    GrGLuint                    fHWProgramID;

    enum TriState {
        kNo_TriState,
        kYes_TriState,
        kUnknown_TriState
    };

    SkAutoTUnref<GrGLFBO> fTempSrcFBO;
    SkAutoTUnref<GrGLFBO> fTempDstFBO;
    SkAutoTUnref<GrGLFBO> fStencilClearFBO;

    // last scissor / viewport scissor state seen by the GL.
    struct {
        TriState    fEnabled;
        GrGLIRect   fRect;
        void invalidate() {
            fEnabled = kUnknown_TriState;
            fRect.invalidate();
        }
    } fHWScissorSettings;

    GrGLIRect   fHWViewport;

    /**
     * Tracks bound vertex and index buffers and vertex attrib array state.
     */
    class HWGeometryState {
    public:
        HWGeometryState() { fVBOVertexArray = NULL; this->invalidate(); }

        ~HWGeometryState() { SkSafeUnref(fVBOVertexArray); }

        void invalidate() {
            fBoundVertexArrayIDIsValid = false;
            fBoundVertexBufferIDIsValid = false;
            fDefaultVertexArrayBoundIndexBufferID = false;
            fDefaultVertexArrayBoundIndexBufferIDIsValid = false;
            fDefaultVertexArrayAttribState.invalidate();
            if (fVBOVertexArray) {
                fVBOVertexArray->invalidateCachedState();
            }
        }

        void notifyVertexArrayDelete(GrGLuint id) {
            if (fBoundVertexArrayIDIsValid && fBoundVertexArrayID == id) {
                // Does implicit bind to 0
                fBoundVertexArrayID = 0;
            }
        }

        void setVertexArrayID(GrGLGpu* gpu, GrGLuint arrayID) {
            if (!gpu->glCaps().vertexArrayObjectSupport()) {
                SkASSERT(0 == arrayID);
                return;
            }
            if (!fBoundVertexArrayIDIsValid || arrayID != fBoundVertexArrayID) {
                GR_GL_CALL(gpu->glInterface(), BindVertexArray(arrayID));
                fBoundVertexArrayIDIsValid = true;
                fBoundVertexArrayID = arrayID;
            }
        }

        void notifyVertexBufferDelete(GrGLuint id) {
            if (fBoundVertexBufferIDIsValid && id == fBoundVertexBufferID) {
                fBoundVertexBufferID = 0;
            }
            if (fVBOVertexArray) {
                fVBOVertexArray->notifyVertexBufferDelete(id);
            }
            fDefaultVertexArrayAttribState.notifyVertexBufferDelete(id);
        }

        void notifyIndexBufferDelete(GrGLuint id) {
            if (fDefaultVertexArrayBoundIndexBufferIDIsValid &&
                id == fDefaultVertexArrayBoundIndexBufferID) {
                fDefaultVertexArrayBoundIndexBufferID = 0;
            }
            if (fVBOVertexArray) {
                fVBOVertexArray->notifyIndexBufferDelete(id);
            }
        }

        void setVertexBufferID(GrGLGpu* gpu, GrGLuint id) {
            if (!fBoundVertexBufferIDIsValid || id != fBoundVertexBufferID) {
                GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ARRAY_BUFFER, id));
                fBoundVertexBufferIDIsValid = true;
                fBoundVertexBufferID = id;
            }
        }

        /**
         * Binds the default vertex array and binds the index buffer. This is used when binding
         * an index buffer in order to update it.
         */
        void setIndexBufferIDOnDefaultVertexArray(GrGLGpu* gpu, GrGLuint id) {
            this->setVertexArrayID(gpu, 0);
            if (!fDefaultVertexArrayBoundIndexBufferIDIsValid ||
                id != fDefaultVertexArrayBoundIndexBufferID) {
                GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, id));
                fDefaultVertexArrayBoundIndexBufferIDIsValid = true;
                fDefaultVertexArrayBoundIndexBufferID = id;
            }
        }

        /**
         * Binds the vertex array object that should be used to render from the vertex buffer.
         * The vertex array is bound and its attrib array state object is returned. The vertex
         * buffer is bound. The index buffer (if non-NULL) is bound to the vertex array. The
         * returned GrGLAttribArrayState should be used to set vertex attribute arrays.
         */
        GrGLAttribArrayState* bindArrayAndBuffersToDraw(GrGLGpu* gpu,
                                                        const GrGLVertexBuffer* vbuffer,
                                                        const GrGLIndexBuffer* ibuffer);

    private:
        GrGLuint                fBoundVertexArrayID;
        GrGLuint                fBoundVertexBufferID;
        bool                    fBoundVertexArrayIDIsValid;
        bool                    fBoundVertexBufferIDIsValid;

        GrGLuint                fDefaultVertexArrayBoundIndexBufferID;
        bool                    fDefaultVertexArrayBoundIndexBufferIDIsValid;
        // We return a non-const pointer to this from bindArrayAndBuffersToDraw when vertex array 0
        // is bound. However, this class is internal to GrGLGpu and this object never leaks out of
        // GrGLGpu.
        GrGLAttribArrayState    fDefaultVertexArrayAttribState;

        // This is used when we're using a core profile and the vertices are in a VBO.
        GrGLVertexArray*        fVBOVertexArray;
    } fHWGeometryState;

    struct {
        GrBlendCoeff    fSrcCoeff;
        GrBlendCoeff    fDstCoeff;
        GrColor         fConstColor;
        bool            fConstColorValid;
        TriState        fEnabled;

        void invalidate() {
            fSrcCoeff = kInvalid_GrBlendCoeff;
            fDstCoeff = kInvalid_GrBlendCoeff;
            fConstColorValid = false;
            fEnabled = kUnknown_TriState;
        }
    } fHWBlendState;

    TriState fMSAAEnabled;

    GrStencilSettings           fHWStencilSettings;
    TriState                    fHWStencilTestEnabled;


    GrPipelineBuilder::DrawFace fHWDrawFace;
    TriState                    fHWWriteToColor;
    TriState                    fHWDitherEnabled;
    SkTArray<uint32_t, true>    fHWBoundTextureUniqueIDs;

    // Track fbo binding state.
    struct HWFBOBinding {
        SkAutoTUnref<const GrGLFBO> fFBO;
        void invalidate() { fFBO.reset(NULL); }
    } fHWFBOBinding[kFBOBindingCnt];

    ///@}

    // we record what stencil format worked last time to hopefully exit early
    // from our loop that tries stencil formats and calls check fb status.
    int fLastSuccessfulStencilFmtIdx;

    typedef GrGpu INHERITED;
    friend class GrGLPathRendering; // For accessing setTextureUnit.
};

#endif
