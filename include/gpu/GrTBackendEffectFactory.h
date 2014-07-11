/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTBackendEffectFactory_DEFINED
#define GrTBackendEffectFactory_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrDrawEffect.h"
#include "gl/GrGLProgramEffects.h"

/**
 * Implements GrBackendEffectFactory for a GrEffect subclass as a singleton.
 */
template <typename EffectClass>
class GrTBackendEffectFactory : public GrBackendEffectFactory {

public:
    typedef typename EffectClass::GLEffect GLEffect;

    /** Returns a human-readable name that is accessible via GrEffect or
        GrGLEffect and is consistent between the two of them.
     */
    virtual const char* name() const SK_OVERRIDE { return EffectClass::Name(); }

    /** Generates an effect's key. This enables caching of generated shaders. Part of the
        id identifies the GrEffect subclass. The remainder is based on the aspects of the
        GrEffect object's configuration that affect GLSL code generation. If this fails
        then program generation should be aborted. Failure occurs if the effect uses more
        transforms, attributes, or textures than the key has space for. */
    virtual bool getGLEffectKey(const GrDrawEffect& drawEffect,
                                const GrGLCaps& caps,
                                GrEffectKeyBuilder* b) const SK_OVERRIDE {
        SkASSERT(kIllegalEffectClassID != fEffectClassID);
        EffectKey effectKey = GLEffect::GenKey(drawEffect, caps);
        EffectKey textureKey = GrGLProgramEffects::GenTextureKey(drawEffect, caps);
        EffectKey transformKey = GrGLProgramEffects::GenTransformKey(drawEffect);
        EffectKey attribKey = GrGLProgramEffects::GenAttribKey(drawEffect);
        static const uint32_t kMetaKeyInvalidMask = ~((uint32_t) SK_MaxU16);
        if ((textureKey | transformKey | attribKey | fEffectClassID) & kMetaKeyInvalidMask) {
            return false;
        }

        // effectKey must be first because it is what will be returned by
        // GrGLProgramDesc::EffectKeyProvider and passed to the GrGLEffect as its key. 
        b->add32(effectKey);
        b->add32(textureKey << 16 | transformKey);
        b->add32(fEffectClassID << 16 | attribKey);
        return true;
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrEffect; caller is responsible for deleting
        the object. */
    virtual GrGLEffect* createGLInstance(const GrDrawEffect& drawEffect) const SK_OVERRIDE {
        return SkNEW_ARGS(GLEffect, (*this, drawEffect));
    }

    /** This class is a singleton. This function returns the single instance.
     */
    static const GrBackendEffectFactory& getInstance() {
        static SkAlignedSTStorage<1, GrTBackendEffectFactory> gInstanceMem;
        static const GrTBackendEffectFactory* gInstance;
        if (!gInstance) {
            gInstance = SkNEW_PLACEMENT(gInstanceMem.get(),
                                        GrTBackendEffectFactory);
        }
        return *gInstance;
    }

protected:
    GrTBackendEffectFactory() {
        fEffectClassID = GenID();
    }
};

#endif
