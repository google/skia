/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMesh_DEFINED
#define GrMesh_DEFINED

#include "GrBuffer.h"
#include "GrGpuResourceRef.h"

class GrNonInstancedMesh {
public:
    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int startVertex() const { return fStartVertex; }
    int startIndex() const { return fStartIndex; }
    int vertexCount() const { return fVertexCount; }
    int indexCount() const { return fIndexCount; }
    bool isIndexed() const { return fIndexCount > 0; }

    const GrBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
    const GrBuffer* indexBuffer() const { return fIndexBuffer.get(); }

protected:
    GrPrimitiveType         fPrimitiveType;
    int                     fStartVertex;
    int                     fStartIndex;
    int                     fVertexCount;
    int                     fIndexCount;
    GrPendingIOResource<const GrBuffer, kRead_GrIOType> fVertexBuffer;
    GrPendingIOResource<const GrBuffer, kRead_GrIOType> fIndexBuffer;
    friend class GrMesh;
};

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrBatch to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh : public GrNonInstancedMesh {
public:
    GrMesh() {}
    GrMesh(const GrMesh& di) { (*this) = di; }
    GrMesh& operator =(const GrMesh& di);

    void init(GrPrimitiveType primType, const GrBuffer* vertexBuffer, int startVertex,
                int vertexCount) {
        SkASSERT(vertexBuffer);
        SkASSERT(vertexCount);
        SkASSERT(startVertex >= 0);
        fPrimitiveType = primType;
        fVertexBuffer.reset(vertexBuffer);
        fIndexBuffer.reset(nullptr);
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
                        const GrBuffer* vertexBuffer,
                        const GrBuffer* indexBuffer,
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
                        const GrBuffer* vertexBuffer,
                        const GrBuffer* indexBuffer,
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
        const GrNonInstancedMesh* init(const GrMesh& mesh) {
            fMesh = &mesh;
            if (mesh.fInstanceCount <= mesh.fMaxInstancesPerDraw) {
                fInstancesRemaining = 0;
                // Note, this also covers the non-instanced case!
                return &mesh;
            }
            SkASSERT(mesh.isInstanced());
            fInstanceBatch.fIndexBuffer.reset(mesh.fIndexBuffer.get());
            fInstanceBatch.fVertexBuffer.reset(mesh.fVertexBuffer.get());
            fInstanceBatch.fIndexCount = mesh.fMaxInstancesPerDraw *
                                         mesh.fIndicesPerInstance;
            fInstanceBatch.fVertexCount = mesh.fMaxInstancesPerDraw *
                                          mesh.fVerticesPerInstance;
            fInstanceBatch.fPrimitiveType = mesh.fPrimitiveType;
            fInstanceBatch.fStartIndex = mesh.fStartIndex;
            fInstanceBatch.fStartVertex = mesh.fStartVertex;
            fInstancesRemaining = mesh.fInstanceCount - mesh.fMaxInstancesPerDraw;
            return &fInstanceBatch;
        }

        const GrNonInstancedMesh* next() {
            if (!fInstancesRemaining) {
                return nullptr;
            }
            fInstanceBatch.fStartVertex += fInstanceBatch.fVertexCount;
            int instances = SkTMin(fInstancesRemaining, fMesh->fMaxInstancesPerDraw);
            fInstanceBatch.fIndexCount = instances * fMesh->fIndicesPerInstance;
            fInstanceBatch.fVertexCount = instances * fMesh->fVerticesPerInstance;
            fInstancesRemaining -= instances;
            return &fInstanceBatch;
        }
    private:
        GrNonInstancedMesh fInstanceBatch;
        const GrMesh* fMesh;
        int fInstancesRemaining;
    };

private:
    int                     fInstanceCount;
    int                     fVerticesPerInstance;
    int                     fIndicesPerInstance;
    int                     fMaxInstancesPerDraw;
};

#endif
