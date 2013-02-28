/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGpuGL_DEFINED
#define GrGpuGL_DEFINED

#include "GrBinHashKey.h"
#include "GrDrawState.h"
#include "GrGpu.h"
#include "GrGLContext.h"
#include "GrGLIndexBuffer.h"
#include "GrGLIRect.h"
#include "GrGLProgram.h"
#include "GrGLStencilBuffer.h"
#include "GrGLTexture.h"
#include "GrGLVertexBuffer.h"
#include "../GrTHashCache.h"

class GrGpuGL : public GrGpu {
public:
    GrGpuGL(const GrGLContext& ctx, GrContext* context);
    virtual ~GrGpuGL();

    const GrGLInterface* glInterface() const { return fGLContext.interface(); }
    GrGLBinding glBinding() const { return fGLContext.info().binding(); }
    GrGLVersion glVersion() const { return fGLContext.info().version(); }
    GrGLSLGeneration glslGeneration() const { return fGLContext.info().glslGeneration(); }

    // Used by GrGLProgram to bind necessary textures for GrGLEffects.
    void bindTexture(int unitIdx, const GrTextureParams& params, GrGLTexture* texture);

    bool programUnitTest(int maxStages);

    // GrGpu overrides
    virtual GrPixelConfig preferredReadPixelsConfig(GrPixelConfig config) const SK_OVERRIDE;
    virtual GrPixelConfig preferredWritePixelsConfig(GrPixelConfig config) const SK_OVERRIDE;
    virtual bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const SK_OVERRIDE;
    virtual bool readPixelsWillPayForYFlip(
                                    GrRenderTarget* renderTarget,
                                    int left, int top,
                                    int width, int height,
                                    GrPixelConfig config,
                                    size_t rowBytes) const SK_OVERRIDE;
    virtual bool fullReadPixelsIsFasterThanPartial() const SK_OVERRIDE;

    virtual void abandonResources() SK_OVERRIDE;

    const GrGLCaps& glCaps() const { return fGLContext.info().caps(); }

