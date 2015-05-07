/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertices_DEFINED
#define GrVertices_DEFINED

#include "GrIndexBuffer.h"
#include "GrVertexBuffer.h"

class GrNonInstancedVertices {
public:
    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int startVertex() const { return fStartVertex; }
    int startIndex() const { return fStartIndex; }
    int vertexCount() const { return fVertexCount; }
    int indexCount() const { return fIndexCount; }
    bool isIndexed() const { return fIndexCount > 0; }

    const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
    const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }

protected:
    GrPrimitiveType         fPrimitiveType;
    int                     fStartVertex;
    int                     fStartIndex;
    int                     fVertexCount;
    int                     fIndexCount;
    GrPendingIOResource<const GrVertexBuffer, kRead_GrIOType> fVertexBuffer;
    GrPendingIOResource<const GrIndexBuffer, kRead_GrIOType>  fIndexBuffer;
    friend class GrVertices;
};

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrBatch to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrVertices : public GrNonInstancedVertices {
public:
    GrVertices() {}
    GrVertices(const GrVertices& di) { (*this) = di; }
    GrVertices& operator =(const GrVertices& di);

    void init(GrPrimitiveType primType, const GrVertexBuffer* vertexBuffer, int startVertex,
                int vertexCount) {
        SkASSERT(vertexBuffer);
        SkASSERT(vertexCount);
        SkASSERT(startVertex >= 0);
        fPrimitiveType = primType;
        fVertexBuffer.reset(vertexBuffer);
        fIndexBuffer.reset(NULL);
        fStartVertex = startVertex;
        fStartIndex = 0;
        fVertexCount = vertexCount;
        fIndexCount = 0;
        fInstanceCount = 0;
        fVerticesPerInstance = 0;
        fIndicesPerInstance = 0;
        fMaxInstancesPerDraw = 0;
    }

    void initIndexed(GrPrimitiveType primType,
                        const GrVertexBuffer* vertexBuffer,
                        const GrIndexBuffer* indexBuffer,
                        int startVertex,
                        int startIndex,
                        int vertexCount,
                        int indexCount) {
        SkASSERT(indexBuffer);
        SkASSERT(vertexBuffer);
        SkASSERT(indexCount);
        SkASSERT(vertexCount);
        SkASSERT(startIndex >= 0);
        SkASSERT(startVertex >= 0);
        fPrimitiveType = primType;
        fVertexBuffer.reset(vertexBuffer);
        fIndexBuffer.reset(indexBuffer);
        fStartVertex = startVertex;
        fStartIndex = startIndex;
        fVertexCount = vertexCount;
        fIndexCount = indexCount;
        fInstanceCount = 0;
        fVerticesPerInstance = 0;
        fIndicesPerInstance = 0;
        fMaxInstancesPerDraw = 0;
    }


    /** Variation of the above that may be used when the total number of instances may exceed
        the number of instances supported by the index buffer. To be used with
        nextInstances() to draw in max-sized batches.*/
    void initInstanced(GrPrimitiveType primType,
                        const GrVertexBuffer* vertexBuffer,
                        const GrIndexBuffer* indexBuffer,
                        int startVertex,
                        int verticesPerInstance,
                        int indicesPerInstance,
                        int instanceCount,
                        int maxInstancesPerDraw) {
        SkASSERT(vertexBuffer);
        SkASSERT(indexBuffer);
        SkASSERT(instanceCount);
        SkASSERT(verticesPerInstance);
        SkASSERT(indicesPerInstance);
        SkASSERT(startVertex >= 0);
        fPrimitiveType = primType;
        fVertexBuffer.reset(vertexBuffer);
        fIndexBuffer.reset(indexBuffer);
        fStartVertex = startVertex;
        fStartIndex = 0;
        fVerticesPerInstance = verticesPerInstance;
        fIndicesPerInstance = indicesPerInstance;
        fInstanceCount = instanceCount;
        fVertexCount = instanceCount * fVerticesPerInstance;
        fIndexCount = instanceCount * fIndicesPerInstance;
        fMaxInstancesPerDraw = maxInstancesPerDraw;
    }


    /** These return 0 if initInstanced was not used to initialize the GrVertices. */
    int verticesPerInstance() const { return fVerticesPerInstance; }
    int indicesPerInstance() const { return fIndicesPerInstance; }
    int instanceCount() const { return fInstanceCount; }

    bool isInstanced() const { return fInstanceCount > 0; }

    class Iterator {
    public:
        const GrNonInstancedVertices* init(const GrVertices& vertices) {
            fVertices = &vertices;
            if (vertices.fInstanceCount <= vertices.fMaxInstancesPerDraw) {
                fInstancesRemaining = 0;
                // Note, this also covers the non-instanced case!
                return &vertices;
            }
            SkASSERT(vertices.isInstanced());
            fInstanceBatch.fIndexBuffer.reset(vertices.fIndexBuffer.get());
            fInstanceBatch.fVertexBuffer.reset(vertices.fVertexBuffer.get());
            fInstanceBatch.fIndexCount = vertices.fMaxInstancesPerDraw *
                                         vertices.fIndicesPerInstance;
            fInstanceBatch.fVertexCount = vertices.fMaxInstancesPerDraw *
                                          vertices.fVerticesPerInstance;
            fInstanceBatch.fPrimitiveType = vertices.fPrimitiveType;
            fInstanceBatch.fStartIndex = vertices.fStartIndex;
            fInstanceBatch.fStartVertex = vertices.fStartVertex;
            fInstancesRemaining = vertices.fInstanceCount - vertices.fMaxInstancesPerDraw;
            return &fInstanceBatch;
        }

        const GrNonInstancedVertices* next() {
            if (!fInstancesRemaining) {
                return NULL;
            }
            fInstanceBatch.fStartVertex += fInstanceBatch.fVertexCount;
            int instances = SkTMin(fInstancesRemaining, fVertices->fMaxInstancesPerDraw);
            fInstanceBatch.fIndexCount = instances * fVertices->fIndicesPerInstance;
            fInstanceBatch.fVertexCount = instances * fVertices->fVerticesPerInstance;
            fInstancesRemaining -= instances;
            return &fInstanceBatch;
        }
    private:
        GrNonInstancedVertices fInstanceBatch;
        const GrVertices* fVertices;
        int fInstancesRemaining;
    };

private:
    int                     fInstanceCount;
    int                     fVerticesPerInstance;
    int                     fIndicesPerInstance;
    int                     fMaxInstancesPerDraw;
};

#endif
