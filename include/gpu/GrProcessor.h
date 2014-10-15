/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "GrBackendProcessorFactory.h"
#include "GrColor.h"
#include "GrProcessorUnitTest.h"
#include "GrProgramElement.h"
#include "GrShaderVar.h"
#include "GrTextureAccess.h"
#include "GrTypesPriv.h"
#include "SkString.h"

class GrBackendProcessorFactory;
class GrContext;
class GrCoordTransform;

/** Provides custom vertex shader, fragment shader, uniform data for a particular stage of the
    Ganesh shading pipeline.
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
    GrProcessor objects *must* be immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    effect must reach 0 before the thread terminates and the pool is destroyed. To create a static
    effect use the macro GR_CREATE_STATIC_EFFECT declared below.
  */
class GrProcessor : public GrProgramElement {
public:
    SK_DECLARE_INST_COUNT(GrProcessor)

    virtual ~GrProcessor();

    struct InvariantOutput{
        InvariantOutput() : fColor(0), fValidFlags(0), fIsSingleComponent(false),
                            fNonMulStageFound(false) {}

        void mulByUnknownOpaqueColor() {
            if (this->isOpaque()) {
                fValidFlags = kA_GrColorComponentFlag;
                fIsSingleComponent = false;
            } else {
                // Since the current state is not opaque we no longer care if the color being
                // multiplied is opaque.
                this->mulByUnknownColor(); 
            }
        }

        void mulByUnknownColor() {
            if (this->hasZeroAlpha()) {
                this->internalSetToTransparentBlack();
            } else {
                this->internalSetToUnknown();
            }
        }

        void mulByUnknownAlpha() {
            if (this->hasZeroAlpha()) {
                this->internalSetToTransparentBlack();
            } else {
                // We don't need to change fIsSingleComponent in this case
                fValidFlags = 0;
            }
        }

        void invalidateComponents(uint8_t invalidateFlags) {
            fValidFlags &= ~invalidateFlags;
            fIsSingleComponent = false;
        }

        void setToTransparentBlack() {
            this->internalSetToTransparentBlack();
            fNonMulStageFound = true;
        }

        void setToOther(uint8_t validFlags, GrColor color) {
            fValidFlags = validFlags;
            fColor = color;
            fIsSingleComponent = false;
            fNonMulStageFound = true;
        }

        void setToUnknown() {
            this->internalSetToUnknown();
            fNonMulStageFound= true;
        }

        bool isOpaque() const {
            return ((fValidFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(fColor));
        }

        bool isSolidWhite() const {
            return (fValidFlags == kRGBA_GrColorComponentFlags && 0xFFFFFFFF == fColor);
        }

        GrColor color() const { return fColor; }
        uint8_t validFlags() const { return fValidFlags; }

        /**
         * If isSingleComponent is true, then the flag values for r, g, b, and a must all be the
         * same. If the flags are all set then all color components must be equal.
         */
        SkDEBUGCODE(void validate() const;)

    private:
        void internalSetToTransparentBlack() {
            fValidFlags = kRGBA_GrColorComponentFlags;
            fColor = 0;
            fIsSingleComponent = true;
        }

        void internalSetToUnknown() {
            fValidFlags = 0;
            fIsSingleComponent = false;
        }

        bool hasZeroAlpha() const {
            return ((fValidFlags & kA_GrColorComponentFlag) && 0 == GrColorUnpackA(fColor));
        }

        SkDEBUGCODE(bool colorComponentsAllEqual() const;)
        /**
         * If alpha is valid, check that any valid R,G,B values are <= A
         */
        SkDEBUGCODE(bool validPreMulColor() const;)

        // Friended class that have "controller" code which loop over stages calling
        // computeInvarianteOutput(). These controllers may need to manually adjust the internal
        // members of InvariantOutput
        friend class GrDrawState;
        friend class GrOptDrawState;
        friend class GrPaint;

        GrColor fColor;
        uint32_t fValidFlags;
        bool fIsSingleComponent;
        bool fNonMulStageFound;
    };

