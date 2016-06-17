/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathRange_DEFINED
#define GrGLPathRange_DEFINED

#include "../GrPathRange.h"
#include "GrStyle.h"
#include "gl/GrGLTypes.h"

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
    GrGLPathRange(GrGLGpu*, PathGenerator*, const GrStyle&);

    /**
     * Initialize a GL path range from an existing range of pre-initialized GPU
     * path objects. This class assumes ownership of the GPU path objects and
     * will delete them when done.
     */
    GrGLPathRange(GrGLGpu*,
                  GrGLuint basePathID,
                  int numPaths,
                  size_t gpuMemorySize,
                  const GrStyle&);

    GrGLuint basePathID() const { return fBasePathID; }

    bool shouldStroke() const { return fShouldStroke; }
    bool shouldFill() const { return fShouldFill; }

protected:
    void onInitPath(int index, const SkPath&) const override;

    void onRelease() override;
    void onAbandon() override;

private:
    void init();
    size_t onGpuMemorySize() const override { return fGpuMemorySize; }

    const GrStyle fStyle;
    GrGLuint fBasePathID;
    mutable size_t fGpuMemorySize;
    bool fShouldStroke;
    bool fShouldFill;

    typedef GrPathRange INHERITED;
};

#endif
