
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrVertexBuffer_DEFINED
#define GrVertexBuffer_DEFINED

#include "GrGeometryBuffer.h"

class GrVertexBuffer : public GrGeometryBuffer {
protected:
    GrVertexBuffer(GrGpu* gpu, bool isWrapped, size_t sizeInBytes, bool dynamic, bool cpuBacked)
        : INHERITED(gpu, isWrapped, sizeInBytes, dynamic, cpuBacked) {}
private:
    typedef GrGeometryBuffer INHERITED;
};

#endif