    /**
     * This function is used to perform optimizations. When called the invarientOuput param
     * indicate whether the input components to this effect in the FS will have known values.
     * In inout the validFlags member is a bitfield of GrColorComponentFlags. The isSingleComponent
     * member indicates whether the input will be 1 or 4 bytes. The function updates the members of
     * inout to indicate known values of its output. A component of the color member only has
     * meaning if the corresponding bit in validFlags is set.
     */
    void computeInvariantOutput(InvariantOutput* inout) const {
        this->onComputeInvariantOutput(inout);
#ifdef SK_DEBUG
        inout->validate();
#endif
    }

    /** This object, besides creating back-end-specific helper objects, is used for run-time-type-
        identification. The factory should be an instance of templated class,
        GrTBackendEffectFactory. It is templated on the subclass of GrProcessor. The subclass must
        have a nested type (or typedef) named GLProcessor which will be the subclass of
        GrGLProcessor created by the factory.

        Example:
        class MyCustomEffect : public GrProcessor {
        ...
            virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
                return GrTBackendEffectFactory<MyCustomEffect>::getInstance();
            }
        ...
        };
     */
    virtual const GrBackendProcessorFactory& getFactory() const = 0;

    /** Human-meaningful string to identify this effect; may be embedded
        in generated shader code. */
    const char* name() const;

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    /** Will this effect read the fragment position? */
    bool willReadFragmentPosition() const { return fWillReadFragmentPosition; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /**
      * Helper for down-casting to a GrProcessor subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

protected:
    /**
     * Subclasses call this from their constructor to register GrTextureAccesses. The effect
     * subclass manages the lifetime of the accesses (this function only stores a pointer). The
     * GrTextureAccess is typically a member field of the GrProcessor subclass. This must only be
     * called from the constructor because GrProcessors are immutable.
     */
    void addTextureAccess(const GrTextureAccess* textureAccess);

    GrProcessor()
        : fWillReadFragmentPosition(false) {}

    /**
     * If the effect will generate a backend-specific effect that will read the fragment position
     * in the FS then it must call this method from its constructor. Otherwise, the request to
     * access the fragment position will be denied.
     */
    void setWillReadFragmentPosition() { fWillReadFragmentPosition = true; }

    SkDEBUGCODE(void assertTexturesEqual(const GrProcessor& other) const;)

private:

    /** 
     * Subclass implements this to support getConstantColorComponents(...).
     */
    virtual void onComputeInvariantOutput(InvariantOutput* inout) const = 0;

    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    bool                                         fWillReadFragmentPosition;

    typedef GrProgramElement INHERITED;
};

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

    /** Will this effect read the destination pixel value? */
    bool willReadDstColor() const { return fWillReadDstColor; }

    /** Will this effect read the source color value? */
    bool willUseInputColor() const { return fWillUseInputColor; }

    /** Returns true if this and other effect conservatively draw identically. It can only return
        true when the two effects are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the effects would
        generate the same shader code. To test for identical code generation use the effects' keys
        computed by the GrBackendEffectFactory. */
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
     * If the effect subclass will read the destination pixel value then it must call this function
     * from its constructor. Otherwise, when its generated backend-specific effect class attempts
     * to generate code that reads the destination pixel it will fail.
     */
    void setWillReadDstColor() { fWillReadDstColor = true; }

    /**
     * If the effect will generate a result that does not depend on the input color value then it
     * must call this function from its constructor. Otherwise, when its generated backend-specific
     * code might fail during variable binding due to unused variables.
     */
    void setWillNotUseInputColor() { fWillUseInputColor = false; }

private:
    /** Subclass implements this to support isEqual(). It will only be called if it is known that
        the two effects are of the same subclass (i.e. they return the same object from
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
#define GR_CREATE_STATIC_FRAGMENT_PROCESSOR(NAME, EFFECT_CLASS, ARGS)                             \
static SkAlignedSStorage<sizeof(EFFECT_CLASS)> g_##NAME##_Storage;                                \
static GrFragmentProcessor*                                                                       \
NAME SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), EFFECT_CLASS, ARGS);                          \
static SkAutoTDestroy<GrFragmentProcessor> NAME##_ad(NAME);

#endif
