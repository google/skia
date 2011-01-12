/*
    Copyright 2010 Google Inc.

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


#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrScalar.h"
#include "GrMatrix.h"
#include "GrColor.h"
#include "GrRefCnt.h"
#include "GrSamplerState.h"
#include "GrClip.h"

class GrTexture;
class GrRenderTarget;
class GrClipIterator;
class GrVertexBuffer;
class GrIndexBuffer;

class GrDrawTarget : public GrRefCnt {
public:
    /**
     * Geometric primitives used for drawing.
     */
    enum PrimitiveType {
        kTriangles_PrimitiveType,
        kTriangleStrip_PrimitiveType,
        kTriangleFan_PrimitiveType,
        kPoints_PrimitiveType,
        kLines_PrimitiveType,
        kLineStrip_PrimitiveType
    };

    /**
     *  Flags that affect rendering. Controlled using enable/disableState(). All
     *  default to disabled.
     */
    enum StateBits {
        kDither_StateBit          = 0x1,//<! Perform color dithering
        kAntialias_StateBit       = 0x2,//<! Perform anti-aliasing. The render-
                                        //   target must support some form of AA
                                        //   (msaa, coverage sampling, etc). For
                                        //   GrGpu-created rendertarget/textures
                                        //   this is controlled by parameters
                                        //   passed to createTexture.
        kClip_StateBit            = 0x4,//<! Controls whether drawing is clipped
                                        //   against the region specified by
                                        //   setClip.
    };

    /**
     * Coeffecients for alpha-blending.
     */
    enum BlendCoeff {
        kZero_BlendCoeff,    //<! 0
        kOne_BlendCoeff,     //<! 1
        kSC_BlendCoeff,      //<! src color
        kISC_BlendCoeff,     //<! one minus src color
        kDC_BlendCoeff,      //<! dst color
        kIDC_BlendCoeff,     //<! one minus dst color
        kSA_BlendCoeff,      //<! src alpha
        kISA_BlendCoeff,     //<! one minus src alpha
        kDA_BlendCoeff,      //<! dst alpha
        kIDA_BlendCoeff,     //<! one minus dst alpha
    };

    /**
     * StencilPass
     *
     * Sets the stencil state for subsequent draw calls. Used to fill paths.
     *
     * Winding requires two passes when the GPU/API doesn't support separate
     * stencil.
     *
     * The color pass for path fill is used to zero out stencil bits used for
     * path filling. Every pixel covere by a winding/EO stencil pass must get
     * covered by the color pass in order to leave stencil buffer in the correct
     * state for the next path draw.
     *
     * NOTE: Stencil-based Winding fill has alias-to-zero problems. (e.g. A
     * winding count of 128,256,512,etc with a 8 bit stencil buffer
     * will be unfilled)
     */
    enum StencilPass {
        kNone_StencilPass,            //<! Not drawing a path or clip.
        kEvenOddStencil_StencilPass,  //<! records in/out in stencil buffer
                                      //   using the Even/Odd fill rule.
        kEvenOddColor_StencilPass,    //<! writes colors to color target in
                                      //   pixels marked inside the fill by
                                      //   kEOFillStencil_StencilPass. Clears
                                      //   stencil in pixels covered by
                                      //   geometry.
        kWindingStencil1_StencilPass, //<! records in/out in stencil buffer
                                      //   using the Winding fill rule.
        kWindingStencil2_StencilPass, //<! records in/out in stencil buffer
                                      //   using the Winding fill rule.
                                      //   Run when single-stencil-pass winding
                                      //   not supported (i.e. no separate
                                      //   stencil support)
        kWindingColor_StencilPass,    //<! writes colors to color target in
                                      //   pixels marked inside the fill by
                                      //   kWindFillStencil_StencilPass. Clears
                                      //   stencil in pixels covered by
                                      //   geometry.
        kDrawTargetCount_StencilPass  //<! Subclass may extend this enum to use
                                      //   the stencil for other purposes (e.g.
                                      //   to do stencil-based clipping)
                                      //   This value is provided as basis for
                                      //   defining these extended enum values.
    };

protected:
    enum MatrixMode {
        kModelView_MatrixMode = 0,
        kTexture_MatrixMode,

        kMatrixModeCount
    };

    struct DrState {
        uint32_t                fFlagBits;
        BlendCoeff          	fSrcBlend;
        BlendCoeff          	fDstBlend;
        GrTexture*          	fTexture;
        GrSamplerState        	fSamplerState;
        GrRenderTarget*     	fRenderTarget;
        GrColor             	fColor;
        float               	fPointSize;
        StencilPass             fStencilPass;
        bool                    fReverseFill;
        GrMatrix                fMatrixModeCache[kMatrixModeCount];
        bool operator ==(const DrState& s) const {
            return 0 == memcmp(this, &s, sizeof(DrState));
        }
        bool operator !=(const DrState& s) const { return !(*this == s); }
    };

