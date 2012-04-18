/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomStage_DEFINED
#define GrCustomStage_DEFINED

class GrContext;
class GrGLProgramStageFactory;

/** Provides custom vertex shader, fragment shader, uniform data for a
    particular stage of the Ganesh shading pipeline.
    TODO: may want to refcount these? */
class GrCustomStage {

public:

    GrCustomStage();
    virtual ~GrCustomStage();

    /** If given an input texture that is/is not opaque, is this
        stage guaranteed to produce an opaque output? */
    virtual bool isOpaque(bool inputTextureIsOpaque) const;

    virtual GrGLProgramStageFactory* getGLFactory() = 0;

protected:

};

#endif
