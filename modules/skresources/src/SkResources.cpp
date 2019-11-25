/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skresources/include/SkResources.h"

namespace skresources {

// TODO: Remove this once we've added actual symbols to skresources. For now, this avoids a linker
// warning about creating a library with no symbols.
void PlaceholderSymbolToAvoidAnEmptyLibrary() {}

} // namespace skresources
