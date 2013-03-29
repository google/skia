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
#include "GrNoncopyable.h"
#include "GrRefCnt.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"
#include "GrTypesPriv.h"

class GrBackendEffectFactory;
class GrContext;
class GrEffect;
class SkString;

/**
 * A Wrapper class for GrEffect. Its ref-count will track owners that may use effects to enqueue
 * new draw operations separately from ownership within a deferred drawing queue. When the
 * GrEffectRef ref count reaches zero the scratch GrResources owned by the effect can be recycled
 * in service of later draws. However, the deferred draw queue may still own direct references to
 * the underlying GrEffect.
 */
class GrEffectRef : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrEffectRef);

    GrEffect* get() { return fEffect; }
    const GrEffect* get() const { return fEffect; }

    const GrEffect* operator-> () { return fEffect; }
    const GrEffect* operator-> () const { return fEffect; }

    void* operator new(size_t size);
    void operator delete(void* target);

private:
    friend class GrEffect; // to construct these

    explicit GrEffectRef(GrEffect* effect);

    virtual ~GrEffectRef();

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

    Because almost no code should ever handle a GrEffect outside of a GrEffectRef, we privately
    inherit from GrRefCnt to help prevent accidental direct ref'ing/unref'ing of effects.
  */
class GrEffect : private GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrEffect)

    /**
     * The types of vertex coordinates available to an effect in the vertex shader. Effects can
     * require their own vertex attribute but these coordinates are made available by the framework
     * in all programs. kCustom_CoordsType is provided to signify that an alternative set of coords
     * is used (usually an explicit vertex attribute) but its meaning is determined by the effect
     * subclass.
     */
    enum CoordsType {
        kLocal_CoordsType,
        kPosition_CoordsType,

        kCustom_CoordsType,
    };

    virtual ~GrEffect();

    /**
     * This function is used to perform optimizations. When called the color and validFlags params
     * indicate whether the input components to this effect in the FS will have known values.
     * validFlags is a bitfield of GrColorComponentFlags. The function updates both params to
     * indicate known values of its output. A component of the color param only has meaning if the
     * corresponding bit in validFlags is set.
     */
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const = 0;

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

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    /** Will this effect read the destination pixel value? */
    bool willReadDst() const { return fWillReadDst; }

    int numVertexAttribs() const { return fVertexAttribTypes.count(); }

    GrSLType vertexAttribType(int index) const { return fVertexAttribTypes[index]; }

    static const int kMaxVertexAttribs = 2;

    /** Useful for effects that want to insert a texture matrix that is implied by the texture
        dimensions */
    static inline SkMatrix MakeDivByTextureWHMatrix(const GrTexture* texture) {
        GrAssert(NULL != texture);
        SkMatrix mat;
        mat.setIDiv(texture->width(), texture->height());
        return mat;
    }

    void* operator new(size_t size);
    void operator delete(void* target);

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
     * Subclasses call this from their constructor to register GrTextureAccesses. The effect
     * subclass manages the lifetime of the accesses (this function only stores a pointer). This
     * must only be called from the constructor because GrEffects are immutable.
     */
    void addTextureAccess(const GrTextureAccess* textureAccess);

    /**
     * Subclasses call this from their constructor to register vertex attributes (at most
     * kMaxVertexAttribs). This must only be called from the constructor because GrEffects are
     * immutable.
     */
    void addVertexAttrib(GrSLType type);

    GrEffect() : fWillReadDst(false), fEffectRef(NULL) {}

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
    void setWillReadDst() { fWillReadDst = true; }

private:
    bool isEqual(const GrEffect& other) const {
        if (&this->getFactory() != &other.getFactory()) {
            return false;
        }
        bool result = this->onIsEqual(other);
#if GR_DEBUG
        if (result) {
            GrAssert(this->numTextures() == other.numTextures());
            for (int i = 0; i < this->numTextures(); ++i) {
                GrAssert(*fTextureAccesses[i] == *other.fTextureAccesses[i]);
            }
        }
#endif
        return result;
    }

    /** Subclass implements this to support isEqual(). It will only be called if it is known that
        the two effects are of the same subclass (i.e. they return the same object from
        getFactory()).*/
    virtual bool onIsEqual(const GrEffect& other) const = 0;

    void EffectRefDestroyed() { fEffectRef = NULL; }

    friend class GrEffectRef;   // to call EffectRefDestroyed()
    friend class GrEffectStage; // to rewrap GrEffect in GrEffectRef when restoring an effect-stage
                                // from deferred state, to call isEqual on naked GrEffects, and
                                // to inc/dec deferred ref counts.

    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    SkSTArray<kMaxVertexAttribs, GrSLType, true> fVertexAttribTypes;
    bool                                         fWillReadDst;
    GrEffectRef*                                 fEffectRef;

    typedef GrRefCnt INHERITED;
};

inline GrEffectRef::GrEffectRef(GrEffect* effect) {
    GrAssert(NULL != effect);
    effect->ref();
    fEffect = effect;
}

#endif
