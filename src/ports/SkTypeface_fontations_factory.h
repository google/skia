/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTypeface_Fontations_factory_DEFINED
#define SkTypeface_Fontations_factory_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkTypeface.h"

namespace SkTypefaces::Fontations {
static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('f', 'n', 't', 'a');
}

#endif
