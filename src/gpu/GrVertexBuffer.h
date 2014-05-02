
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
    GrVertexBuffer(GrGpu* gpu, bool isWrapped, size_t gpuMemorySize, bool dynamic, bool cpuBacked)
        : INHERITED(gpu, isWrapped, gpuMemorySize, dynamic, cpuBacked) {}
private:
    typedef GrGeometryBuffer INHERITED;
};

#endif
