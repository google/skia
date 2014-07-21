
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

class GrGpuGL;

/**
 * Currently this represents a range of GL_NV_path_rendering Path IDs. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPathRange : public GrPathRange {
public:
    GrGLPathRange(GrGpu*, size_t size, const SkStrokeRec&);
    virtual ~GrGLPathRange();

    GrGLuint basePathID() const { return fBasePathID; }

    virtual void initAt(size_t index, const SkPath&);

    // TODO: Use a better approximation for the individual path sizes.
    virtual size_t gpuMemorySize() const SK_OVERRIDE {
        return 100 * fNumDefinedPaths;
    }

protected:
    virtual void onRelease() SK_OVERRIDE;
    virtual void onAbandon() SK_OVERRIDE;

private:
    GrGLuint fBasePathID;
    size_t fNumDefinedPaths;

    typedef GrPathRange INHERITED;
};

#endif
