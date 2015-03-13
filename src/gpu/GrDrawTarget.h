/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrClip.h"
#include "GrClipMaskManager.h"
#include "GrContext.h"
#include "GrPathProcessor.h"
#include "GrPrimitiveProcessor.h"
#include "GrIndexBuffer.h"
#include "GrPathRendering.h"
#include "GrPipelineBuilder.h"
#include "GrTraceMarker.h"
#include "GrVertexBuffer.h"

#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"
#include "SkXfermode.h"

class GrBatch;
class GrClip;
class GrDrawTargetCaps;
class GrGeometryProcessor;
class GrPath;
class GrPathRange;
class GrPipeline;

class GrDrawTarget : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawTarget)

    typedef GrPathRange::PathIndexType PathIndexType;
    typedef GrPathRendering::PathTransformType PathTransformType;

    ///////////////////////////////////////////////////////////////////////////

    // The context may not be fully constructed and should not be used during GrDrawTarget
    // construction.
    GrDrawTarget(GrContext* context);
    virtual ~GrDrawTarget();

    /**
     * Gets the capabilities of the draw target.
     */
    const GrDrawTargetCaps* caps() const { return fCaps.get(); }

    /**
     * There are two types of "sources" of geometry (vertices and indices) for
     * draw calls made on the target. When performing an indexed draw, the
     * indices and vertices can use different source types. Once a source is
     * specified it can be used for multiple draws. However, the time at which
     * the geometry data is no longer editable depends on the source type.
     *
     * Sometimes it is necessary to perform a draw while upstack code has
     * already specified geometry that it isn't finished with. So there are push
     * and pop methods. This allows the client to push the sources, draw
     * something using alternate sources, and then pop to restore the original
     * sources.
     *
     * Aside from pushes and pops, a source remains valid until another source
     * is set or resetVertexSource / resetIndexSource is called. Drawing from
     * a reset source is an error.
     *
     * The two types of sources are:
     *
     * 1. Reserve. This is most useful when the caller has data it must
     *    transform before drawing and is not long-lived. The caller requests
     *    that the draw target make room for some amount of vertex and/or index
     *    data. The target provides ptrs to hold the vertex and/or index data.
     *
     *    The data is writable up until the next drawIndexed, drawNonIndexed,
     *    drawIndexedInstances, drawRect, copySurface, or pushGeometrySource. At
     *    this point the data is frozen and the ptrs are no longer valid.
     *
     *    Where the space is allocated and how it is uploaded to the GPU is
     *    subclass-dependent.
     *
     * 2. Vertex and Index Buffers. This is most useful for geometry that will
     *    is long-lived. When the data in the buffer is consumed depends on the
     *    GrDrawTarget subclass. For deferred subclasses the caller has to
     *    guarantee that the data is still available in the buffers at playback.
     *    (TODO: Make this more automatic as we have done for read/write pixels)
     */

    /**
     * Reserves space for vertices and/or indices. Zero can be specifed as
     * either the vertex or index count if the caller desires to only reserve
     * space for only indices or only vertices. If zero is specifed for
     * vertexCount then the vertex source will be unmodified and likewise for
     * indexCount.
     *
     * If the function returns true then the reserve suceeded and the vertices
     * and indices pointers will point to the space created.
     *
     * If the target cannot make space for the request then this function will
     * return false. If vertexCount was non-zero then upon failure the vertex
     * source is reset and likewise for indexCount.
     *
     * The pointers to the space allocated for vertices and indices remain valid
     * until a drawIndexed, drawNonIndexed, drawIndexedInstances, drawRect,
     * copySurface, or push/popGeomtrySource is called. At that point logically a
     * snapshot of the data is made and the pointers are invalid.
     *
     * @param vertexCount  the number of vertices to reserve space for. Can be
     *                     0. Vertex size is queried from the current GrPipelineBuilder.
     * @param indexCount   the number of indices to reserve space for. Can be 0.
     * @param vertices     will point to reserved vertex space if vertexCount is
     *                     non-zero. Illegal to pass NULL if vertexCount > 0.
     * @param indices      will point to reserved index space if indexCount is
     *                     non-zero. Illegal to pass NULL if indexCount > 0.
     */
     bool reserveVertexAndIndexSpace(int vertexCount,
                                     size_t vertexStride,
                                     int indexCount,
                                     void** vertices,
                                     void** indices);

    /**
     * Provides hints to caller about the number of vertices and indices
     * that can be allocated cheaply. This can be useful if caller is reserving
     * space but doesn't know exactly how much geometry is needed.
     *
     * Also may hint whether the draw target should be flushed first. This is
     * useful for deferred targets.
     *
     * @param vertexCount  in: hint about how many vertices the caller would
     *                     like to allocate. Vertex size is queried from the
     *                     current GrPipelineBuilder.
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
    virtual bool geometryHints(size_t vertexStride, int* vertexCount, int* indexCount) const;

    /**
     * Sets source of vertex data for the next draw. Data does not have to be
     * in the buffer until drawIndexed, drawNonIndexed, or drawIndexedInstances.
     *
     * @param buffer        vertex buffer containing vertex data. Must be
     *                      unlocked before draw call. Vertex size is queried
     *                      from current GrPipelineBuilder.
     */
    void setVertexSourceToBuffer(const GrVertexBuffer* buffer, size_t vertexStride);

    /**
     * Sets source of index data for the next indexed draw. Data does not have
     * to be in the buffer until drawIndexed.
     *
     * @param buffer index buffer containing indices. Must be unlocked
     *               before indexed draw call.
     */
    void setIndexSourceToBuffer(const GrIndexBuffer* buffer);

    /**
     * Resets vertex source. Drawing from reset vertices is illegal. Set vertex
     * source to reserved, array, or buffer before next draw. May be able to free
     * up temporary storage allocated by setVertexSourceToArray or
     * reserveVertexSpace.
     */
    void resetVertexSource();

    /**
     * Resets index source. Indexed Drawing from reset indices is illegal. Set
     * index source to reserved, array, or buffer before next indexed draw. May
     * be able to free up temporary storage allocated by setIndexSourceToArray
     * or reserveIndexSpace.
     */
    void resetIndexSource();

    /**
     * Query to find out if the vertex or index source is reserved.
     */
    bool hasReservedVerticesOrIndices() const {
        return kReserved_GeometrySrcType == this->getGeomSrc().fVertexSrc ||
        kReserved_GeometrySrcType == this->getGeomSrc().fIndexSrc;
    }

    /**
     * Pushes and resets the vertex/index sources. Any reserved vertex / index
     * data is finalized (i.e. cannot be updated after the matching pop but can
     * be drawn from). Must be balanced by a pop.
     */
    void pushGeometrySource();

    /**
     * Pops the vertex / index sources from the matching push.
     */
    void popGeometrySource();

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
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawIndexed(GrPipelineBuilder*,
                     const GrGeometryProcessor*,
                     GrPrimitiveType type,
                     int startVertex,
                     int startIndex,
                     int vertexCount,
                     int indexCount,
                     const SkRect* devBounds = NULL);

    /**
     * Draws non-indexed geometry using the current state and current vertex
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param vertexCount  one greater than the max index.
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawNonIndexed(GrPipelineBuilder*,
                        const GrGeometryProcessor*,
                        GrPrimitiveType type,
                        int startVertex,
                        int vertexCount,
                        const SkRect* devBounds = NULL);

    // TODO devbounds should live on the batch
    void drawBatch(GrPipelineBuilder*, GrBatch*, const SkRect* devBounds = NULL);

    /**
     * Draws path into the stencil buffer. The fill must be either even/odd or
     * winding (not inverse or hairline). It will respect the HW antialias flag
     * on the GrPipelineBuilder (if possible in the 3D API).  Note, we will never have an inverse
     * fill with stencil path
     */
    void stencilPath(GrPipelineBuilder*, const GrPathProcessor*, const GrPath*,
                     GrPathRendering::FillType);

    /**
     * Draws a path. Fill must not be a hairline. It will respect the HW
     * antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     */
    void drawPath(GrPipelineBuilder*, const GrPathProcessor*, const GrPath*,
                  GrPathRendering::FillType);

    /**
     * Draws the aggregate path from combining multiple. Note that this will not
     * always be equivalent to back-to-back calls to drawPath(). It will respect
     * the HW antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     *
     * @param pathRange       Source paths to draw from
     * @param indices         Array of path indices to draw
     * @param indexType       Data type of the array elements in indexBuffer
     * @param transformValues Array of transforms for the individual paths
     * @param transformType   Type of transforms in transformBuffer
     * @param count           Number of paths to draw
     * @param fill            Fill type for drawing all the paths
     */
    void drawPaths(GrPipelineBuilder*,
                   const GrPathProcessor*,
                   const GrPathRange* pathRange,
                   const void* indices,
                   PathIndexType indexType,
                   const float transformValues[],
                   PathTransformType transformType,
                   int count,
                   GrPathRendering::FillType fill);

    /**
     * Helper function for drawing rects. It performs a geometry src push and pop
     * and thus will finalize any reserved geometry.
     *
     * @param rect        the rect to draw
     * @param localRect   optional rect that specifies local coords to map onto
     *                    rect. If NULL then rect serves as the local coords.
     * @param localMatrix Optional local matrix. The local coordinates are specified by localRect,
     *                    or if it is NULL by rect. This matrix applies to the coordinate implied by
     *                    that rectangle before it is input to GrCoordTransforms that read local
     *                    coordinates
     */
    void drawRect(GrPipelineBuilder* pipelineBuilder,
                  GrColor color,
                  const SkMatrix& viewMatrix,
                  const SkRect& rect,
                  const SkRect* localRect,
                  const SkMatrix* localMatrix) {
        this->onDrawRect(pipelineBuilder, color, viewMatrix, rect, localRect, localMatrix);
    }

    /**
     * Helper for drawRect when the caller doesn't need separate local rects or matrices.
     */
    void drawSimpleRect(GrPipelineBuilder* ds, GrColor color, const SkMatrix& viewM,
                        const SkRect& rect) {
        this->drawRect(ds, color, viewM, rect, NULL, NULL);
    }
    void drawSimpleRect(GrPipelineBuilder* ds, GrColor color, const SkMatrix& viewM,
                        const SkIRect& irect) {
        SkRect rect = SkRect::Make(irect);
        this->drawRect(ds, color, viewM, rect, NULL, NULL);
    }

    /**
     * This call is used to draw multiple instances of some geometry with a
     * given number of vertices (V) and indices (I) per-instance. The indices in
     * the index source must have the form i[k+I] == i[k] + V. Also, all indices
     * i[kI] ... i[(k+1)I-1] must be elements of the range kV ... (k+1)V-1. As a
     * concrete example, the following index buffer for drawing a series of
     * quads each as two triangles each satisfies these conditions with V=4 and
     * I=6:
     *      (0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11, ...)
     *
     * The call assumes that the pattern of indices fills the entire index
     * source. The size of the index buffer limits the number of instances that
     * can be drawn by the GPU in a single draw. However, the caller may specify
     * any (positive) number for instanceCount and if necessary multiple GPU
     * draws will be issued. Moreover, when drawIndexedInstances is called
     * multiple times it may be possible for GrDrawTarget to group them into a
     * single GPU draw.
     *
     * @param type          the type of primitives to draw
     * @param instanceCount the number of instances to draw. Each instance
     *                      consists of verticesPerInstance vertices indexed by
     *                      indicesPerInstance indices drawn as the primitive
     *                      type specified by type.
     * @param verticesPerInstance   The number of vertices in each instance (V
     *                              in the above description).
     * @param indicesPerInstance    The number of indices in each instance (I
     *                              in the above description).
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawIndexedInstances(GrPipelineBuilder*,
                              const GrGeometryProcessor*,
                              GrPrimitiveType type,
                              int instanceCount,
                              int verticesPerInstance,
                              int indicesPerInstance,
                              const SkRect* devBounds = NULL);

    /**
     * Clear the passed in render target. Ignores the GrPipelineBuilder and clip. Clears the whole
     * thing if rect is NULL, otherwise just the rect. If canIgnoreRect is set then the entire
     * render target can be optionally cleared.
     */
    void clear(const SkIRect* rect,
               GrColor color,
               bool canIgnoreRect,
               GrRenderTarget* renderTarget);

    /**
     * Discards the contents render target.
     **/
    virtual void discard(GrRenderTarget*) = 0;

    /**
     * Called at start and end of gpu trace marking
     * GR_CREATE_GPU_TRACE_MARKER(marker_str, target) will automatically call these at the start
     * and end of a code block respectively
     */
    void addGpuTraceMarker(const GrGpuTraceMarker* marker);
    void removeGpuTraceMarker(const GrGpuTraceMarker* marker);

    /**
     * Takes the current active set of markers and stores them for later use. Any current marker
     * in the active set is removed from the active set and the targets remove function is called.
     * These functions do not work as a stack so you cannot call save a second time before calling
     * restore. Also, it is assumed that when restore is called the current active set of markers
     * is empty. When the stored markers are added back into the active set, the targets add marker
     * is called.
     */
    void saveActiveTraceMarkers();
    void restoreActiveTraceMarkers();

    /**
     * Copies a pixel rectangle from one surface to another. This call may finalize
     * reserved vertex/index data (as though a draw call was made). The src pixels
     * copied are specified by srcRect. They are copied to a rect of the same
     * size in dst with top left at dstPoint. If the src rect is clipped by the
     * src bounds then  pixel values in the dst rect corresponding to area clipped
     * by the src rect are not overwritten. This method can fail and return false
     * depending on the type of surface, configs, etc, and the backend-specific
     * limitations. If rect is clipped out entirely by the src or dst bounds then
     * true is returned since there is no actual copy necessary to succeed.
     */
    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);
    /**
     * Function that determines whether a copySurface call would succeed without actually
     * performing the copy.
     */
    bool canCopySurface(const GrSurface* dst,
                        const GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint);

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    virtual void purgeResources() {};

    ////////////////////////////////////////////////////////////////////////////

    class AutoReleaseGeometry : public ::SkNoncopyable {
    public:
        AutoReleaseGeometry(GrDrawTarget*  target,
                            int            vertexCount,
                            size_t         vertexStride,
                            int            indexCount);
        AutoReleaseGeometry();
        ~AutoReleaseGeometry();
        bool set(GrDrawTarget*  target,
                 int            vertexCount,
                 size_t         vertexStride,
                 int            indexCount);
        bool succeeded() const { return SkToBool(fTarget); }
        void* vertices() const { SkASSERT(this->succeeded()); return fVertices; }
        void* indices() const { SkASSERT(this->succeeded()); return fIndices; }
        SkPoint* positions() const {
            return static_cast<SkPoint*>(this->vertices());
        }

    private:
        void reset();

        GrDrawTarget* fTarget;
        void*         fVertices;
        void*         fIndices;
    };

    ////////////////////////////////////////////////////////////////////////////

    /**
     * Saves the geometry src state at construction and restores in the destructor. It also saves
     * and then restores the vertex attrib state.
     */
    class AutoGeometryPush : public ::SkNoncopyable {
    public:
        AutoGeometryPush(GrDrawTarget* target) {
            SkASSERT(target);
            fTarget = target;
            target->pushGeometrySource();
        }

        ~AutoGeometryPush() { fTarget->popGeometrySource(); }

    private:
        GrDrawTarget*                           fTarget;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Draw execution tracking (for font atlases and other resources)
    class DrawToken {
    public:
        DrawToken(GrDrawTarget* drawTarget, uint32_t drawID) :
                  fDrawTarget(drawTarget), fDrawID(drawID) {}

        bool isIssued() { return fDrawTarget && fDrawTarget->isIssued(fDrawID); }

    private:
        GrDrawTarget*  fDrawTarget;
        uint32_t       fDrawID;   // this may wrap, but we're doing direct comparison
                                  // so that should be okay
    };

    virtual DrawToken getCurrentDrawToken() { return DrawToken(this, 0); }

    /**
     * Used to communicate draws to GPUs / subclasses
     */
    class DrawInfo {
    public:
        DrawInfo() { fDevBounds = NULL; }
        DrawInfo(const DrawInfo& di) { (*this) = di; }
        DrawInfo& operator =(const DrawInfo& di);

        GrPrimitiveType primitiveType() const { return fPrimitiveType; }
        int startVertex() const { return fStartVertex; }
        int startIndex() const { return fStartIndex; }
        int vertexCount() const { return fVertexCount; }
        int indexCount() const { return fIndexCount; }
        int verticesPerInstance() const { return fVerticesPerInstance; }
        int indicesPerInstance() const { return fIndicesPerInstance; }
        int instanceCount() const { return fInstanceCount; }

        void setPrimitiveType(GrPrimitiveType type) { fPrimitiveType = type; }
        void setStartVertex(int startVertex) { fStartVertex = startVertex; }
        void setStartIndex(int startIndex) { fStartIndex = startIndex; }
        void setVertexCount(int vertexCount) { fVertexCount = vertexCount; }
        void setIndexCount(int indexCount) { fIndexCount = indexCount; }
        void setVerticesPerInstance(int verticesPerI) { fVerticesPerInstance = verticesPerI; }
        void setIndicesPerInstance(int indicesPerI) { fIndicesPerInstance = indicesPerI; }
        void setInstanceCount(int instanceCount) { fInstanceCount = instanceCount; }

        bool isIndexed() const { return fIndexCount > 0; }
#ifdef SK_DEBUG
        bool isInstanced() const; // this version is longer because of asserts
#else
        bool isInstanced() const { return fInstanceCount > 0; }
#endif

        // adds or remove instances
        void adjustInstanceCount(int instanceOffset);
        // shifts the start vertex
        void adjustStartVertex(int vertexOffset);
        // shifts the start index
        void adjustStartIndex(int indexOffset);

        void setDevBounds(const SkRect& bounds) {
            fDevBoundsStorage = bounds;
            fDevBounds = &fDevBoundsStorage;
        }
        const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
        const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }
        void setVertexBuffer(const GrVertexBuffer* vb) {
            fVertexBuffer.reset(vb);
        }
        void setIndexBuffer(const GrIndexBuffer* ib) {
            fIndexBuffer.reset(ib);
        }
        const SkRect* getDevBounds() const { return fDevBounds; }

    private:
        friend class GrDrawTarget;

        GrPrimitiveType         fPrimitiveType;

        int                     fStartVertex;
        int                     fStartIndex;
        int                     fVertexCount;
        int                     fIndexCount;

        int                     fInstanceCount;
        int                     fVerticesPerInstance;
        int                     fIndicesPerInstance;

        SkRect                  fDevBoundsStorage;
        SkRect*                 fDevBounds;

        GrPendingIOResource<const GrVertexBuffer, kRead_GrIOType> fVertexBuffer;
        GrPendingIOResource<const GrIndexBuffer, kRead_GrIOType>  fIndexBuffer;
    };

    /**
     * Used to populate the vertex and index buffer on the draw info before onDraw is called.
     */
    virtual void setDrawBuffers(DrawInfo*, size_t vertexStride) = 0;;
    bool programUnitTest(int maxStages);

