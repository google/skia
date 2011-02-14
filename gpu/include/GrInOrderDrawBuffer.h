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


#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrDrawTarget.h"
#include "GrAllocPool.h"
#include "GrAllocator.h"
#include "GrClip.h"

class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;

/**
 * GrInOrderDrawBuffer is an implementation of GrDrawTarget that queues up
 * draws for eventual playback into a GrGpu. In theory one draw buffer could
 * playback into another. When index or vertex buffers are used as geometry
 * sources it is the callers the draw buffer only holds references to the
 * buffers. It is the callers responsibility to ensure that the data is still
 * valid when the draw buffer is played back into a GrGpu. Similarly, it is the
 * caller's responsibility to ensure that all referenced textures, buffers,
 * and rendertargets are associated in the GrGpu object that the buffer is
 * played back into. The buffer requires VB and IB pools to store geometry.
 */

class GrInOrderDrawBuffer : public GrDrawTarget {
public:

    GrInOrderDrawBuffer(GrVertexBufferAllocPool* vertexPool,
                        GrIndexBufferAllocPool* indexPool);

    virtual ~GrInOrderDrawBuffer();

    void initializeDrawStateAndClip(const GrDrawTarget& target);

    virtual void drawIndexed(PrimitiveType primitiveType,
                             int startVertex,
                             int startIndex,
                             int vertexCount,
                             int indexCount);

    virtual void drawNonIndexed(PrimitiveType primitiveType,
                                int startVertex,
                                int vertexCount);

    virtual bool geometryHints(GrVertexLayout vertexLayout,
                               int* vertexCount,
                               int* indexCount) const;

    void reset();

    void playback(GrDrawTarget* target);

private:

    struct Draw {
        PrimitiveType           fPrimitiveType;
        int                     fStartVertex;
        int                     fStartIndex;
        int                     fVertexCount;
        int                     fIndexCount;
        bool                    fStateChanged;
        bool                    fClipChanged;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
    };

    virtual bool acquireGeometryHelper(GrVertexLayout vertexLayout,
                                       void**         vertices,
                                       void**         indices);
    virtual void releaseGeometryHelper();
    virtual void clipWillChange(const GrClip& clip);

    virtual void setVertexSourceToArrayHelper(const void* vertexArray,
                                              int vertexCount);

    virtual void setIndexSourceToArrayHelper(const void* indexArray,
                                             int indexCount);


    bool grabState();
    bool grabClip();

    GrTAllocator<Draw>              fDraws;
    // HACK: We currently do not hold refs on RTs in the saved draw states.
    // The reason is that in the GL implementation when a GrTexture is destroyed
    // that has an associated RT the RT is destroyed regardless of its ref count.
    // We need a third object that holds the shared GL ids and persists until
    // both reach ref count 0. (skia issue 122)
    GrTAllocator<SavedDrawState>    fStates;

    GrTAllocator<GrClip>            fClips;
    bool                            fClipChanged;

    GrVertexBufferAllocPool&        fVertexPool;
    const GrVertexBuffer*           fCurrPoolVertexBuffer;
    int                             fCurrPoolStartVertex;

    GrIndexBufferAllocPool&         fIndexPool;
    const GrIndexBuffer*            fCurrPoolIndexBuffer;
    int                             fCurrPoolStartIndex;

    // caller may conservatively over reserve vertices / indices.
    // we release unused space back to allocator if possible
    size_t                          fReservedVertexBytes;
    size_t                          fReservedIndexBytes;
    size_t                          fUsedReservedVertexBytes;
    size_t                          fUsedReservedIndexBytes;

    static const uint32_t           STATES_BLOCK_SIZE = 8;
    static const uint32_t           DRAWS_BLOCK_SIZE  = 8;
    static const uint32_t           CLIPS_BLOCK_SIZE  = 8;
    static const uint32_t           VERTEX_BLOCK_SIZE = 1 << 12;
    static const uint32_t           INDEX_BLOCK_SIZE  = 1 << 10;
    int8_t                          fDrawsStorage[sizeof(Draw) *
                                                  DRAWS_BLOCK_SIZE];
    int8_t                          fStatesStorage[sizeof(SavedDrawState) *
                                                   STATES_BLOCK_SIZE];
    int8_t                          fClipsStorage[sizeof(GrClip) *
                                                  CLIPS_BLOCK_SIZE];
};

#endif
