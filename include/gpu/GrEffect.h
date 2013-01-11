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

class GrBackendEffectFactory;
class GrContext;
class SkString;

/** Provides custom vertex shader, fragment shader, uniform data for a particular stage of the
    Ganesh shading pipeline.
    Subclasses must have a function that produces a human-readable name:
        static const char* Name();
    GrEffect objects *must* be immutable: after being constructed, their fields may not change.
  */
class GrEffect : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrEffect)

    GrEffect() {};
    virtual ~GrEffect();

    /**
     * Flags for getConstantColorComponents. They are defined so that the bit order reflects the
     * GrColor shift order.
     */
    enum ValidComponentFlags {
        kR_ValidComponentFlag = 1 << (GrColor_SHIFT_R / 8),
        kG_ValidComponentFlag = 1 << (GrColor_SHIFT_G / 8),
        kB_ValidComponentFlag = 1 << (GrColor_SHIFT_B / 8),
        kA_ValidComponentFlag = 1 << (GrColor_SHIFT_A / 8),

        kAll_ValidComponentFlags = (kR_ValidComponentFlag | kG_ValidComponentFlag |
                                    kB_ValidComponentFlag | kA_ValidComponentFlag)
    };

    /**
     * This function is used to perform optimizations. When called the color and validFlags params
     * indicate whether the input components to this effect in the FS will have known values. The
     * function updates both params to indicate known values of its output. A component of the color
     * param only has meaning if the corresponding bit in validFlags is set.
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

    /** Returns true if the other effect will generate identical output.
        Must only be called if the two are already known to be of the
        same type (i.e.  they return the same value from getFactory()).

        Equality is not the same thing as equivalence.
        To test for equivalence (that they will generate the same
        shader code, but may have different uniforms), check equality
        of the EffectKey produced by the GrBackendEffectFactory:
        a.getFactory().glEffectKey(a) == b.getFactory().glEffectKey(b).

        The default implementation of this function returns true iff
        the two stages have the same return value for numTextures() and
        for texture() over all valid indices.
     */
    virtual bool isEqual(const GrEffect&) const;

    /** Human-meaningful string to identify this effect; may be embedded
        in generated shader code. */
    const char* name() const;

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

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

protected:
    /**
     * Subclasses call this from their constructor to register GrTextureAcceses. The effect subclass
     * manages the lifetime of the accesses (this function only stores a pointer). This must only be
     * called from the constructor because GrEffects are supposed to be immutable.
     */
    void addTextureAccess(const GrTextureAccess* textureAccess);

private:
    SkSTArray<4, const GrTextureAccess*, true> fTextureAccesses;
    typedef GrRefCnt INHERITED;
};

#endif
