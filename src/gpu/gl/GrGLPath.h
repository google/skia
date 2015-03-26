
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPath_DEFINED
#define GrGLPath_DEFINED

#include "../GrPath.h"
#include "gl/GrGLFunctions.h"

class GrGLGpu;

/**
 * Currently this represents a path built using GL_NV_path_rendering. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPath : public GrPath {
public:
    static void InitPathObject(GrGLGpu*,
                               GrGLuint pathID,
                               const SkPath&,
                               const SkStrokeRec&);

    GrGLPath(GrGLGpu* gpu, const SkPath& path, const SkStrokeRec& stroke);
    GrGLuint pathID() const { return fPathID; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    // TODO: Figure out how to get an approximate size of the path in Gpu memory.
    size_t onGpuMemorySize() const override { return 100; }

    GrGLuint fPathID;

    typedef GrPath INHERITED;
};

#endif
