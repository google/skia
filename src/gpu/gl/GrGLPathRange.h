
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathRange_DEFINED
#define GrGLPathRange_DEFINED

#include "../GrPathRange.h"
#include "gl/GrGLFunctions.h"

class GrGLGpu;

/**
 * Currently this represents a range of GL_NV_path_rendering Path IDs. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPathRange : public GrPathRange {
public:
    /**
     * Initialize a GL path range from a PathGenerator. This class will allocate
     * the GPU path objects and initialize them lazily.
     */
    GrGLPathRange(GrGLGpu*, PathGenerator*, const SkStrokeRec&);

    /**
     * Initialize a GL path range from an existing range of pre-initialized GPU
     * path objects. This class assumes ownership of the GPU path objects and
     * will delete them when done.
     */
    GrGLPathRange(GrGLGpu*,
                  GrGLuint basePathID,
                  int numPaths,
                  size_t gpuMemorySize,
                  const SkStrokeRec&);

    GrGLuint basePathID() const { return fBasePathID; }

protected:
    void onInitPath(int index, const SkPath&) const SK_OVERRIDE;

    void onRelease() SK_OVERRIDE;
    void onAbandon() SK_OVERRIDE;

private:
    size_t onGpuMemorySize() const SK_OVERRIDE { return fGpuMemorySize; }

    GrGLuint fBasePathID;
    mutable size_t fGpuMemorySize;

    typedef GrPathRange INHERITED;
};

#endif
