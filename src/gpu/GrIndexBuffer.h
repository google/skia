
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrIndexBuffer_DEFINED
#define GrIndexBuffer_DEFINED

#include "GrGeometryBuffer.h"

class GrIndexBuffer : public GrGeometryBuffer {
public:
        /**
         * Retrieves the maximum number of quads that could be rendered
         * from the index buffer (using kTriangles_PrimitiveType).
         * @return the maximum number of quads using full size of index buffer.
         */
        int maxQuads() const {
            return this->sizeInBytes() / (sizeof(uint16_t) * 6);
        }
protected:
    GrIndexBuffer(GrGpu* gpu, size_t sizeInBytes, bool dynamic)
        : INHERITED(gpu, sizeInBytes, dynamic) {}
private:
    typedef GrGeometryBuffer INHERITED;
};

#endif
