/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEffect_DEFINED
#define GrEffect_DEFINED

#include "GrColor.h"
#include "GrEffectUnitTest.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"
#include "GrTypesPriv.h"

class GrBackendEffectFactory;
class GrContext;
class GrCoordTransform;
class GrEffect;
class GrVertexEffect;
class SkString;

/** Provides custom vertex shader, fragment shader, uniform data for a particular stage of the
    Ganesh shading pipeline.
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
    GrEffect objects *must* be immutable: after being constructed, their fields may not change.

    Dynamically allocated GrEffects are managed by a per-thread memory pool. The ref count of an
    effect must reach 0 before the thread terminates and the pool is destroyed. To create a static
    effect use the macro GR_CREATE_STATIC_EFFECT declared below.
  */
class GrEffect : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrEffect)

    virtual ~GrEffect();

    /**
     * This function is used to perform optimizations. When called the color and validFlags params
     * indicate whether the input components to this effect in the FS will have known values.
     * validFlags is a bitfield of GrColorComponentFlags. The function updates both params to
     * indicate known values of its output. A component of the color param only has meaning if the
     * corresponding bit in validFlags is set.
     */
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const = 0;

    /** Will this effect read the source color value? */
    bool willUseInputColor() const { return fWillUseInputColor; }

    /** This object, besides creating back-end-specific helper objects, is used for run-time-type-
        identification. The factory should be an instance of templated class,
        GrTBackendEffectFactory. It is templated on the subclass of GrEffect. The subclass must have
        a nested type (or typedef) named GLEffect which will be the subclass of GrGLEffect created
        by the factory.

        Example:
        class MyCustomEffect : public GrEffect {
        ...
            virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
                return GrTBackendEffectFactory<MyCustomEffect>::getInstance();
            }
        ...
        };
     */
    virtual const GrBackendEffectFactory& getFactory() const = 0;

    /** Returns true if this and other effect conservatively draw identically. It can only return
        true when the two effects are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the effects would
        generate the same shader code. To test for identical code generation use the effects' keys
        computed by the GrBackendEffectFactory.
     */
    bool isEqual(const GrEffect& other) const {
        if (&this->getFactory() != &other.getFactory()) {
            return false;
        }
        bool result = this->onIsEqual(other);
#ifdef SK_DEBUG
        if (result) {
            this->assertEquality(other);
        }
#endif
        return result;
    }

    /** Human-meaningful string to identify this effect; may be embedded
        in generated shader code. */
    const char* name() const;

    int numTransforms() const { return fCoordTransforms.count(); }

    /** Returns the coordinate transformation at index. index must be valid according to
        numTransforms(). */
    const GrCoordTransform& coordTransform(int index) const { return *fCoordTransforms[index]; }

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    /** Will this effect read the destination pixel value? */
    bool willReadDstColor() const { return fWillReadDstColor; }

    /** Will this effect read the fragment position? */
    bool willReadFragmentPosition() const { return fWillReadFragmentPosition; }

    /** Will this effect emit custom vertex shader code?
        (To set this value the effect must inherit from GrVertexEffect.) */
    bool hasVertexCode() const { return fHasVertexCode; }

    int numVertexAttribs() const {
        SkASSERT(0 == fVertexAttribTypes.count() || fHasVertexCode);
        return fVertexAttribTypes.count();
    }

    GrSLType vertexAttribType(int index) const { return fVertexAttribTypes[index]; }

    static const int kMaxVertexAttribs = 2;

    /** Useful for effects that want to insert a texture matrix that is implied by the texture
        dimensions */
    static inline SkMatrix MakeDivByTextureWHMatrix(const GrTexture* texture) {
        SkASSERT(NULL != texture);
        SkMatrix mat;
        mat.setIDiv(texture->width(), texture->height());
        return mat;
    }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

protected:
    /**
     * Subclasses call this from their constructor to register coordinate transformations. The
     * effect subclass manages the lifetime of the transformations (this function only stores a
     * pointer). The GrCoordTransform is typically a member field of the GrEffect subclass. When the
     * matrix has perspective, the transformed coordinates will have 3 components. Otherwise they'll
     * have 2. This must only be called from the constructor because GrEffects are immutable.
     */
    void addCoordTransform(const GrCoordTransform* coordTransform);

    /**
     * Subclasses call this from their constructor to register GrTextureAccesses. The effect
     * subclass manages the lifetime of the accesses (this function only stores a pointer). The
     * GrTextureAccess is typically a member field of the GrEffect subclass. This must only be
     * called from the constructor because GrEffects are immutable.
     */
    void addTextureAccess(const GrTextureAccess* textureAccess);

    GrEffect()
        : fWillReadDstColor(false)
        , fWillReadFragmentPosition(false)
        , fWillUseInputColor(true)
        , fHasVertexCode(false) {}

    /**
      * Helper for down-casting to a GrEffect subclass
      */
    template <typename T> static const T& CastEffect(const GrEffect& effect) {
        return *static_cast<const T*>(&effect);
    }

    /**
     * If the effect subclass will read the destination pixel value then it must call this function
     * from its constructor. Otherwise, when its generated backend-specific effect class attempts
     * to generate code that reads the destination pixel it will fail.
     */
    void setWillReadDstColor() { fWillReadDstColor = true; }

    /**
     * If the effect will generate a backend-specific effect that will read the fragment position
     * in the FS then it must call this method from its constructor. Otherwise, the request to
     * access the fragment position will be denied.
     */
    void setWillReadFragmentPosition() { fWillReadFragmentPosition = true; }

    /**
     * If the effect will generate a result that does not depend on the input color value then it must
     * call this function from its constructor. Otherwise, when its generated backend-specific code
     * might fail during variable binding due to unused variables.
     */
    void setWillNotUseInputColor() { fWillUseInputColor = false; }

private:
    SkDEBUGCODE(void assertEquality(const GrEffect& other) const;)

    /** Subclass implements this to support isEqual(). It will only be called if it is known that
        the two effects are of the same subclass (i.e. they return the same object from
        getFactory()).*/
    virtual bool onIsEqual(const GrEffect& other) const = 0;

    friend class GrVertexEffect; // to set fHasVertexCode and build fVertexAttribTypes.

    SkSTArray<4, const GrCoordTransform*, true>  fCoordTransforms;
    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    SkSTArray<kMaxVertexAttribs, GrSLType, true> fVertexAttribTypes;
    bool                                         fWillReadDstColor;
    bool                                         fWillReadFragmentPosition;
    bool                                         fWillUseInputColor;
    bool                                         fHasVertexCode;

    typedef SkRefCnt INHERITED;
};

/**
 * This creates an effect outside of the effect memory pool. The effect's destructor will be called
 * at global destruction time. NAME will be the name of the created GrEffect.
 */
#define GR_CREATE_STATIC_EFFECT(NAME, EFFECT_CLASS, ARGS)                                         \
static SkAlignedSStorage<sizeof(EFFECT_CLASS)> g_##NAME##_Storage;                                \
static GrEffect* NAME SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), EFFECT_CLASS, ARGS);         \
static SkAutoTDestroy<GrEffect> NAME##_ad(NAME);


#endif
