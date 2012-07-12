/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCustomStage.h"

SK_DEFINE_INST_COUNT(GrCustomStage)

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
    for (unsigned int i = 0; i < this->numTextures(); ++i) {
        if (this->texture(i) != s.texture(i)) {
            return false;
        }
    }
    return true;
}

unsigned int GrCustomStage::numTextures() const {
    return 0;
}

GrTexture* GrCustomStage::texture(unsigned int index) const {
    return NULL;
}

