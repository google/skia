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

// TODO: don't save clip per draw
class GrInOrderDrawBuffer : public GrDrawTarget {
public:

    GrInOrderDrawBuffer(GrVertexBufferAllocPool* pool = NULL);

    virtual ~GrInOrderDrawBuffer();

    void initializeDrawStateAndClip(const GrDrawTarget& target);

    virtual void drawIndexed(PrimitiveType type,
                             uint32_t startVertex,
                             uint32_t startIndex,
                             uint32_t vertexCount,
                             uint32_t indexCount);

    virtual void drawNonIndexed(PrimitiveType type,
                                uint32_t startVertex,
                                uint32_t vertexCount);

    virtual bool geometryHints(GrVertexLayout vertexLayout,
                               int32_t*       vertexCount,
                               int32_t*       indexCount) const;

    void reset();

    void playback(GrDrawTarget* target);

private:

    struct Draw {
        PrimitiveType   fType;
        uint32_t        fStartVertex;
        uint32_t        fStartIndex;
        uint32_t        fVertexCount;
        uint32_t        fIndexCount;
        bool            fStateChange;
        GrVertexLayout  fVertexLayout;
        bool            fUseVertexBuffer;
        bool            fClipChanged;
        union {
            const GrVertexBuffer*   fVertexBuffer;
            const void*             fVertexArray;
        };
        bool            fUseIndexBuffer;
        union {
            const GrIndexBuffer*    fIndexBuffer;
            const void*             fIndexArray;
        };
    };

    virtual bool acquireGeometryHelper(GrVertexLayout vertexLayout,
                                       void**         vertices,
                                       void**         indices);
    virtual void releaseGeometryHelper();
    virtual void clipWillChange(const GrClip& clip);


    bool grabState();
    bool grabClip();

    GrTAllocator<Draw>              fDraws;
    // HACK: We hold refs on textures in saved state but not RTs, VBs, and IBs.
    // a) RTs aren't ref counted (yet)
    // b) we are only using this class for text which doesn't use VBs or IBs
    // This should be fixed by either refcounting them all or having some
    // notification occur if a cache is purging an object we have a ptr to.
    GrTAllocator<SavedDrawState>    fStates;

    GrTAllocator<GrClip>            fClips;
    bool                            fClipChanged;

    // vertices are either queued in cpu arrays or some vertex buffer pool
    // that knows about a specific GrGpu object.
    GrAllocPool                     fCPUVertices;
    GrVertexBufferAllocPool*        fBufferVertices;
    GrAllocPool                     fIndices;
    void*                           fCurrReservedVertices;
    void*                       	fCurrReservedIndices;
    // valid if we're queueing vertices in fBufferVertices
    GrVertexBuffer*                 fCurrVertexBuffer;
    uint32_t                        fCurrStartVertex;

    // caller may conservatively over allocate vertices / indices.
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
