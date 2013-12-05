/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEffect.h"
#include "GrBackendEffectFactory.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrMemoryPool.h"
#include "SkTLS.h"

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
SkTArray<GrEffectTestFactory*, true>* GrEffectTestFactory::GetFactories() {
    static SkTArray<GrEffectTestFactory*, true> gFactories;
    return &gFactories;
}
#endif

namespace GrEffectUnitTest {
const SkMatrix& TestMatrix(SkRandom* random) {
    static SkMatrix gMatrices[5];
    static bool gOnce;
    if (!gOnce) {
        gMatrices[0].reset();
        gMatrices[1].setTranslate(SkIntToScalar(-100), SkIntToScalar(100));
        gMatrices[2].setRotate(SkIntToScalar(17));
        gMatrices[3].setRotate(SkIntToScalar(185));
        gMatrices[3].postTranslate(SkIntToScalar(66), SkIntToScalar(-33));
        gMatrices[3].postScale(SkIntToScalar(2), SK_ScalarHalf);
        gMatrices[4].setRotate(SkIntToScalar(215));
        gMatrices[4].set(SkMatrix::kMPersp0, 0.00013f);
        gMatrices[4].set(SkMatrix::kMPersp1, -0.000039f);
        gOnce = true;
    }
    return gMatrices[random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gMatrices)))];
}
}

class GrEffect_Globals {
public:
    static GrMemoryPool* GetTLS() {
        return (GrMemoryPool*)SkTLS::Get(CreateTLS, DeleteTLS);
    }

private:
    static void* CreateTLS() {
        return SkNEW_ARGS(GrMemoryPool, (4096, 4096));
    }

    static void DeleteTLS(void* pool) {
        SkDELETE(reinterpret_cast<GrMemoryPool*>(pool));
    }
};

int32_t GrBackendEffectFactory::fCurrEffectClassID = GrBackendEffectFactory::kIllegalEffectClassID;

///////////////////////////////////////////////////////////////////////////////

GrEffectRef::~GrEffectRef() {
    SkASSERT(this->unique());
    fEffect->EffectRefDestroyed();
    fEffect->unref();
}

void* GrEffectRef::operator new(size_t size) {
    return GrEffect_Globals::GetTLS()->allocate(size);
}

void GrEffectRef::operator delete(void* target) {
    GrEffect_Globals::GetTLS()->release(target);
}

///////////////////////////////////////////////////////////////////////////////

GrEffect::~GrEffect() {
    SkASSERT(NULL == fEffectRef);
}

const char* GrEffect::name() const {
    return this->getFactory().name();
}

void GrEffect::addCoordTransform(const GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    SkDEBUGCODE(transform->setInEffect();)
}

void GrEffect::addTextureAccess(const GrTextureAccess* access) {
    fTextureAccesses.push_back(access);
}

void* GrEffect::operator new(size_t size) {
    return GrEffect_Globals::GetTLS()->allocate(size);
}

void GrEffect::operator delete(void* target) {
    GrEffect_Globals::GetTLS()->release(target);
}

#ifdef SK_DEBUG
void GrEffect::assertEquality(const GrEffect& other) const {
    SkASSERT(this->numTransforms() == other.numTransforms());
    for (int i = 0; i < this->numTransforms(); ++i) {
        SkASSERT(this->coordTransform(i) == other.coordTransform(i));
    }
    SkASSERT(this->numTextures() == other.numTextures());
    for (int i = 0; i < this->numTextures(); ++i) {
        SkASSERT(this->textureAccess(i) == other.textureAccess(i));
    }
}
#endif
