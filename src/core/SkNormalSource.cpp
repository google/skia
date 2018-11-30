/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNormalFlatSource.h"
#include "SkNormalMapSource.h"
#include "SkNormalSource.h"

// Generating vtable
SkNormalSource::~SkNormalSource() {}

void SkNormalSource::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkNormalMapSourceImpl);
    SK_REGISTER_FLATTENABLE(SkNormalFlatSourceImpl);
}

