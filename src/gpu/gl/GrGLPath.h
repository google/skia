
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

class GrGpuGL;
struct GrGLInterface;

/**
 * Currently this represents a path built using GL_NV_path_rendering. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPath : public GrPath {
public:
    static void InitPathObject(const GrGLInterface*,
                               GrGLuint pathID,
                               const SkPath&,
                               const SkStrokeRec&);

    GrGLPath(GrGpuGL* gpu, const SkPath& path, const SkStrokeRec& stroke);
    virtual ~GrGLPath();
    GrGLuint pathID() const { return fPathID; }
    // TODO: Figure out how to get an approximate size of the path in Gpu
    // memory.
    virtual size_t gpuMemorySize() const SK_OVERRIDE { return 100; }

protected:
    virtual void onRelease() SK_OVERRIDE;
    virtual void onAbandon() SK_OVERRIDE;

private:
    GrGLuint fPathID;

    typedef GrPath INHERITED;
};

#endif
