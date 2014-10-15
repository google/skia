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

/** Provides custom fragment shader code. Fragment processors receive an input color (vec4f) and
    produce an output color. They may reference textures and uniforms. They may use
    GrCoordTransforms to receive a transformation of the local coordinates that map from local space
    to the fragment being processed.
 */
class GrFragmentProcessor : public GrProcessor {
public:
    GrFragmentProcessor()
        : INHERITED()
        , fWillReadDstColor(false)
        , fWillUseInputColor(true) {}

    virtual const GrBackendFragmentProcessorFactory& getFactory() const = 0;

    int numTransforms() const { return fCoordTransforms.count(); }

    /** Returns the coordinate transformation at index. index must be valid according to
        numTransforms(). */
    const GrCoordTransform& coordTransform(int index) const { return *fCoordTransforms[index]; }

    /** Will this prceossor read the destination pixel value? */
    bool willReadDstColor() const { return fWillReadDstColor; }

    /** Will this prceossor read the source color value? */
    bool willUseInputColor() const { return fWillUseInputColor; }

    /** Returns true if this and other prceossor conservatively draw identically. It can only return
        true when the two prceossor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the prceossor would
        generate the same shader code. To test for identical code generation use the prceossor' keys
        computed by the GrBackendProcessorFactory. */
    bool isEqual(const GrFragmentProcessor& other) const {
        if (&this->getFactory() != &other.getFactory()) {
            return false;
        }
        bool result = this->onIsEqual(other);
#ifdef SK_DEBUG
        if (result) {
            this->assertTexturesEqual(other);
        }
#endif
        return result;
    }

protected:
    /**
     * Fragment Processor subclasses call this from their constructor to register coordinate
     * transformations. The processor subclass manages the lifetime of the transformations (this
     * function only stores a pointer). The GrCoordTransform is typically a member field of the
     * GrProcessor subclass. When the matrix has perspective, the transformed coordinates will have
     * 3 components. Otherwise they'll have 2. This must only be called from the constructor because
     * GrProcessors are immutable.
     */
    void addCoordTransform(const GrCoordTransform*);

    /**
     * If the prceossor subclass will read the destination pixel value then it must call this function
     * from its constructor. Otherwise, when its generated backend-specific prceossor class attempts
     * to generate code that reads the destination pixel it will fail.
     */
    void setWillReadDstColor() { fWillReadDstColor = true; }

    /**
     * If the prceossor will generate a result that does not depend on the input color value then it
     * must call this function from its constructor. Otherwise, when its generated backend-specific
     * code might fail during variable binding due to unused variables.
     */
    void setWillNotUseInputColor() { fWillUseInputColor = false; }

private:
    /** Subclass implements this to support isEqual(). It will only be called if it is known that
        the two prceossor are of the same subclass (i.e. they return the same object from
        getFactory()).*/
    virtual bool onIsEqual(const GrFragmentProcessor& other) const = 0;

    SkSTArray<4, const GrCoordTransform*, true>  fCoordTransforms;
    bool                                         fWillReadDstColor;
    bool                                         fWillUseInputColor;

    typedef GrProcessor INHERITED;
};

/**
 * This creates an effect outside of the effect memory pool. The effect's destructor will be called
 * at global destruction time. NAME will be the name of the created GrProcessor.
 */
#define GR_CREATE_STATIC_FRAGMENT_PROCESSOR(NAME, FP_CLASS, ARGS)                                 \
static SkAlignedSStorage<sizeof(FP_CLASS)> g_##NAME##_Storage;                                    \
static GrFragmentProcessor* NAME SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), FP_CLASS, ARGS);  \
static SkAutoTDestroy<GrFragmentProcessor> NAME##_ad(NAME);

#endif
