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
class GrGLSLFragmentProcessor;
class GrInvariantOutput;
class GrProcessorKeyBuilder;

/** Provides custom fragment shader code. Fragment processors receive an input color (vec4f) and
    produce an output color. They may reference textures and uniforms. They may use
    GrCoordTransforms to receive a transformation of the local coordinates that map from local space
    to the fragment being processed.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    /**
    *  In many instances (e.g. SkShader::asFragmentProcessor() implementations) it is desirable to
    *  only consider the input color's alpha. However, there is a competing desire to have reusable
    *  GrFragmentProcessor subclasses that can be used in other scenarios where the entire input
    *  color is considered. This function exists to filter the input color and pass it to a FP. It
    *  does so by returning a parent FP that multiplies the passed in FPs output by the parent's
    *  input alpha. The passed in FP will not receive an input color.
    */
    static sk_sp<GrFragmentProcessor> MulOutputByInputAlpha(sk_sp<GrFragmentProcessor>);

    /**
     *  Similar to the above but it modulates the output r,g,b of the child processor by the input
     *  rgb and then multiplies all the components by the input alpha. This effectively modulates
     *  the child processor's premul color by a unpremul'ed input and produces a premul output
     */
    static sk_sp<GrFragmentProcessor> MulOutputByInputUnpremulColor(sk_sp<GrFragmentProcessor>);

    /**
     *  Returns a parent fragment processor that adopts the passed fragment processor as a child.
     *  The parent will ignore its input color and instead feed the passed in color as input to the
     *  child.
     */
    static sk_sp<GrFragmentProcessor> OverrideInput(sk_sp<GrFragmentProcessor>, GrColor);

    /**
     * Returns a fragment processor that runs the passed in array of fragment processors in a
     * series. The original input is passed to the first, the first's output is passed to the
     * second, etc. The output of the returned processor is the output of the last processor of the
     * series.
     *
     * The array elements with be moved.
     */
    static sk_sp<GrFragmentProcessor> RunInSeries(sk_sp<GrFragmentProcessor>*, int cnt);

    GrFragmentProcessor()
        : INHERITED()
        , fUsesLocalCoords(false)
        , fNumTexturesExclChildren(0)
        , fNumBuffersExclChildren(0)
        , fNumTransformsExclChildren(0) {}

    ~GrFragmentProcessor() override;

    GrGLSLFragmentProcessor* createGLSLInstance() const;

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
        this->onGetGLSLProcessorKey(caps, b);
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            fChildProcessors[i]->getGLSLProcessorKey(caps, b);
        }
    }

    int numTexturesExclChildren() const { return fNumTexturesExclChildren; }

    int numBuffersExclChildren() const { return fNumBuffersExclChildren; }

    int numTransformsExclChildren() const { return fNumTransformsExclChildren; }

    int numTransforms() const { return fCoordTransforms.count(); }

    /** Returns the coordinate transformation at index. index must be valid according to
        numTransforms(). */
    const GrCoordTransform& coordTransform(int index) const { return *fCoordTransforms[index]; }

    const SkTArray<const GrCoordTransform*, true>& coordTransforms() const {
        return fCoordTransforms;
    }

    void gatherCoordTransforms(SkTArray<const GrCoordTransform*, true>* outTransforms) const {
        if (!fCoordTransforms.empty()) {
            outTransforms->push_back_n(fCoordTransforms.count(), fCoordTransforms.begin());
        }
    }

    int numChildProcessors() const { return fChildProcessors.count(); }

    const GrFragmentProcessor& childProcessor(int index) const { return *fChildProcessors[index]; }

    /** Do any of the coordtransforms for this processor require local coords? */
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use getGLSLProcessorKey
     */
    bool isEqual(const GrFragmentProcessor& that, bool ignoreCoordTransforms) const;

    /**
     * This function is used to perform optimizations. When called the invarientOuput param
     * indicate whether the input components to this processor in the FS will have known values.
     * In inout the validFlags member is a bitfield of GrColorComponentFlags. The isSingleComponent
     * member indicates whether the input will be 1 or 4 bytes. The function updates the members of
     * inout to indicate known values of its output. A component of the color member only has
     * meaning if the corresponding bit in validFlags is set.
     */
    void computeInvariantOutput(GrInvariantOutput* inout) const {
        this->onComputeInvariantOutput(inout);
    }

protected:
    void addTextureAccess(const GrTextureAccess* textureAccess) override;
    void addBufferAccess(const GrBufferAccess*) override;

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
     * FragmentProcessor subclasses call this from their constructor to register any child
     * FragmentProcessors they have. This must be called AFTER all texture accesses and coord
     * transforms have been added.
     * This is for processors whose shader code will be composed of nested processors whose output
     * colors will be combined somehow to produce its output color.  Registering these child
     * processors will allow the ProgramBuilder to automatically handle their transformed coords and
     * texture accesses and mangle their uniform and output color names.
     */
    int registerChildProcessor(sk_sp<GrFragmentProcessor> child);

    /**
     * Subclass implements this to support getConstantColorComponents(...).
     *
     * Note: it's up to the subclass implementation to do any recursive call to compute the child
     * procs' output invariants; computeInvariantOutput will not be recursive.
     */
    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const = 0;

private:
    void notifyRefCntIsZero() const final;

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrFragmentProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLFragmentProcessor* onCreateGLSLInstance() const = 0;

    /** Implemented using GLFragmentProcessor::GenKey as described in this class's comment. */
    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const = 0;

    /**
     * Subclass implements this to support isEqual(). It will only be called if it is known that
     * the two processors are of the same subclass (i.e. they return the same object from
     * getFactory()). The processor subclass should not compare its coord transforms as that will
     * be performed automatically in the non-virtual isEqual().
     */
    virtual bool onIsEqual(const GrFragmentProcessor&) const = 0;

    bool hasSameTransforms(const GrFragmentProcessor&) const;

    bool                                       fUsesLocalCoords;

    /**
     * fCoordTransforms stores the transforms of this proc, followed by all the transforms of this
     * proc's children. In other words, each proc stores all the transforms of its subtree as if
     * they were collected using preorder traversal.
     *
     * Example:
     * Suppose we have frag proc A, who has two children B and D. B has a child C, and D has
     * two children E and F. Suppose procs A, B, C, D, E, F have 1, 2, 1, 1, 3, 2 transforms
     * respectively. The following shows what the fCoordTransforms array of each proc would contain:
     *
     *                                   (A)
     *                        [a1,b1,b2,c1,d1,e1,e2,e3,f1,f2]
     *                                  /    \
     *                                /        \
     *                            (B)           (D)
     *                        [b1,b2,c1]   [d1,e1,e2,e3,f1,f2]
     *                          /             /    \
     *                        /             /        \
     *                      (C)          (E)          (F)
     *                     [c1]      [e1,e2,e3]      [f1,f2]
     *
     * The same goes for fTextureAccesses with textures.
     */
    SkSTArray<4, const GrCoordTransform*, true> fCoordTransforms;
    int                                         fNumTexturesExclChildren;
    int                                         fNumBuffersExclChildren;
    int                                         fNumTransformsExclChildren;

    /**
     * This is not SkSTArray<1, sk_sp<GrFragmentProcessor>> because this class holds strong
     * references until notifyRefCntIsZero and then it holds pending executions.
     */
    SkSTArray<1, GrFragmentProcessor*, true>    fChildProcessors;

    typedef GrProcessor INHERITED;
};

#endif
