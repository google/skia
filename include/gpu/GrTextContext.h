/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrContext.h"
#include "GrGlyph.h"
#include "GrPaint.h"

#include "SkPostConfig.h"

class GrContext;
class GrDrawTarget;
class GrFontScaler;

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual ~GrTextContext() {}
    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                                 GrFontScaler*) = 0;

protected:
    GrTextContext(GrContext*, const GrPaint&, const SkPaint&);

    GrPaint                fPaint;
    SkPaint                fSkPaint;
    GrContext*             fContext;
    GrDrawTarget*          fDrawTarget;

    SkIRect                fClipRect;
};

/*
 * These classes wrap the creation of a single text context for a given GPU device. The
 * assumption is that we'll only be using one text context at a time for that device.
 */
class GrTextContextManager {
public:
    virtual ~GrTextContextManager() {}
    virtual GrTextContext* create(GrContext* context, const GrPaint& grPaint,
                                  const SkPaint& skPaint) = 0;
};

template <class TextContextClass>
class GrTTextContextManager : public GrTextContextManager {
private:
    class ManagedTextContext : public TextContextClass {
    public:
        ~ManagedTextContext() {}
        
        ManagedTextContext(GrContext* context,
                           const GrPaint& grPaint,
                           const SkPaint& skPaint,
                           GrTTextContextManager<TextContextClass>* manager) :
        TextContextClass(context, grPaint, skPaint) {
            fManager = manager;
        }

        static void operator delete(void* ptr) {
            if (ptr == NULL) {
                return;
            }
            ManagedTextContext* context = reinterpret_cast<ManagedTextContext*>(ptr);
            context->fManager->recycle(context);
        }

        static void operator delete(void*, void*) {
        }

        GrTTextContextManager<TextContextClass>* fManager;
    };
    
public:
    GrTTextContextManager() {
        fAllocation = sk_malloc_throw(sizeof(ManagedTextContext));
        fUsed = false;
    }

    ~GrTTextContextManager() {
        SkASSERT(!fUsed);
        sk_free(fAllocation);
    }

    GrTextContext* create(GrContext* context, const GrPaint& grPaint,
                          const SkPaint& skPaint) {
        // add check for usePath here?
        SkASSERT(!fUsed);
        ManagedTextContext* obj = SkNEW_PLACEMENT_ARGS(fAllocation, ManagedTextContext,
                                                       (context, grPaint, skPaint, this));
        fUsed = true;
        return obj;
    }

private:
    void recycle(GrTextContext* textContext) {
        SkASSERT((void*)textContext == fAllocation);
        SkASSERT(fUsed);
        fUsed = false;
    }

    void* fAllocation;
    bool  fUsed;
};

#endif
