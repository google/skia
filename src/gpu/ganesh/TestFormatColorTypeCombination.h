/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

namespace GrTest {
    struct TestFormatColorTypeCombination {
        GrColorType fColorType;
        GrBackendFormat fFormat;
    };
}
