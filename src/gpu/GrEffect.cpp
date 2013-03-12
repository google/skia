/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEffect.h"
#include "GrBackendEffectFactory.h"
#include "GrContext.h"
#include "GrMemoryPool.h"
#include "SkTLS.h"

SK_DEFINE_INST_COUNT(GrEffect)

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
SkTArray<GrEffectTestFactory*, true>* GrEffectTestFactory::GetFactories() {
    static SkTArray<GrEffectTestFactory*, true> gFactories;
    return &gFactories;
}
#endif

namespace GrEffectUnitTest {
const SkMatrix& TestMatrix(SkMWCRandom* random) {
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
        gMatrices[4].set(SkMatrix::kMPersp0, SkFloatToScalar(0.00013f));
        gMatrices[4].set(SkMatrix::kMPersp1, SkFloatToScalar(-0.000039f));
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

SK_DEFINE_INST_COUNT(GrEffectRef)

GrEffectRef::~GrEffectRef() {
    GrAssert(1 == this->getRefCnt());
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
    GrAssert(NULL == fEffectRef);
}

const char* GrEffect::name() const {
    return this->getFactory().name();
}

void GrEffect::addTextureAccess(const GrTextureAccess* access) {
    fTextureAccesses.push_back(access);
}

void GrEffect::addVertexAttrib(GrSLType type) {
    GrAssert(fVertexAttribTypes.count() < kMaxVertexAttribs);
    fVertexAttribTypes.push_back(type);
}

void* GrEffect::operator new(size_t size) {
    return GrEffect_Globals::GetTLS()->allocate(size);
}

void GrEffect::operator delete(void* target) {
    GrEffect_Globals::GetTLS()->release(target);
}
