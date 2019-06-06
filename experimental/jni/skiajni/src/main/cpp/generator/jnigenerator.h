/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TESTBED_JNIGENERATOR_H
#define TESTBED_JNIGENERATOR_H

#include "generator.h"

namespace generator {

void genJavaAPI(JavaConverter& conv, const char* path);

}

#endif  // TESTBED_JNIGENERATOR_H
