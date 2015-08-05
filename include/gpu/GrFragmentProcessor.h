/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessor_DEFINED
#define GrFragmentProcessor_DEFINED

#include "GrProcessor.h"

class GrCoordTransform;
class GrGLSLCaps;
class GrGLFragmentProcessor;
class GrProcessorKeyBuilder;

/** Provides custom fragment shader code. Fragment processors receive an input color (vec4f) and
    produce an output color. They may reference textures and uniforms. They may use
    GrCoordTransforms to receive a transformation of the local coordinates that map from local space
    to the fragment being processed.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    GrFragmentProcessor()
        : INHERITED()
        , fUsesLocalCoords(false) {}

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrFragmentProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLFragmentProcessor* createGLInstance() const = 0;

    /** Human-meaningful string to identify this GrFragmentProcessor; may be embedded
        in generated shader code. */
    virtual const char* name() const = 0;

    void getGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
        this->onGetGLProcessorKey(caps, b);
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            fChildProcessors[i]->getGLProcessorKey(caps, b);
        }
    }

    int numTransforms() const { return fCoordTransforms.count(); }

    int numTransformsIncludeChildProcs() const {
        int numTransforms = fCoordTransforms.count();
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            numTransforms += fChildProcessors[i]->numTransformsIncludeChildProcs();
        }
        return numTransforms;
    }

    /** Returns the coordinate transformation at index. index must be valid according to
        numTransforms(). */
    const GrCoordTransform& coordTransform(int index) const { return *fCoordTransforms[index]; }

    const SkTArray<const GrCoordTransform*, true>& coordTransforms() const {
        return fCoordTransforms;
    }

    /** Gather the coord transforms into an array. We use preorder traversal */
    void gatherCoordTransforms(SkTArray<const GrCoordTransform*, true>* outTransforms) const {
        SkASSERT(outTransforms);
        outTransforms->push_back_n(fCoordTransforms.count(), fCoordTransforms.begin());
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            fChildProcessors[i]->gatherCoordTransforms(outTransforms);
        }
    }

    int numChildProcessors() const { return fChildProcessors.count(); }

    GrFragmentProcessor* childProcessor(int index) const { return fChildProcessors[index]; }

    const SkTArray<GrFragmentProcessor*, false>& childProcessors() const {
        return fChildProcessors;
    }

    int numTexturesIncludeChildProcs() const {
        int numTextures = this->numTextures();
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            numTextures += fChildProcessors[i]->numTexturesIncludeChildProcs();
        }
        return numTextures;
    }

    /** Do any of the coordtransforms for this processor require local coords? */
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use getGLProcessorKey*/
    bool isEqual(const GrFragmentProcessor& that, bool ignoreCoordTransforms) const {
        if (this->classID() != that.classID() ||
            !this->hasSameTextureAccesses(that)) {
            return false;
        }
        if (ignoreCoordTransforms) {
            if (this->numTransforms() != that.numTransforms()) {
                return false;
            }
        } else if (!this->hasSameTransforms(that)) {
            return false;
        }
        return this->onIsEqual(that);
    }

    /**
     * This function is used to perform optimizations. When called the invarientOuput param
     * indicate whether the input components to this processor in the FS will have known values.
     * In inout the validFlags member is a bitfield of GrColorComponentFlags. The isSingleComponent
     * member indicates whether the input will be 1 or 4 bytes. The function updates the members of
     * inout to indicate known values of its output. A component of the color member only has
     * meaning if the corresponding bit in validFlags is set.
     */
    void computeInvariantOutput(GrInvariantOutput* inout) const;

protected:
    /**
     * Fragment Processor subclasses call this from their constructor to register coordinate
     * transformations. Coord transforms provide a mechanism for a processor to receive coordinates
     * in their FS code. The matrix expresses a transformation from local space. For a given
     * fragment the matrix will be applied to the local coordinate that maps to the fragment.
     *
     * When the transformation has perspective, the transformed coordinates will have
     * 3 components. Otherwise they'll have 2. 
     *
     * This must only be called from the constructor because GrProcessors are immutable. The
     * processor subclass manages the lifetime of the transformations (this function only stores a
     * pointer). The GrCoordTransform is typically a member field of the GrProcessor subclass. 
     *
     * A processor subclass that has multiple methods of construction should always add its coord
     * transforms in a consistent order. The non-virtual implementation of isEqual() automatically
     * compares transforms and will assume they line up across the two processor instances.
     */
    void addCoordTransform(const GrCoordTransform*);

    /**
     * FragmentProcessor subclasses call this to register any child FragmentProcessors they have.
     * This is for processors whose shader code will be composed of nested processors whose output
     * colors will be combined somehow to produce its output color.  Registering these child
     * processors will allow the ProgramBuilder to automatically add their transformed coords and
     * texture accesses and mangle their uniform and output color names and
     */
    void registerChildProcessor(GrFragmentProcessor* child);

    /**
     * Subclass implements this to support getConstantColorComponents(...).
     */
    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const = 0;

private:
    /** Implemented using GLFragmentProcessor::GenKey as described in this class's comment. */
    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const = 0;

    /**
     * Subclass implements this to support isEqual(). It will only be called if it is known that
     * the two processors are of the same subclass (i.e. they return the same object from
     * getFactory()). The processor subclass should not compare its coord transforms as that will
     * be performed automatically in the non-virtual isEqual().
     */
    virtual bool onIsEqual(const GrFragmentProcessor&) const = 0;

    bool hasSameTransforms(const GrFragmentProcessor&) const;

    SkSTArray<4, const GrCoordTransform*, true>  fCoordTransforms;
    bool                                         fUsesLocalCoords;
    SkTArray<GrFragmentProcessor*, false>        fChildProcessors;

    typedef GrProcessor INHERITED;
};

#endif
