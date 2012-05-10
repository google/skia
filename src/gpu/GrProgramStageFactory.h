/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramStageFactory_DEFINED
#define GrProgramStageFactory_DEFINED

#include "GrTypes.h"

class GrCustomStage;
class GrGLProgramStage;

/** Given a GrCustomStage of a particular type, creates the corresponding
    graphics-backend-specific GrProgramStage. Also tracks equivalence
    of shaders generated via stageKey().

    TODO: most of this class' subclasses are boilerplate and ought to be
    templateable?
*/

class GrProgramStageFactory {

public:

    virtual ~GrProgramStageFactory();

    /** Returns a short unique identifier for this subclass x its
        parameters. If the key differs, different shader code must
        be generated; if the key matches, shader code can be reused.
        0 == no custom stage. */
    virtual uint16_t stageKey(const GrCustomStage*);

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrCustomStage; caller is responsible for deleting
        the object. */
    virtual GrGLProgramStage* createGLInstance(GrCustomStage*) = 0;

protected:

    /** Disable default constructor - instances should be singletons
        with static factory functions: our test examples are all stateless,
        but we suspect that future implementations may want to cache data? */
    GrProgramStageFactory() { }
};


#endif