protected:
    friend class GrTargetCommands; // for PipelineInfo

    enum GeometrySrcType {
        kNone_GeometrySrcType,     //<! src has not been specified
        kReserved_GeometrySrcType, //<! src was set using reserve*Space
        kBuffer_GeometrySrcType    //<! src was set using set*SourceToBuffer
    };

    struct GeometrySrcState {
        GeometrySrcType         fVertexSrc;
        union {
            // valid if src type is buffer
            const GrVertexBuffer*   fVertexBuffer;
            // valid if src type is reserved or array
            int                     fVertexCount;
        };

        GeometrySrcType         fIndexSrc;
        union {
            // valid if src type is buffer
            const GrIndexBuffer*    fIndexBuffer;
            // valid if src type is reserved or array
            int                     fIndexCount;
        };

        size_t                  fVertexSize;
    };

    int indexCountInCurrentSource() const {
        const GeometrySrcState& src = this->getGeomSrc();
        switch (src.fIndexSrc) {
            case kNone_GeometrySrcType:
                return 0;
            case kReserved_GeometrySrcType:
                return src.fIndexCount;
            case kBuffer_GeometrySrcType:
                return static_cast<int>(src.fIndexBuffer->gpuMemorySize() / sizeof(uint16_t));
            default:
                SkFAIL("Unexpected Index Source.");
                return 0;
        }
    }

    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    // subclasses must call this in their destructors to ensure all vertex
    // and index sources have been released (including those held by
    // pushGeometrySource())
    void releaseGeometry();

    // accessors for derived classes
    const GeometrySrcState& getGeomSrc() const { return fGeoSrcStateStack.back(); }
    // it is preferable to call this rather than getGeomSrc()->fVertexSize because of the assert.
    size_t getVertexSize() const {
        // the vertex layout is only valid if a vertex source has been specified.
        SkASSERT(this->getGeomSrc().fVertexSrc != kNone_GeometrySrcType);
        return this->getGeomSrc().fVertexSize;
    }

    // Subclass must initialize this in its constructor.
    SkAutoTUnref<const GrDrawTargetCaps> fCaps;

    const GrTraceMarkerSet& getActiveTraceMarkers() { return fActiveTraceMarkers; }

    // Makes a copy of the dst if it is necessary for the draw. Returns false if a copy is required
    // but couldn't be made. Otherwise, returns true.  This method needs to be protected because it
    // needs to be accessed by GLPrograms to setup a correct drawstate
    bool setupDstReadIfNecessary(const GrPipelineBuilder&,
                                 const GrProcOptInfo& colorPOI,
                                 const GrProcOptInfo& coveragePOI,
                                 GrDeviceCoordTexture* dstCopy,
                                 const SkRect* drawBounds);

    struct PipelineInfo {
        PipelineInfo(GrPipelineBuilder* pipelineBuilder, GrScissorState* scissor,
                     const GrPrimitiveProcessor* primProc,
                     const SkRect* devBounds, GrDrawTarget* target);

        PipelineInfo(GrPipelineBuilder* pipelineBuilder, GrScissorState* scissor,
                     const GrBatch* batch, const SkRect* devBounds,
                     GrDrawTarget* target);

        bool willBlendWithDst(const GrPrimitiveProcessor* primProc) const {
            return fPipelineBuilder->willBlendWithDst(primProc);
        }
    private:
        friend class GrDrawTarget;

        bool mustSkipDraw() const { return (NULL == fPipelineBuilder); }

        GrPipelineBuilder*      fPipelineBuilder;
        GrScissorState*         fScissor;
        GrProcOptInfo           fColorPOI; 
        GrProcOptInfo           fCoveragePOI; 
        GrDeviceCoordTexture    fDstCopy;
    };

    void setupPipeline(const PipelineInfo& pipelineInfo, GrPipeline* pipeline);

    // A subclass can optionally overload this function to be notified before
    // vertex and index space is reserved.
    virtual void willReserveVertexAndIndexSpace(int vertexCount,
                                                size_t vertexStride,
                                                int indexCount) {}

