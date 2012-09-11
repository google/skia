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
#include "GrCustomStageUnitTest.h"
#include "GrTextureAccess.h"

class GrContext;
class GrTexture;
class SkString;

/** Provides custom vertex shader, fragment shader, uniform data for a
    particular stage of the Ganesh shading pipeline.
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
    GrCustomStage objects *must* be immutable: after being constructed,
    their fields may not change.  (Immutability isn't actually required
    until they've been used in a draw call, but supporting that would require
    setters and getters that could fail, copy-on-write, or deep copying of these
    objects when they're stored by a GrGLProgramStage.)
  */
class GrCustomStage : public GrRefCnt {

public:
    SK_DECLARE_INST_COUNT(GrCustomStage)

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

    /** Returns true if the other custom stage will generate identical output.
        Must only be called if the two are already known to be of the
        same type (i.e.  they return the same value from getFactory()).

        Equality is not the same thing as equivalence.
        To test for equivalence (that they will generate the same
        shader code, but may have different uniforms), check equality
        of the stageKey produced by the GrProgramStageFactory:
        a.getFactory().glStageKey(a) == b.getFactory().glStageKey(b).

        The default implementation of this function returns true iff
        the two stages have the same return value for numTextures() and
        for texture() over all valid indicse.
     */
    virtual bool isEqual(const GrCustomStage&) const;

    /** Human-meaningful string to identify this effect; may be embedded
        in generated shader code. */
    const char* name() const { return this->getFactory().name(); }

    virtual int numTextures() const;

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    virtual const GrTextureAccess& textureAccess(int index) const;

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    void* operator new(size_t size);
    void operator delete(void* target);

private:
    typedef GrRefCnt INHERITED;
};

#endif
