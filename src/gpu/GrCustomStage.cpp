/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCustomStage.h"

int32_t GrProgramStageFactory::fCurrStageClassID =
                                    GrProgramStageFactory::kIllegalStageClassID;

GrCustomStage::GrCustomStage() {
}

GrCustomStage::~GrCustomStage() {

}

bool GrCustomStage::isOpaque(bool inputTextureIsOpaque) const {
    return false;
}