private:
    /**
     * This will be called before allocating a texture as a dst for copySurface. This function
     * populates the dstDesc's config, flags, and origin so as to maximize efficiency and guarantee
     * success of the copySurface call.
     */
    void initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* dstDesc) {
        if (!this->onInitCopySurfaceDstDesc(src, dstDesc)) {
            dstDesc->fOrigin = kDefault_GrSurfaceOrigin;
            dstDesc->fFlags = kRenderTarget_GrSurfaceFlag;
            dstDesc->fConfig = src->config();
        }
    }

    /** Internal implementation of canCopySurface. */
    bool internalCanCopySurface(const GrSurface* dst,
                                const GrSurface* src,
                                const SkIRect& clippedSrcRect,
                                const SkIPoint& clippedDstRect);

    // implemented by subclass to allocate space for reserved geom
    virtual bool onReserveVertexSpace(size_t vertexSize, int vertexCount, void** vertices) = 0;
    virtual bool onReserveIndexSpace(int indexCount, void** indices) = 0;
    // implemented by subclass to handle release of reserved geom space
    virtual void releaseReservedVertexSpace() = 0;
    virtual void releaseReservedIndexSpace() = 0;
    // subclass overrides to be notified just before geo src state is pushed/popped.
    virtual void geometrySourceWillPush() = 0;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) = 0;
    // subclass called to perform drawing
    virtual void onDraw(const GrGeometryProcessor*, const DrawInfo&, const PipelineInfo&) = 0;
    virtual void onDrawBatch(GrBatch*, const PipelineInfo&) = 0;
    // TODO copy in order drawbuffer onDrawRect to here
    virtual void onDrawRect(GrPipelineBuilder*,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) = 0;

    virtual void onStencilPath(const GrPipelineBuilder&,
                               const GrPathProcessor*,
                               const GrPath*,
                               const GrScissorState&,
                               const GrStencilSettings&) = 0;
    virtual void onDrawPath(const GrPathProcessor*,
                            const GrPath*,
                            const GrStencilSettings&,
                            const PipelineInfo&) = 0;
    virtual void onDrawPaths(const GrPathProcessor*,
                             const GrPathRange*,
                             const void* indices,
                             PathIndexType,
                             const float transformValues[],
                             PathTransformType,
                             int count,
                             const GrStencilSettings&,
                             const PipelineInfo&) = 0;

    virtual void onClear(const SkIRect* rect, GrColor color, bool canIgnoreRect,
                         GrRenderTarget* renderTarget) = 0;

    /** The subclass will get a chance to copy the surface for falling back to the default
        implementation, which simply draws a rectangle (and fails if dst isn't a render target). It
        should assume that any clipping has already been performed on the rect and point. It won't
        be called if the copy can be skipped. */
    virtual bool onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    /** Indicates whether onCopySurface would succeed. It should assume that any clipping has
        already been performed on the rect and point. It won't be called if the copy can be
        skipped. */
    virtual bool onCanCopySurface(const GrSurface* dst,
                                  const GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) = 0;
    /**
     * This will be called before allocating a texture to be a dst for onCopySurface. Only the
     * dstDesc's config, flags, and origin need be set by the function. If the subclass cannot
     * create a surface that would succeed its implementation of onCopySurface, it should return
     * false. The base class will fall back to creating a render target to draw into using the src.
     */
    virtual bool onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* dstDesc) = 0;

    // helpers for reserving vertex and index space.
    bool reserveVertexSpace(size_t vertexSize,
                            int vertexCount,
                            void** vertices);
    bool reserveIndexSpace(int indexCount, void** indices);

    // called by drawIndexed and drawNonIndexed. Use a negative indexCount to
    // indicate non-indexed drawing.
    bool checkDraw(const GrPipelineBuilder&,
                   const GrGeometryProcessor*,
                   GrPrimitiveType type,
                   int startVertex,
                   int startIndex,
                   int vertexCount,
                   int indexCount) const;
    // called when setting a new vert/idx source to unref prev vb/ib
    void releasePreviousVertexSource();
    void releasePreviousIndexSource();

    // Check to see if this set of draw commands has been sent out
    virtual bool       isIssued(uint32_t drawID) { return true; }
    void getPathStencilSettingsForFilltype(GrPathRendering::FillType,
                                           const GrStencilBuffer*,
                                           GrStencilSettings*);
    virtual GrClipMaskManager* clipMaskManager() = 0;
    virtual bool setupClip(GrPipelineBuilder*,
                           GrPipelineBuilder::AutoRestoreFragmentProcessors*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState*,
                           const SkRect* devBounds) = 0;

    enum {
        kPreallocGeoSrcStateStackCnt = 4,
    };
    SkSTArray<kPreallocGeoSrcStateStackCnt, GeometrySrcState, true> fGeoSrcStateStack;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by DrawTarget.
    GrContext*                                                      fContext;
    // To keep track that we always have at least as many debug marker adds as removes
    int                                                             fGpuTraceMarkerCount;
    GrTraceMarkerSet                                                fActiveTraceMarkers;
    GrTraceMarkerSet                                                fStoredTraceMarkers;

    typedef SkRefCnt INHERITED;
};