public:
    ///////////////////////////////////////////////////////////////////////////

    GrDrawTarget();

    /**
     * Sets the current clip to the region specified by clip. All draws will be
     * clipped against this clip if kClip_StateBit is enabled.
     *
     * @param description of the clipping region
     */
    void setClip(const GrClip& clip);

    /**
     * Gets the current clip.
     *
     * @return the clip.
     */
    const GrClip& getClip() const;

    /**
     * Sets the texture used at the next drawing call
     *
     * @param texture The texture to set. Can be NULL though there is no advantage
     * to settings a NULL texture if doing non-textured drawing
     */
    void setTexture(GrTexture* texture);

    /**
     * Retrieves the currently set texture.
     *
     * @return    The currently set texture. The return value will be NULL if no
     *            texture has been set, NULL was most recently passed to
     *            setTexture, or the last setTexture was destroyed.
     */
    GrTexture* currentTexture() const;

    /**
     * Sets the rendertarget used at the next drawing call
     *
     * @param target  The render target to set. Must be a valid rendertarget.
     *                That is it is a value that was returned by
     *                currentRenderTarget() or GrTexture::asRenderTarget().
     */
    void setRenderTarget(GrRenderTarget* target);

    /**
     * Retrieves the currently set rendertarget.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* currentRenderTarget() const;

    /**
     * Sets the sampler state for the next draw.
     *
     * The sampler state determines the address wrap modes and
     * filtering
     *
     * @param samplerState    Specifies the sampler state.
     */
    void setSamplerState(const GrSamplerState& samplerState);

    /**
     * Sets the matrix applied to texture coordinates.
     *
     * The post-matrix texture coordinates in the square [0,1]^2 cover the
     * entire area of the texture. This means the full POT width when a NPOT
     * texture is embedded in a POT width texture to meet the 3D API
     * requirements. The texture matrix is applied both when the texture
     * coordinates are explicit and when vertex positions are used as texture
     * coordinates. In the latter case the texture matrix is applied to the
     * pre-modelview position values.
     *
     * @param m the matrix used to transform the texture coordinates.
     */
    void setTextureMatrix(const GrMatrix& m) {
        this->loadMatrix(m, kTexture_MatrixMode);
    }

    /**
     * Sets the matrix applied to veretx positions.
     *
     * In the post-view-matrix space the rectangle [0,w]x[0,h]
     * fully covers the render target. (w and h are the width and height of the
     * the rendertarget.)
     *
     * @param m the matrix used to transform the vertex positions.
     */
    void setViewMatrix(const GrMatrix& m) {
        this->loadMatrix(m, kModelView_MatrixMode);
    }

    /**
     *  Multiplies the current view matrix by a matrix
     *
     *  After this call V' = V*m where V is the old view matrix,
     *  m is the parameter to this function, and V' is the new view matrix.
     *  (We consider positions to be column vectors so position vector p is
     *  transformed by matrix X as p' = X*p.)
     *
     *  @param m the matrix used to modify the modelview matrix.
     */
    void concatViewMatrix(const GrMatrix& m);

    /**
     *  Sets color for next draw to a premultiplied-alpha color.
     *
     *  @param the color to set.
     */
    void setColor(GrColor);

    /**
     *  Sets the color to be used for the next draw to be
     *  (r,g,b,a) = (alpha, alpha, alpha, alpha).
     *
     *  @param alpha The alpha value to set as the color.
     */
    void setAlpha(uint8_t alpha);

    /**
     * Sets pass for path rendering
     *
     * @param pass of path rendering
     */
    void setStencilPass(StencilPass pass);

    /**
     * Reveses the in/out decision of the fill rule for path rendering.
     * Only affects kEOFillColor_StencilPass and kWindingFillColor_StencilPass
     *
     * @param reverse true to reverse, false otherwise
     */
    void setReverseFill(bool reverse);

    /**
     * Enable render state settings.
     *
     * @param flags   bitfield of StateBits specifing the states to enable
     */
    void enableState(uint32_t stateBits);

    /**
     * Disable render state settings.
     *
     * @param flags   bitfield of StateBits specifing the states to disable
     */
    void disableState(uint32_t stateBits);

    bool isDitherState() const {
        return fCurrDrawState.fFlagBits & kDither_StateBit;
    }

    /**
     * Sets the size of points used the next time points are drawn.
     *
     * @param the point size
     */
    void setPointSize(float size);

    /**
     * Sets the blending function coeffecients.
     *
     * The blend function will be:
     *    D' = sat(S*srcCoef + D*dstCoef)
     *
     *   where D is the existing destination color, S is the incoming source
     *   color, and D' is the new destination color that will be written. sat()
     *   is the saturation function.
     *
     * @param srcCoef coeffecient applied to the src color.
     * @param dstCoef coeffecient applied to the dst color.
     */
    void setBlendFunc(BlendCoeff srcCoef, BlendCoeff dstCoef);

    /**
     * Retrieves the current view matrix
     * @param matrix will be the current view matrix after return.
     */
    void getViewMatrix(GrMatrix* matrix) const;

    /**
     *  Retrieves the inverse of the current view matrix.
     *
     *  If the current view matrix is invertible, return true, and if matrix
     *  is non-null, copy the inverse into it. If the current view matrix is
     *  non-invertible, return false and ignore the matrix parameter.
     *
     * @param matrix if not null, will receive a copy of the current inverse.
     */
    bool getViewInverse(GrMatrix* matrix) const;

    /**
     * Used to save and restore the GrGpu's drawing state
     */
    struct SavedDrawState {
    private:
        DrState fState;
        friend class GrDrawTarget;
    };

    /**
     * Saves the current draw state. The state can be restored at a later time
     * with restoreDrawState.
     *
     * See also AutoStateRestore class.
     *
     * @param   state will hold the state after the function returns.
     */
    void saveCurrentDrawState(SavedDrawState* state) const;

    /**
     * Restores previously saved draw state. The client guarantees that state
     * was previously passed to saveCurrentDrawState and that the rendertarget
     * and texture set at save are still valid.
     *
     * See also AutoStateRestore class.
     *
     * @param   state the previously saved state to restore.
     */
    void restoreDrawState(const SavedDrawState& state);

    /**
     * Copies the draw state from another target to this target.
     *
     * @param srcTarget     draw target used as src of the draw state.
     */
    void copyDrawState(const GrDrawTarget& srcTarget);

    /**
     * Flags that indicate the layout of vertex data.
     *
     * kSeparateTexCoord_VertexLayoutBit is incompatible with
     * kPositionAsTexCoord_VertexLayoutBit. kTextFormat_VertexLayoutBit is
     * incompatible with any other flags.
     *
     * When kTextFormat_VertexLayoutBit is set:
     *      Texture coordinates are separate.
     *      Positions and Texture coordinates are SkGpuTextVertex.
     * For non-text vertices:
     *      Position and texture coordinates are GrPoints.
     *      Colors are GrColors.
     *
     * The order is always positions, texture coords, colors.
     */
    enum VertexLayoutBits {
        kSeparateTexCoord_VertexLayoutBit   = 0x1, //<! vertices have texture
                                                   //   coords that are not
                                                   //   inferred from the
                                                   //   positions
        kPositionAsTexCoord_VertexLayoutBit = 0x2, //<! vertices use positions
                                                   //   as texture coords.
        kColor_VertexLayoutBit              = 0x4, //<! vertices have colors
        kTextFormat_VertexLayoutBit         = 0x8, //<! vertices represent glyphs
                                                   //   and therefore contain
                                                   //   two GrGpuTextVertexs.
                                                   //   One for pos and one for
                                                   //   text coords.
        // for below assert
        kDummy,
        kHighVertexLayoutBit = kDummy - 1
    };
    GR_STATIC_ASSERT(kHighVertexLayoutBit < (1 << 8*sizeof(GrVertexLayout)));

    /**
     * Reserves space for vertices and/or indices. Draw target will use
     * reserved vertices / indices at next draw.
     *
     * If succeeds:
     *          if vertexCount is nonzero, *vertices will be the array
     *          of vertices to be filled by caller. The next draw will read
     *          these vertices.
     *
     *          if indecCount is nonzero, *indices will be the array of indices
     *          to be filled by caller. The next indexed draw will read from
     *          these indices.
     *
     * If a client does not already have a vertex buffer or cpu arrays then this
     * is the preferred way to allocate vertex/index array. It allows the
     * subclass of GrDrawTarget to decide whether to put data in buffers, to
     * group vertex data that uses the same state (e.g. for deferred rendering),
     * etc.
     *
     * This must be matched with a releaseReservedGeometry call after all
     * draws that reference the reserved geometry data have been called.
     *
     * AutoGeometryRelease can be used to automatically call the release.
     *
     * @param vertexCount  the number of vertices to reserve space for. Can be 0.
     * @param indexCount   the number of indices to reserve space for. Can be 0.
     * @param vertexLayout the format of vertices (ignored if vertexCount == 0).
     * @param vertices     will point to reserved vertex space if vertexCount is
     *                     non-zero. Illegal to pass NULL if vertexCount > 0.
     * @param indices      will point to reserved index space if indexCount is
     *                     non-zero. Illegal to pass NULL if indexCount > 0.
     *
     * @return  true if succeeded in allocating space for the vertices and false
     *               if not.
     */
    bool reserveAndLockGeometry(GrVertexLayout    vertexLayout,
                                uint32_t          vertexCount,
                                uint32_t          indexCount,
                                void**            vertices,
                                void**            indices);
    /**
     * Provides hints to caller about the number of vertices and indices
     * that can be allocated cheaply. This can be useful if caller is reserving
     * space but doesn't know exactly how much geometry is needed.
     *
     * Also may hint whether the draw target should be flushed first. This is
     * useful for deferred targets.
     *
     * @param vertexLayout layout of vertices caller would like to reserve
     * @param vertexCount  in: hint about how many vertices the caller would
     *                     like to allocate.
     *                     out: a hint about the number of vertices that can be
     *                     allocated cheaply. Negative means no hint.
     *                     Ignored if NULL.
     * @param indexCount   in: hint about how many indices the caller would
     *                     like to allocate.
     *                     out: a hint about the number of indices that can be
     *                     allocated cheaply. Negative means no hint.
     *                     Ignored if NULL.
     *
     * @return  true if target should be flushed based on the input values.
     */
    virtual bool geometryHints(GrVertexLayout vertexLayout,
                               int32_t*       vertexCount,
                               int32_t*       indexCount) const;

    /**
     * Releases reserved vertex/index data from reserveAndLockGeometry().
     */
    void releaseReservedGeometry();

    /**
     * Sets source of vertex data for the next draw. Data does not have to be
     * in the array until drawIndexed or drawNonIndexed.
     *
     * @param array         cpu array containing vertex data.
     * @param vertexLayout  layout of the vertex data in the array.
     */
    void setVertexSourceToArray(const void* array, GrVertexLayout vertexLayout);

    /**
     * Sets source of index data for the next indexed draw. Data does not have
     * to be in the array until drawIndexed or drawNonIndexed.
     *
     * @param array cpu array containing index data.
     */
    void setIndexSourceToArray(const void* array);

    /**
     * Sets source of vertex data for the next draw. Data does not have to be
     * in the buffer until drawIndexed or drawNonIndexed.
     *
     * @param buffer        vertex buffer containing vertex data. Must be
     *                      unlocked before draw call.
     * @param vertexLayout  layout of the vertex data in the buffer.
     */
    void setVertexSourceToBuffer(const GrVertexBuffer* buffer,
                                 GrVertexLayout vertexLayout);

    /**
     * Sets source of index data for the next indexed draw. Data does not have
     * to be in the buffer until drawIndexed or drawNonIndexed.
     *
     * @param buffer index buffer containing indices. Must be unlocked
     *               before indexed draw call.
     */
    void setIndexSourceToBuffer(const GrIndexBuffer* buffer);

    /**
     * Draws indexed geometry using the current state and current vertex / index
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param startIndex   first index to read from index src.
     * @param vertexCount  one greater than the max index.
     * @param indexCount   the number of index elements to read. The index count
     *                     is effectively trimmed to the last completely
     *                     specified primitive.
     */
    virtual void drawIndexed(PrimitiveType type,
                             uint32_t startVertex,
                             uint32_t startIndex,
                             uint32_t vertexCount,
                             uint32_t indexCount) = 0;

    /**
     * Draws non-indexed geometry using the current state and current vertex
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param vertexCount  one greater than the max index.
     */
    virtual void drawNonIndexed(PrimitiveType type,
                                uint32_t startVertex,
                                uint32_t vertexCount)  = 0;

    ///////////////////////////////////////////////////////////////////////////

    class AutoStateRestore : ::GrNoncopyable {
    public:
        AutoStateRestore(GrDrawTarget* target);
        ~AutoStateRestore();

    private:
        GrDrawTarget*       fDrawTarget;
        SavedDrawState      fDrawState;
    };

    ///////////////////////////////////////////////////////////////////////////

    class AutoReleaseGeometry : ::GrNoncopyable {
    public:
        AutoReleaseGeometry(GrDrawTarget*  target,
                            GrVertexLayout vertexLayout,
                            uint32_t       vertexCount,
                            uint32_t       indexCount) {
            fTarget = target;
            fSuccess = fTarget->reserveAndLockGeometry(vertexLayout,
                                                       vertexCount,
                                                       indexCount,
                                                       &fVertices,
                                                       &fIndices);
        }
        ~AutoReleaseGeometry() {
            if (fSuccess) {
                fTarget->releaseReservedGeometry();
            }
        }

        bool succeeded() const { return fSuccess; }
        void* vertices() const { return fVertices; }
        void* indices() const { return fIndices; }

        GrPoint* positions() const {
            return static_cast<GrPoint*>(fVertices);
        }

    private:
        GrDrawTarget* fTarget;
        bool          fSuccess;
        void*         fVertices;
        void*         fIndices;
    };

    ///////////////////////////////////////////////////////////////////////////

    class AutoClipRestore : ::GrNoncopyable {
    public:
        AutoClipRestore(GrDrawTarget* target) {
            fTarget = target;
            fClip = fTarget->getClip();
        }

        ~AutoClipRestore() {
            fTarget->setClip(fClip);
        }
    private:
        GrDrawTarget* fTarget;
        GrClip        fClip;
    };

    ////////////////////////////////////////////////////////////////////////////

    /**
     * Helper function to compute the size of a vertex from a vertex layout
     * @return size of a single vertex.
     */
    static size_t VertexSize(GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of texture coordinates in a vertex
     * @return offset of texture coordinates in vertex layout or -1 if the
     *         layout has no texture coordinates.
     */
    static int VertexTexCoordOffset(GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of the color in a vertex
     * @return offset of color in vertex layout or -1 if the
     *         layout has no color.
     */
    static int VertexColorOffset(GrVertexLayout vertexLayout);

    /**
     * Helper function to compute vertex size and component offsets.
     * @param texCoordOffset  after return it is the offset of texture coords
     *                        in vertex layout or -1 if the layout has no
     *                        texture coords.
     * @param colorOffset     after return it is the offset of color in vertex
     *                        layout or -1 if the layout has no color.
     * @return size of a single vertex.
     */
    static int VertexSizeAndOffsets(GrVertexLayout vertexLayout,
                                    int* texCoordOffset,
                                    int* colorOffset);
    /**
     * Helper function to determine if vertex layout contains either explicit or
     * implicit texture coordinates.
     *
     * @return true if vertex specifies texture coordinates, false otherwise.
     */
    static bool VertexHasTexCoords(GrVertexLayout vertexLayout);

protected:

    // Helpers for GrDrawTarget subclasses that won't have private access to
    // SavedDrawState but need to peek at the state values.
    static DrState& accessSavedDrawState(SavedDrawState& sds)
                                                        { return sds.fState; }
    static const DrState& accessSavedDrawState(const SavedDrawState& sds)
                                                        { return sds.fState; }

    // implemented by subclass
    virtual bool acquireGeometryHelper(GrVertexLayout vertexLayout,
                                       void** vertices,
                                       void** indices) = 0;

    virtual void releaseGeometryHelper() = 0;

    virtual void clipWillChange(const GrClip& clip) = 0;

    enum GeometrySrcType {
        kArray_GeometrySrcType,
        kReserved_GeometrySrcType,
        kBuffer_GeometrySrcType
    };

    struct {
        bool            fLocked;
        uint32_t        fVertexCount;
        uint32_t        fIndexCount;
    } fReservedGeometry;

    struct GeometrySrc {
        GeometrySrcType             fVertexSrc;
        union {
            const GrVertexBuffer*   fVertexBuffer;
            const void*             fVertexArray;
        };
        GeometrySrcType             fIndexSrc;
        union {
            const GrIndexBuffer*    fIndexBuffer;
            const void*             fIndexArray;
        };
        GrVertexLayout              fVertexLayout;
    } fGeometrySrc;

    GrClip fClip;

    DrState fCurrDrawState;

    // set texture or modelview matrix
    void loadMatrix(const GrMatrix&, MatrixMode);

    // not meant for outside usage. Could cause problems if calls between
    // the save and restore mess with reserved geometry state.
    class AutoGeometrySrcRestore {
    public:
        AutoGeometrySrcRestore(GrDrawTarget* target) {
            fTarget = target;
            fGeometrySrc = fTarget->fGeometrySrc;
        }
        ~AutoGeometrySrcRestore() {
            fTarget->fGeometrySrc = fGeometrySrc;
        }
    private:
        GrDrawTarget *fTarget;
        GeometrySrc  fGeometrySrc;

        AutoGeometrySrcRestore();
        AutoGeometrySrcRestore(const AutoGeometrySrcRestore&);
        AutoGeometrySrcRestore& operator =(AutoGeometrySrcRestore&);
    };

};

#endif
