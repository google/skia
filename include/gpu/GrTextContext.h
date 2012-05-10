
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrGlyph.h"
#include "GrMatrix.h"
#include "GrRefCnt.h"

class GrContext;
class GrFontScaler;
class GrPaint;

class SkGpuDevice;
class SkPaint;

/**
 * Derived classes can use stages GrPaint::kTotalStages through 
 * GrDrawState::kNumStages-1. The stages before GrPaint::kTotalStages
 * are reserved for setting up the draw (i.e., textures and filter masks).
 */
class GrTextContext: public GrRefCnt {
protected:
    GrContext*      fContext;

public:
    /**
     * To use a text context it must be wrapped in an AutoFinish. AutoFinish's
     * destructor ensures all drawing is flushed to the GrContext.
     */
    class AutoFinish {
    public:
        AutoFinish(GrTextContext* textContext, GrContext* context,
                   const GrPaint&, const GrMatrix* extMatrix);
        ~AutoFinish();
        GrTextContext* getTextContext() const;

    private:
        GrTextContext* fTextContext;
    };

    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                                 GrFontScaler*) = 0;

    virtual ~GrTextContext() {}

protected:
    GrTextContext() {
        fContext = NULL;
    }

    bool isValid() const {
        return (NULL != fContext);
    }

    /**
     * Initialize the object.
     *
     * Before call to this method, the instance is considered to be in
     * invalid state. I.e. call to any method other than isValid will result in
     * undefined behaviour.
     *
     * @see finish
     */
    virtual void init(GrContext* context, const GrPaint&,
                      const GrMatrix* extMatrix) {
        fContext = context;
    }

    /**
     * Reset the object to invalid state.
     *
     * After call to this method, the instance is considered to be in
     * invalid state.
     *
     * It might be brought back to a valid state by calling init.
     *
     * @see init
     */
    virtual void finish() {
        fContext = NULL;
    }

private:
    typedef GrRefCnt INHERITED;
};

inline GrTextContext::AutoFinish::AutoFinish(GrTextContext* textContext,
                                             GrContext* context,
                                             const GrPaint& grPaint,
                                             const GrMatrix* extMatrix) {
    GrAssert(NULL != textContext);
    fTextContext = textContext;
    fTextContext->ref();
    fTextContext->init(context, grPaint, extMatrix);
}

inline GrTextContext::AutoFinish::~AutoFinish() {
    fTextContext->finish();
    fTextContext->unref();
}

inline GrTextContext* GrTextContext::AutoFinish::getTextContext() const {
    return fTextContext;
}

#endif
