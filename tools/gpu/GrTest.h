/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTest_DEFINED
#define GrTest_DEFINED

#include "GrContext.h"

namespace GrTest {
    /**
     * Forces the GrContext to use a small atlas which only has room for one plot and will thus
     * constantly be evicting entries
     */
    void SetupAlwaysEvictAtlas(GrContext*);
};

#endif
