/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextValue_DEFINED
#define SkottieTextValue_DEFINED

#include "modules/skottie/include/SkottieProperty.h"

namespace skjson { class Value; }

namespace skottie {
typedef TextPropertyValue TextValue;

namespace internal {

// Unlike other types, TextValue parsing requires access to an AnimationBuilder.
bool Parse(const skjson::Value&, const AnimationBuilder&, TextValue*);

} // namespace internal
} // namespace skottie

#endif // SkottieTextValue_DEFINED
