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
 * Implements GrBackendEffectFactory for a GrEffect subclass as a singleton. This can be used by
 * most GrEffect subclasses to implement the GrEffect::getFactory() method:
 *
 * const GrBackendEffectFactory& MyEffect::getFactory() const {
 *     return GrTBackendEffectFactory<MyEffect>::getInstance();
 * }
 *
 * Using this class requires that the GrEffect subclass always produces the same GrGLEffect
 * subclass. Additionally, It adds the following requirements to the GrEffect and GrGLEffect
 * subclasses:
 *
 * 1. The GrGLEffect used by GrEffect subclass MyEffect must be named or typedef'ed to
 *    MyEffect::GLEffect.
 * 2. MyEffect::GLEffect must have a static function:
 *      EffectKey GenKey(const GrDrawEffect, const GrGLCaps&)
 *    which generates a key that maps 1 to 1 with code variations emitted by
 *    MyEffect::GLEffect::emitCode().
 * 3. MyEffect must have a static function:
 *      const char* Name()
 *    which returns a human-readable name for the effect.
 */
template <typename EffectClass>
class GrTBackendEffectFactory : public GrBackendEffectFactory {

public:
    typedef typename EffectClass::GLEffect GLEffect;

    /** Returns a human-readable name for the effect. Implemented using GLEffect::Name as described
     *  in this class's comment. */
    virtual const char* name() const SK_OVERRIDE { return EffectClass::Name(); }


    /** Implemented using GLEffect::GenKey as described in this class's comment. */
    virtual void getGLEffectKey(const GrDrawEffect& drawEffect,
                                const GrGLCaps& caps,
                                GrEffectKeyBuilder* b) const SK_OVERRIDE {
        GLEffect::GenKey(drawEffect, caps, b);
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrEffect; caller is responsible for deleting
        the object. */
    virtual GrGLEffect* createGLInstance(const GrDrawEffect& drawEffect) const SK_OVERRIDE {
        return SkNEW_ARGS(GLEffect, (*this, drawEffect));
    }

    /** This class is a singleton. This function returns the single instance. */
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
    GrTBackendEffectFactory() {}
};

#endif
