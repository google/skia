/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomStage_DEFINED
#define GrCustomStage_DEFINED

#include "GrRefCnt.h"
#include "GrNoncopyable.h"
#include "GrProgramStageFactory.h"

class GrContext;

/** Provides custom vertex shader, fragment shader, uniform data for a
    particular stage of the Ganesh shading pipeline. 
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
  */
class GrCustomStage : public GrRefCnt {

public:
    typedef GrProgramStageFactory::StageKey StageKey;

    GrCustomStage();
    virtual ~GrCustomStage();

    /** If given an input texture that is/is not opaque, is this
        stage guaranteed to produce an opaque output? */
    virtual bool isOpaque(bool inputTextureIsOpaque) const;

    /** This object, besides creating back-end-specific helper
        objects, is used for run-time-type-identification. The factory should be
        an instance of templated class, GrTProgramStageFactory. It is templated
        on the subclass of GrCustomStage. The subclass must have a nested type
        (or typedef) named GLProgramStage which will be the subclass of
        GrGLProgramStage created by the factory.

        Example:
        class MyCustomStage : public GrCustomStage {
        ...
            virtual const GrProgramStageFactory& getFactory() const 
                                                            SK_OVERRIDE {
                return GrTProgramStageFactory<MyCustomStage>::getInstance();
            }
        ...
        };
     */
    virtual const GrProgramStageFactory& getFactory() const = 0;

    /** Returns true if the other custom stage will generate
        equal output.
        Must only be called if the two are already known to be of the
        same type (i.e.  they return the same value from getFactory()).
        For equivalence (that they will generate the same
        shader, but perhaps have different uniforms), check equality
        of the stageKey produced by the GrProgramStageFactory. */
    virtual bool isEqual(const GrCustomStage *) const = 0;

     /** Human-meaningful string to identify this effect; may be embedded
         in generated shader code. */
    const char* name() const { return this->getFactory().name(); }

private:

    typedef GrRefCnt INHERITED;
};

#endif
