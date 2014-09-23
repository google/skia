/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessor.h"
#include "GrBackendProcessorFactory.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrMemoryPool.h"
#include "SkTLS.h"

namespace GrProcessorUnitTest {
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

class GrProcessor_Globals {
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

int32_t GrBackendProcessorFactory::fCurrEffectClassID =
        GrBackendProcessorFactory::kIllegalEffectClassID;

///////////////////////////////////////////////////////////////////////////////

GrProcessor::~GrProcessor() {}

const char* GrProcessor::name() const {
    return this->getFactory().name();
}

void GrProcessor::addCoordTransform(const GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    SkDEBUGCODE(transform->setInEffect();)
}

void GrProcessor::addTextureAccess(const GrTextureAccess* access) {
    fTextureAccesses.push_back(access);
    this->addGpuResource(access->getProgramTexture());
}

void* GrProcessor::operator new(size_t size) {
    return GrProcessor_Globals::GetTLS()->allocate(size);
}

void GrProcessor::operator delete(void* target) {
    GrProcessor_Globals::GetTLS()->release(target);
}

#ifdef SK_DEBUG
void GrProcessor::assertEquality(const GrProcessor& other) const {
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
