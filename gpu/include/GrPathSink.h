
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrPathSink_DEFINED
#define GrPathSink_DEFINED

#include "GrScalar.h"

class GrPathSink {
public:
    virtual ~GrPathSink() {}

    virtual void moveTo(GrScalar x, GrScalar y) = 0;
    virtual void lineTo(GrScalar x, GrScalar y) = 0;
    virtual void quadTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1) = 0;
    virtual void cubicTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1,
                         GrScalar x2, GrScalar y2) = 0;
    virtual void close() = 0;
};

#endif

