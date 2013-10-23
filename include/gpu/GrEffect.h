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

/**
 * A Wrapper class for GrEffect. Its ref-count will track owners that may use effects to enqueue
 * new draw operations separately from ownership within a deferred drawing queue. When the
 * GrEffectRef ref count reaches zero the scratch GrResources owned by the effect can be recycled
 * in service of later draws. However, the deferred draw queue may still own direct references to
 * the underlying GrEffect.
 *
 * GrEffectRefs created by new are placed in a per-thread managed pool. The pool is destroyed when
 * the thread ends. Therefore, all dynamically allocated GrEffectRefs must be unreffed before thread
 * termination.
 */
class GrEffectRef : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrEffectRef);
    virtual ~GrEffectRef();

    GrEffect* get() { return fEffect; }
    const GrEffect* get() const { return fEffect; }

    const GrEffect* operator-> () { return fEffect; }
    const GrEffect* operator-> () const { return fEffect; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

private:
    friend class GrEffect; // to construct these

    explicit GrEffectRef(GrEffect* effect);

    GrEffect* fEffect;

    typedef SkRefCnt INHERITED;
};

/** Provides custom vertex shader, fragment shader, uniform data for a particular stage of the
    Ganesh shading pipeline.
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
    GrEffect objects *must* be immutable: after being constructed, their fields may not change.

    GrEffect subclass objects should be created by factory functions that return GrEffectRef.
    There is no public way to wrap a GrEffect in a GrEffectRef. Thus, a factory should be a static
    member function of a GrEffect subclass.

    Because almost no code should ever handle a GrEffect directly outside of a GrEffectRef, we
    privately inherit from SkRefCnt to help prevent accidental direct ref'ing/unref'ing of effects.

    Dynamically allocated GrEffects and their corresponding GrEffectRefs are managed by a per-thread
    memory pool. The ref count of an effect must reach 0 before the thread terminates and the pool
    is destroyed. To create a static effect use the macro GR_CREATE_STATIC_EFFECT declared below.
  */
