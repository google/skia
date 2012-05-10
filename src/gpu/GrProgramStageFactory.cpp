/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProgramStageFactory.h"

GrProgramStageFactory::~GrProgramStageFactory(void) {

}

uint16_t GrProgramStageFactory::stageKey(const GrCustomStage*) {
    return 0;
}

