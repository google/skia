/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexArrayObj_DEFINED
#define GrVertexArrayObj_DEFINED

#include "GrFakeRefObj.h"

class GrVertexArrayObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrVertexArrayObj);

public:
    GrVertexArrayObj() : GrFakeRefObj() {}

    typedef GrFakeRefObj INHERITED;
};
#endif
