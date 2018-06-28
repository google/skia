/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathSink_DEFINED
#define SkPathSink_DEFINED

#include "SkPoint.h"

class SK_API SkPathSink {
public:
    virtual ~SkPathSink() {}

    SkPathSink& moveTo(SkPoint);
    SkPathSink& lineTo(SkPoint);
    SkPathSink& quadTo(SkPoint, SkPoint);
    SkPathSink& conicTo(SkPoint, SkPoint, SkScalar);
    SkPathSink& cubicTo(SkPoint, SkPoint, SkPoint);

protected:
    virtual void onMoveTo(SkPoint) = 0;
    virtual void onLineTo(SkPoint) = 0;
    virtual void onQuadTo(SkPoint, SkPoint) = 0;
    virtual void onConicTo(SkPoint, SkPoint, SkScalar) = 0;
    virtual void onCubicTo(SkPoint, SkPoint, SkPoint) = 0;
};

typedef void (*SkPathSourceProc)(void* ctx, SkPathSink*);

#endif
