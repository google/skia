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

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrBatch to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrVertices {
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
        fVertexBuffer.reset(SkRef(vertexBuffer));
        fIndexBuffer.reset(NULL);
        fStartVertex = startVertex;
        fStartIndex = 0;
        fVertexCount = vertexCount;
        fIndexCount = 0;
        fInstanceCount = 0;
        fVerticesPerInstance = 0;
        fIndicesPerInstance = 0;
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
        fVertexBuffer.reset(SkRef(vertexBuffer));
        fIndexBuffer.reset(SkRef(indexBuffer));
        fStartVertex = startVertex;
        fStartIndex = startIndex;
        fVertexCount = vertexCount;
        fIndexCount = indexCount;
        fInstanceCount = 0;
        fVerticesPerInstance = 0;
        fIndicesPerInstance = 0;
    }

    void initInstanced(GrPrimitiveType primType,
                        const GrVertexBuffer* vertexBuffer,
                        const GrIndexBuffer* indexBuffer,
                        int startVertex,
                        int verticesPerInstance,
                        int indicesPerInstance,
                        int instanceCount) {
        SkASSERT(vertexBuffer);
        SkASSERT(indexBuffer);
        SkASSERT(instanceCount);
        SkASSERT(verticesPerInstance);
        SkASSERT(indicesPerInstance);
        SkASSERT(startVertex >= 0);
        fPrimitiveType = primType;
        fVertexBuffer.reset(SkRef(vertexBuffer));
        fIndexBuffer.reset(SkRef(indexBuffer));
        fStartVertex = startVertex;
        fStartIndex = 0;
        fVerticesPerInstance = verticesPerInstance;
        fIndicesPerInstance = indicesPerInstance;
        fInstanceCount = instanceCount;
        fVertexCount = instanceCount * fVerticesPerInstance;
        fIndexCount = instanceCount * fIndicesPerInstance;
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
                        int* instancesRemaining,
                        int maxInstancesPerDraw) {
        int instanceCount = SkTMin(*instancesRemaining, maxInstancesPerDraw);
        *instancesRemaining -= instanceCount;
        this->initInstanced(primType, vertexBuffer, indexBuffer, startVertex,
                            verticesPerInstance, indicesPerInstance, instanceCount);
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int startVertex() const { return fStartVertex; }
    int startIndex() const { return fStartIndex; }
    int vertexCount() const { return fVertexCount; }
    int indexCount() const { return fIndexCount; }

    /** These return 0 if initInstanced was not used to initialize the GrVertices. */
    int verticesPerInstance() const { return fVerticesPerInstance; }
    int indicesPerInstance() const { return fIndicesPerInstance; }
    int instanceCount() const { return fInstanceCount; }

    bool isIndexed() const { return fIndexCount > 0; }
    bool isInstanced() const { return fInstanceCount > 0; }

    /** Called after using this draw info to draw the next set of instances.
        The vertex offset is advanced while the index buffer is reused at the same
        position. instancesRemaining is number of instances that remain, maxInstances is
        the most number of instances that can be used with the index buffer. If there
        are no instances remaining, the GrVertices is unmodified and false is returned.*/
    bool nextInstances(int* instancesRemaining, int maxInstances) {
        SkASSERT(this->isInstanced());
        if (!*instancesRemaining) {
            return false;
        }
        fStartVertex += fVertexCount;
        fInstanceCount = SkTMin(*instancesRemaining, maxInstances);
        fVertexCount = fInstanceCount * fVerticesPerInstance;
        fIndexCount = fInstanceCount * fIndicesPerInstance;
        *instancesRemaining -= fInstanceCount;
        return true;
    }

    const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
    const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }

private:
    GrPrimitiveType         fPrimitiveType;

    int                     fStartVertex;
    int                     fStartIndex;
    int                     fVertexCount;
    int                     fIndexCount;

    int                     fInstanceCount;
    int                     fVerticesPerInstance;
    int                     fIndicesPerInstance;

    GrPendingIOResource<const GrVertexBuffer, kRead_GrIOType> fVertexBuffer;
    GrPendingIOResource<const GrIndexBuffer, kRead_GrIOType>  fIndexBuffer;
};

#endif