class GrEffect : private SkRefCnt {
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
        generate the same shader code. To test for identical code generation use the EffectKey
        computed by the GrBackendEffectFactory:
            effectA.getFactory().glEffectKey(effectA) == effectB.getFactory().glEffectKey(effectB).
     */
    bool isEqual(const GrEffectRef& other) const {
        return this->isEqual(*other.get());
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

    /** These functions are used when recording effects into a deferred drawing queue. The inc call
        keeps the effect alive outside of GrEffectRef while allowing any resources owned by the
        effect to be returned to the cache for reuse. The dec call must balance the inc call. */
    void incDeferredRefCounts() const {
        this->ref();
        int count = fTextureAccesses.count();
        for (int t = 0; t < count; ++t) {
            fTextureAccesses[t]->getTexture()->incDeferredRefCount();
        }
    }
    void decDeferredRefCounts() const {
        int count = fTextureAccesses.count();
        for (int t = 0; t < count; ++t) {
            fTextureAccesses[t]->getTexture()->decDeferredRefCount();
        }
        this->unref();
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
        , fHasVertexCode(false)
        , fEffectRef(NULL) {}

    /** This should be called by GrEffect subclass factories. See the comment on AutoEffectUnref for
        an example factory function. */
    static GrEffectRef* CreateEffectRef(GrEffect* effect) {
        if (NULL == effect->fEffectRef) {
            effect->fEffectRef = SkNEW_ARGS(GrEffectRef, (effect));
        } else {
            effect->fEffectRef->ref();
        }
        return effect->fEffectRef;
    }

    static const GrEffectRef* CreateEffectRef(const GrEffect* effect) {
        return CreateEffectRef(const_cast<GrEffect*>(effect));
    }

    /** Used by GR_CREATE_STATIC_EFFECT below */
    static GrEffectRef* CreateStaticEffectRef(void* refStorage, GrEffect* effect) {
        SkASSERT(NULL == effect->fEffectRef);
        effect->fEffectRef = SkNEW_PLACEMENT_ARGS(refStorage, GrEffectRef, (effect));
        return effect->fEffectRef;
    }


    /** Helper used in subclass factory functions to unref the effect after it has been wrapped in a
        GrEffectRef. E.g.:

        class EffectSubclass : public GrEffect {
        public:
            GrEffectRef* Create(ParamType1 param1, ParamType2 param2, ...) {
                AutoEffectUnref effect(SkNEW_ARGS(EffectSubclass, (param1, param2, ...)));
                return CreateEffectRef(effect);
            }
     */
    class AutoEffectUnref {
    public:
        AutoEffectUnref(GrEffect* effect) : fEffect(effect) { }
        ~AutoEffectUnref() { fEffect->unref(); }
        operator GrEffect*() { return fEffect; }
    private:
        GrEffect* fEffect;
    };

    /** Helper for getting the GrEffect out of a GrEffectRef and down-casting to a GrEffect subclass
      */
    template <typename T>
    static const T& CastEffect(const GrEffect& effectRef) {
        return *static_cast<const T*>(&effectRef);
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

    SkDEBUGCODE(void assertEquality(const GrEffect& other) const;)

    /** Subclass implements this to support isEqual(). It will only be called if it is known that
        the two effects are of the same subclass (i.e. they return the same object from
        getFactory()).*/
    virtual bool onIsEqual(const GrEffect& other) const = 0;

    void EffectRefDestroyed() { fEffectRef = NULL; }

    friend class GrEffectRef;    // to call EffectRefDestroyed()
    friend class GrEffectStage;  // to rewrap GrEffect in GrEffectRef when restoring an effect-stage
                                 // from deferred state, to call isEqual on naked GrEffects, and
                                 // to inc/dec deferred ref counts.
    friend class GrVertexEffect; // to set fHasVertexCode and build fVertexAttribTypes.

    SkSTArray<4, const GrCoordTransform*, true>  fCoordTransforms;
    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    SkSTArray<kMaxVertexAttribs, GrSLType, true> fVertexAttribTypes;
    bool                                         fWillReadDstColor;
    bool                                         fWillReadFragmentPosition;
    bool                                         fWillUseInputColor;
    bool                                         fHasVertexCode;
    GrEffectRef*                                 fEffectRef;

    typedef SkRefCnt INHERITED;
};

inline GrEffectRef::GrEffectRef(GrEffect* effect) {
    SkASSERT(NULL != effect);
    effect->ref();
    fEffect = effect;
}

/**
 * This creates an effect outside of the effect memory pool. The effect's destructor will be called
 * at global destruction time. NAME will be the name of the created GrEffectRef.
 */
#define GR_CREATE_STATIC_EFFECT(NAME, EFFECT_CLASS, ARGS)                                         \
enum {                                                                                            \
    k_##NAME##_EffectRefOffset = GR_CT_ALIGN_UP(sizeof(EFFECT_CLASS), 8),                         \
    k_##NAME##_StorageSize = k_##NAME##_EffectRefOffset + sizeof(GrEffectRef)                     \
};                                                                                                \
static SkAlignedSStorage<k_##NAME##_StorageSize> g_##NAME##_Storage;                              \
static void* NAME##_RefLocation = (char*)g_##NAME##_Storage.get() + k_##NAME##_EffectRefOffset;   \
static GrEffect* NAME##_Effect SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), EFFECT_CLASS, ARGS);\
static SkAutoTDestroy<GrEffect> NAME##_ad(NAME##_Effect);                                         \
static GrEffectRef* NAME(GrEffect::CreateStaticEffectRef(NAME##_RefLocation, NAME##_Effect));     \
static SkAutoTDestroy<GrEffectRef> NAME##_Ref_ad(NAME)


#endif
