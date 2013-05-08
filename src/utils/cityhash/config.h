/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Converts from Skia build flags to the macro definitions cityhash normally
 * gets from autoconf.
 */

#include "SkTypes.h"

#ifdef SK_CPU_BENDIAN
  #define WORDS_BIGENDIAN 1
#endif
