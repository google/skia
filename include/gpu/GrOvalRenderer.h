/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrContext.h"
#include "GrPaint.h"
#include "GrRefCnt.h"
#include "GrRect.h"

class GrContext;
class GrDrawTarget;
class GrPaint;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals (filled & stroked)
 */
class GrOvalRenderer : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrOvalRenderer)

    GrOvalRenderer() {}

    ~GrOvalRenderer() {}

    bool drawOval(GrDrawTarget* target, const GrContext* context, const GrPaint& paint,
                  const GrRect& oval, const SkStrokeRec& stroke);
private:
    void drawEllipse(GrDrawTarget* target, const GrPaint& paint,
                     const GrRect& ellipse,
                     const SkStrokeRec& stroke);
    void drawCircle(GrDrawTarget* target, const GrPaint& paint,
                    const GrRect& circle,
                    const SkStrokeRec& stroke);

    typedef GrRefCnt INHERITED;
};

#endif // GrOvalRenderer_DEFINED
