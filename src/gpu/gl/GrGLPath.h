
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

/**
 * Currently this represents a path built using GL_NV_path_rendering. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPath : public GrPath {
public:
    /**
     * Initialize a GL path object with a given path and stroke.
     *
     * @return the approximate GPU memory size of the path object in bytes.
     */
    static size_t InitPathObject(GrGpuGL*,
                                 GrGLuint pathID,
                                 const SkPath&,
                                 const SkStrokeRec&);

    GrGLPath(GrGpuGL* gpu, const SkPath& path, const SkStrokeRec& stroke);
    virtual ~GrGLPath();
    GrGLuint pathID() const { return fPathID; }
    virtual size_t gpuMemorySize() const SK_OVERRIDE { return fGpuMemorySize; }

protected:
    virtual void onRelease() SK_OVERRIDE;
    virtual void onAbandon() SK_OVERRIDE;

private:
    GrGLuint fPathID;
    size_t fGpuMemorySize;

    typedef GrPath INHERITED;
};

#endif
