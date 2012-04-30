/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomStage_DEFINED
#define GrCustomStage_DEFINED

#include "GrRefCnt.h"

class GrContext;
class GrGLProgramStageFactory;

/** Provides custom vertex shader, fragment shader, uniform data for a
    particular stage of the Ganesh shading pipeline.
    TODO: may want to refcount these? */
class GrCustomStage : public GrRefCnt {

public:

    GrCustomStage();
    virtual ~GrCustomStage();

    /** If given an input texture that is/is not opaque, is this
        stage guaranteed to produce an opaque output? */
    virtual bool isOpaque(bool inputTextureIsOpaque) const;

    /** This pointer, besides creating back-end-specific helper
        objects, is used for run-time-type-identification. Every
        subclass must return a consistent unique value for it. */
    virtual GrGLProgramStageFactory* getGLFactory() = 0;

};

#endif
