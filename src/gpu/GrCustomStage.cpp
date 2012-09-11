/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCustomStage.h"
#include "GrMemoryPool.h"
#include "SkTLS.h"

SK_DEFINE_INST_COUNT(GrCustomStage)

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
SkTArray<GrCustomStageTestFactory*, true>* GrCustomStageTestFactory::GetFactories() {
    static SkTArray<GrCustomStageTestFactory*, true> gFactories;
    return &gFactories;
}
#endif

class GrCustomStage_Globals {
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

int32_t GrProgramStageFactory::fCurrStageClassID =
                                    GrProgramStageFactory::kIllegalStageClassID;

GrCustomStage::GrCustomStage() {

}

GrCustomStage::~GrCustomStage() {

}

bool GrCustomStage::isOpaque(bool inputTextureIsOpaque) const {
    return false;
}

bool GrCustomStage::isEqual(const GrCustomStage& s) const {
    if (this->numTextures() != s.numTextures()) {
        return false;
    }
    for (int i = 0; i < this->numTextures(); ++i) {
        if (this->texture(i) != s.texture(i)) {
            return false;
        }
    }
    return true;
}

int GrCustomStage::numTextures() const {
    return 0;
}

const GrTextureAccess& GrCustomStage::textureAccess(int index) const {
    GrCrash("We shouldn't be calling this function on the base class.");
    static GrTextureAccess kDummy;
    return kDummy;
}

void * GrCustomStage::operator new(size_t size) {
    return GrCustomStage_Globals::GetTLS()->allocate(size);
}

void GrCustomStage::operator delete(void* target) {
    GrCustomStage_Globals::GetTLS()->release(target);
}
