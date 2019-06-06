/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TESTBED_SKIA_API_IDL_H
#define TESTBED_SKIA_API_IDL_H

#include "generator.h"
#include "include/c/sk_path.h"
#include "include/c/sk_shader.h"

namespace generator {

extern API skiaAPI;

extern NameConverter skiaCNames;

extern NameConverter skiaJavaNames;

extern JavaConverter skiaJavaConv;

}  // namespace generator

#endif  // TESTBED_SKIA_API_IDL_H