    // Callbacks to update state tracking when related GL objects are bound or deleted
    void notifyVertexBufferBind(GrGLuint id);
    void notifyVertexBufferDelete(GrGLuint id);
    void notifyIndexBufferBind(GrGLuint id);
    void notifyIndexBufferDelete(GrGLuint id);
    void notifyTextureDelete(GrGLTexture* texture);
    void notifyRenderTargetDelete(GrRenderTarget* renderTarget);

private:
    // GrGpu overrides
    virtual void onResetContext() SK_OVERRIDE;

    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes) SK_OVERRIDE;
    virtual GrVertexBuffer* onCreateVertexBuffer(uint32_t size,
                                                 bool dynamic) SK_OVERRIDE;
    virtual GrIndexBuffer* onCreateIndexBuffer(uint32_t size,
                                               bool dynamic) SK_OVERRIDE;
    virtual GrPath* onCreatePath(const SkPath&) SK_OVERRIDE;
    virtual GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) SK_OVERRIDE;
    virtual GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) SK_OVERRIDE;
    virtual bool createStencilBufferForRenderTarget(GrRenderTarget* rt,
                                                    int width,
                                                    int height) SK_OVERRIDE;
    virtual bool attachStencilBufferToRenderTarget(
        GrStencilBuffer* sb,
        GrRenderTarget* rt) SK_OVERRIDE;

    virtual void onClear(const GrIRect* rect, GrColor color) SK_OVERRIDE;

    virtual void onForceRenderTargetFlush() SK_OVERRIDE;

    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top,
                              int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes) SK_OVERRIDE;

    virtual bool onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes) SK_OVERRIDE;

    virtual void onResolveRenderTarget(GrRenderTarget* target) SK_OVERRIDE;

    virtual void onGpuDraw(const DrawInfo&) SK_OVERRIDE;

    virtual void setStencilPathSettings(const GrPath&,
                                        SkPath::FillType,
                                        GrStencilSettings* settings)
                                        SK_OVERRIDE;
    virtual void onGpuStencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;

    virtual void clearStencil() SK_OVERRIDE;
    virtual void clearStencilClip(const GrIRect& rect,
                                  bool insideClip) SK_OVERRIDE;
    virtual bool flushGraphicsState(DrawType) SK_OVERRIDE;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    // Sets up vertex attribute pointers and strides. On return indexOffsetInBytes gives the offset
    // an into the index buffer. It does not account for drawInfo.startIndex() but rather the start
    // index is relative to the returned offset.
    void setupGeometry(const DrawInfo& info, size_t* indexOffsetInBytes);
    // binds appropriate vertex and index buffers. It also returns offsets for the vertex and index
    // buffers. These offsets account for placement within a pool buffer or CPU-side addresses for
    // use with buffer 0. They do not account for start values in the DrawInfo (which is not passed
    // here). The vertex buffer that contains the vertex data is returned. It is not necessarily
    // bound.
    GrGLVertexBuffer* setBuffers(bool indexed, size_t* vertexOffsetInBytes, size_t* indexOffsetInBytes);

    // Subclasses should call this to flush the blend state.
    // The params should be the final coefficients to apply
    // (after any blending optimizations or dual source blending considerations
    // have been accounted for).
    void flushBlend(bool isLines, GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff);

    bool hasExtension(const char* ext) const { return fGLContext.info().hasExtension(ext); }

    const GrGLContext& glContext() const { return fGLContext; }

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

    class ProgramCache : public ::GrNoncopyable {
    public:
        ProgramCache(const GrGLContext& gl);

        void abandon();
        GrGLProgram* getProgram(const GrGLProgram::Desc& desc, const GrEffectStage* stages[]);
    private:
        enum {
            kKeySize = sizeof(GrGLProgram::Desc),
            // We may actually have kMaxEntries+1 shaders in the GL context because we create a new
            // shader before evicting from the cache.
            kMaxEntries = 32
        };

        class Entry;
        // The value of the hash key is based on the ProgramDesc.
        typedef GrTBinHashKey<Entry, kKeySize> ProgramHashKey;

        class Entry : public ::GrNoncopyable {
        public:
            Entry() : fProgram(NULL), fLRUStamp(0) {}
            Entry& operator = (const Entry& entry) {
                GrSafeRef(entry.fProgram.get());
                fProgram.reset(entry.fProgram.get());
                fKey = entry.fKey;
                fLRUStamp = entry.fLRUStamp;
                return *this;
            }
            int compare(const ProgramHashKey& key) const {
                return fKey.compare(key);
            }

        public:
            SkAutoTUnref<GrGLProgram>   fProgram;
            ProgramHashKey              fKey;
            unsigned int                fLRUStamp; // Move outside entry?
        };

        GrTHashTable<Entry, ProgramHashKey, 8> fHashCache;

        Entry                       fEntries[kMaxEntries];
        int                         fCount;
        unsigned int                fCurrLRUStamp;
        const GrGLContext&          fGL;
    };

    // sets the matrix for path stenciling (uses the GL fixed pipe matrices)
    void flushPathStencilMatrix();

    // flushes dithering, color-mask, and face culling stat
    void flushMiscFixedFunctionState();

    // flushes the scissor. see the note on flushBoundTextureAndParams about
    // flushing the scissor after that function is called.
    void flushScissor();

    // Inits GrDrawTarget::Caps, subclass may enable additional caps.
    void initCaps();

    void initFSAASupport();

    // determines valid stencil formats
    void initStencilFormats();

    void setSpareTextureUnit();

    // bound is region that may be modified and therefore has to be resolved.
    // NULL means whole target. Can be an empty rect.
    void flushRenderTarget(const GrIRect* bound);
    void flushStencil(DrawType);
    void flushAAState(DrawType);

    bool configToGLFormats(GrPixelConfig config,
                           bool getSizedInternal,
                           GrGLenum* internalFormat,
                           GrGLenum* externalFormat,
                           GrGLenum* externalType);
    // helper for onCreateTexture and writeTexturePixels
    bool uploadTexData(const GrGLTexture::Desc& desc,
                       bool isNewTexture,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const void* data,
                       size_t rowBytes);

    bool createRenderTargetObjects(int width, int height,
                                   GrGLuint texID,
                                   GrGLRenderTarget::Desc* desc);

    void fillInConfigRenderableTable();

    GrGLContext fGLContext;

    // GL program-related state
    ProgramCache*               fProgramCache;
    SkAutoTUnref<GrGLProgram>   fCurrentProgram;

    ///////////////////////////////////////////////////////////////////////////
    ///@name Caching of GL State
    ///@{
    int                         fHWActiveTextureUnitIdx;
    GrGLuint                    fHWProgramID;

    GrGLProgram::SharedGLState  fSharedGLProgramState;

    enum TriState {
        kNo_TriState,
        kYes_TriState,
        kUnknown_TriState
    };

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
        HWGeometryState() { fAttribArrayCount = 0; this->invalidate();}

        void setMaxAttribArrays(int max) {
            fAttribArrayCount = max;
            fAttribArrays.reset(max);
            for (int i = 0; i < fAttribArrayCount; ++i) {
                fAttribArrays[i].invalidate();
            }
        }

        void invalidate() {
            fBoundVertexBufferIDIsValid = false;
            fBoundIndexBufferIDIsValid = false;
            for (int i = 0; i < fAttribArrayCount; ++i) {
                fAttribArrays[i].invalidate();
            }
        }

        void notifyVertexBufferDelete(GrGLuint id) {
            if (0 != id) {
                if (this->isVertexBufferIDBound(id)) {
                    // deleting bound buffer does implied bind to 0
                    this->setVertexBufferID(0);
                }
                for (int i = 0; i < fAttribArrayCount; ++i) {
                    if (fAttribArrays[i].vertexBufferID() == id) {
                        fAttribArrays[i].invalidate();
                    }
                }
            }
        }

        void notifyIndexBufferDelete(GrGLuint id) {
            if (0 != id) {
                if (this->isIndexBufferIDBound(id)) {
                    // deleting bound buffer does implied bind to 0
                    this->setIndexBufferID(0);
                }
            }
        }

        void setVertexBufferID(GrGLuint id) {
            fBoundVertexBufferIDIsValid = true;
            fBoundVertexBufferID = id;
        }

        void setIndexBufferID(GrGLuint id) {
            fBoundIndexBufferIDIsValid = true;
            fBoundIndexBufferID = id;
        }

        bool isVertexBufferIDBound(GrGLuint id) const {
            return fBoundVertexBufferIDIsValid && id == fBoundVertexBufferID;
        }

        bool isIndexBufferIDBound(GrGLuint id) const {
            return fBoundIndexBufferIDIsValid && id == fBoundIndexBufferID;
        }

        void setAttribArray(const GrGpuGL* gpu,
                            int index,
                            GrGLVertexBuffer* vertexBuffer,
                            GrGLint size,
                            GrGLenum type,
                            GrGLboolean normalized,
                            GrGLsizei stride,
                            GrGLvoid* offset) {
            GrAssert(index >= 0 && index < fAttribArrayCount);
            AttribArray* attrib = fAttribArrays.get() + index;
            attrib->set(gpu, this, index, vertexBuffer, size, type, normalized, stride, offset);
        }

        void disableUnusedAttribArrays(const GrGpuGL* gpu,
                                       uint32_t usedAttribIndexMask) {
            for (int i = 0; i < fAttribArrayCount; ++i) {
                if (!(usedAttribIndexMask & (1 << i))) {
                    fAttribArrays[i].disable(gpu, i);
                }
            }
        }

    private:
        GrGLuint                fBoundVertexBufferID;
        GrGLuint                fBoundIndexBufferID;
        bool                    fBoundVertexBufferIDIsValid;
        bool                    fBoundIndexBufferIDIsValid;

        struct AttribArray {
        public:
            void set(const GrGpuGL* gpu,
                     HWGeometryState* geoState,
                     int index,
                     GrGLVertexBuffer* vertexBuffer,
                     GrGLint size,
                     GrGLenum type,
                     GrGLboolean normalized,
                     GrGLsizei stride,
                     GrGLvoid* offset);

            void disable(const GrGpuGL* gpu, int index) {
                if (!fEnableIsValid || fEnabled) {
                    GR_GL_CALL(gpu->glInterface(), DisableVertexAttribArray(index));
                    fEnableIsValid = true;
                    fEnabled = false;
                }
            }

            void invalidate() {
                fEnableIsValid = false;
                fAttribPointerIsValid = false;
            }

            GrGLuint vertexBufferID() const { return fVertexBufferID; }
        private:
            bool        fEnableIsValid;
            bool        fAttribPointerIsValid;
            bool        fEnabled;
            GrGLuint    fVertexBufferID;
            GrGLint     fSize;
            GrGLenum    fType;
            GrGLboolean fNormalized;
            GrGLsizei   fStride;
            GrGLvoid*   fOffset;
        };
        SkAutoTArray<AttribArray> fAttribArrays;
        int                       fAttribArrayCount;
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

    struct {
        TriState fMSAAEnabled;
        TriState fSmoothLineEnabled;
        void invalidate() {
            fMSAAEnabled = kUnknown_TriState;
            fSmoothLineEnabled = kUnknown_TriState;
        }
    } fHWAAState;


    GrGLProgram::MatrixState    fHWPathStencilMatrixState;

    GrStencilSettings           fHWStencilSettings;
    TriState                    fHWStencilTestEnabled;

    GrDrawState::DrawFace       fHWDrawFace;
    TriState                    fHWWriteToColor;
    TriState                    fHWDitherEnabled;
    GrRenderTarget*             fHWBoundRenderTarget;
    GrTexture*                  fHWBoundTextures[GrDrawState::kNumStages];
    ///@}

    // we record what stencil format worked last time to hopefully exit early
    // from our loop that tries stencil formats and calls check fb status.
    int fLastSuccessfulStencilFmtIdx;

    bool fPrintedCaps;

    typedef GrGpu INHERITED;
};

#endif
