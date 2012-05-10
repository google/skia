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
class GrProgramStageFactory;

/** Provides custom vertex shader, fragment shader, uniform data for a
    particular stage of the Ganesh shading pipeline.
    TODO: may want to refcount these? */
class GrCustomStage : public GrRefCnt {

public:

    GrCustomStage();
    virtual ~GrCustomStage();

    /** Human-meaningful string to identify this effect; may be embedded
        in generated shader code. */
    virtual const char* name() const = 0;

    /** If given an input texture that is/is not opaque, is this
        stage guaranteed to produce an opaque output? */
    virtual bool isOpaque(bool inputTextureIsOpaque) const;

    /** This pointer, besides creating back-end-specific helper
        objects, is used for run-time-type-identification. Every
        subclass must return a consistent unique value for it. */
    virtual GrProgramStageFactory* getFactory() const = 0;

    /** Returns true if the other custom stage will generate
        equal output.
        Must only be called if the two are already known to be of the
        same type (i.e.  they return the same value from getFactory()).
        For equivalence (that they will generate the same
        shader, but perhaps have different uniforms), check equality
        of the stageKey produced by the GrProgramStageFactory. */
    virtual bool isEqual(const GrCustomStage *) const = 0;

private:

    typedef GrRefCnt INHERITED;
};

#endif
