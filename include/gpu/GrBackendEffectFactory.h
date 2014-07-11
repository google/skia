/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendEffectFactory_DEFINED
#define GrBackendEffectFactory_DEFINED

#include "GrTypes.h"
#include "SkTemplates.h"
#include "SkThread.h"
#include "SkTypes.h"
#include "SkTArray.h"

/** Given a GrEffect of a particular type, creates the corresponding graphics-backend-specific
    effect object. Also tracks equivalence of shaders generated via a key. Each factory instance
    is assigned a generation ID at construction. The ID of the return of GrEffect::getFactory()
    is used as a type identifier. Thus a GrEffect subclass must return a singleton from
    getFactory(). GrEffect subclasses should use the derived class GrTBackendEffectFactory that is
    templated on the GrEffect subclass as their factory object. It requires that the GrEffect
    subclass has a nested class (or typedef) GLEffect which is its GL implementation and a subclass
    of GrGLEffect.
 */

class GrGLEffect;
class GrGLCaps;
class GrDrawEffect;

/**
 * Used by effects to build their keys. It incorpates each per-effect key into a larger shader key.
 */
class GrEffectKeyBuilder {
public:
    GrEffectKeyBuilder(SkTArray<unsigned char, true>* data) : fData(data), fCount(0) {
        SkASSERT(0 == fData->count() % sizeof(uint32_t));
    }

    void add32(uint32_t v) {
        ++fCount;
        fData->push_back_n(4, reinterpret_cast<uint8_t*>(&v));
    }

    size_t size() const { return sizeof(uint32_t) * fCount; }

private:
    SkTArray<uint8_t, true>* fData; // unowned ptr to the larger key.
    int fCount;                     // number of uint32_ts added to fData by the effect.
};

class GrBackendEffectFactory : SkNoncopyable {
public:
    typedef uint32_t EffectKey;

    virtual bool getGLEffectKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*) const = 0;
    virtual GrGLEffect* createGLInstance(const GrDrawEffect&) const = 0;

    bool operator ==(const GrBackendEffectFactory& b) const {
        return fEffectClassID == b.fEffectClassID;
    }
    bool operator !=(const GrBackendEffectFactory& b) const {
        return !(*this == b);
    }

    virtual const char* name() const = 0;

protected:
    enum {
        kIllegalEffectClassID = 0,
    };

    GrBackendEffectFactory() {
        fEffectClassID = kIllegalEffectClassID;
    }
    virtual ~GrBackendEffectFactory() {}

    static int32_t GenID() {
        // fCurrEffectClassID has been initialized to kIllegalEffectClassID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        int32_t id = sk_atomic_inc(&fCurrEffectClassID) + 1;
        return id;
    }

    int32_t fEffectClassID;

private:
    static int32_t fCurrEffectClassID;
};

#endif
