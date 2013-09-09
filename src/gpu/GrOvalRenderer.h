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

class GrContext;
class GrDrawTarget;
class GrPaint;
struct SkRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrOvalRenderer)

    GrOvalRenderer() : fRRectIndexBuffer(NULL) {}
    ~GrOvalRenderer() {
        this->reset();
    }

    void reset();

    bool drawOval(GrDrawTarget* target, const GrContext* context, bool useAA,
                  const SkRect& oval, const SkStrokeRec& stroke);
    bool drawSimpleRRect(GrDrawTarget* target, GrContext* context, bool useAA,
                         const SkRRect& rrect, const SkStrokeRec& stroke);

private:
    bool drawEllipse(GrDrawTarget* target, bool useAA,
                     const SkRect& ellipse,
                     const SkStrokeRec& stroke);
    bool drawDIEllipse(GrDrawTarget* target, bool useAA,
                       const SkRect& ellipse,
                       const SkStrokeRec& stroke);
    void drawCircle(GrDrawTarget* target, bool useAA,
                    const SkRect& circle,
                    const SkStrokeRec& stroke);

    GrIndexBuffer* rRectIndexBuffer(GrGpu* gpu);

    GrIndexBuffer* fRRectIndexBuffer;

    typedef SkRefCnt INHERITED;
};

#endif // GrOvalRenderer_DEFINED