/*
 * This class is JUST for clip mask manager.  Everyone else should just use draw target above.
 */
class GrClipTarget : public GrDrawTarget {
public:
    GrClipTarget(GrContext* context) : INHERITED(context) {
        fClipMaskManager.setClipTarget(this);
    }

    /* Clip mask manager needs access to the context.
     * TODO we only need a very small subset of context in the CMM.
     */
    GrContext* getContext() { return INHERITED::getContext(); }
    const GrContext* getContext() const { return INHERITED::getContext(); }

    /**
     * Clip Mask Manager(and no one else) needs to clear private stencil bits.
     * ClipTarget subclass sets clip bit in the stencil buffer. The subclass
     * is free to clear the remaining bits to zero if masked clears are more
     * expensive than clearing all bits.
     */
    virtual void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* = NULL) = 0;

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    void purgeResources() SK_OVERRIDE {
        // The clip mask manager can rebuild all its clip masks so just
        // get rid of them all.
        fClipMaskManager.purgeResources();
    };

protected:
    GrClipMaskManager           fClipMaskManager;

private:
    GrClipMaskManager* clipMaskManager() SK_OVERRIDE { return &fClipMaskManager; }

    virtual bool setupClip(GrPipelineBuilder*,
                           GrPipelineBuilder::AutoRestoreFragmentProcessors*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState* scissorState,
                           const SkRect* devBounds) SK_OVERRIDE;

    typedef GrDrawTarget INHERITED;
};

#endif
