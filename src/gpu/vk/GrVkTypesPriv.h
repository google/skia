/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypesPriv_DEFINED
#define GrVkTypesPriv_DEFINED

#include "GrTypes.h"

enum class GrVkAttribBinding : int {
    kVertex = 0,
    kInstance = 1
};
static constexpr int kNumGrVkAttribBindings = 2;

#endif
