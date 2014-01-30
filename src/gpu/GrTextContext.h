/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrPoint.h"
#include "GrGlyph.h"
#include "GrPaint.h"
#include "SkDeviceProperties.h"

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
    virtual void drawText(const char text[], size_t byteLength, SkScalar x, SkScalar y) = 0;
    virtual void drawPosText(const char text[], size_t byteLength,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPosition) = 0;
    
protected:
    GrTextContext(GrContext*, const GrPaint&, const SkPaint&, const SkDeviceProperties&);

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    static void MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                            const char text[], size_t byteLength, SkVector* stopVector);

    GrContext*         fContext;
    GrPaint            fPaint;
    SkPaint            fSkPaint;
    SkDeviceProperties fDeviceProperties;
    GrDrawTarget*      fDrawTarget;

    SkIRect            fClipRect;
};

/*
 * These classes wrap the creation of a single text context for a given GPU device. The
 * assumption is that we'll only be using one text context at a time for that device.
 */
class GrTextContextManager {
public:
    virtual ~GrTextContextManager() {}
    virtual GrTextContext* create(GrContext* grContext, const GrPaint& grPaint,
                                  const SkPaint& skPaint, const SkDeviceProperties& props) = 0;
    virtual bool canDraw(const SkPaint& paint, const SkMatrix& ctm) = 0;
};

template <class TextContextClass>
class GrTTextContextManager : public GrTextContextManager {
private:
    class ManagedTextContext : public TextContextClass {
    public:
        virtual ~ManagedTextContext() {}

        ManagedTextContext(GrContext* grContext,
                           const GrPaint& grPaint,
                           const SkPaint& skPaint,
                           const SkDeviceProperties& properties,
                           GrTTextContextManager<TextContextClass>* manager) :
                          TextContextClass(grContext, grPaint, skPaint, properties) {
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

    virtual ~GrTTextContextManager() {
        SkASSERT(!fUsed);
        sk_free(fAllocation);
    }

    virtual GrTextContext* create(GrContext* grContext, const GrPaint& grPaint,
                                  const SkPaint& skPaint, const SkDeviceProperties& properties)
                                 SK_OVERRIDE {
        // add check for usePath here?
        SkASSERT(!fUsed);
        ManagedTextContext* obj = SkNEW_PLACEMENT_ARGS(fAllocation, ManagedTextContext,
                                                       (grContext, grPaint, skPaint, properties,
                                                        this));
        fUsed = true;
        return obj;
    }

    virtual bool canDraw(const SkPaint& paint, const SkMatrix& ctm) SK_OVERRIDE {
        return TextContextClass::CanDraw(paint, ctm);
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
