/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderNode_DEFINED
#define SkSGRenderNode_DEFINED

#include "SkSGNode.h"

#include "SkColorFilter.h"

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Base class for nodes which can render to a canvas.
 */
class RenderNode : public Node {
protected:
    struct RenderContext;

public:
    // Render the node and its descendants to the canvas.
    void render(SkCanvas*, const RenderContext* = nullptr) const;

protected:
    RenderNode();

    virtual void onRender(SkCanvas*, const RenderContext*) const = 0;

    // Paint property overrides.
    // These are deferred until we can determine whether they can be applied to the individual
    // draw paints, or whether they require content isolation (applied to a layer).
    struct RenderContext {
        sk_sp<SkColorFilter> fColorFilter;
        float                fOpacity = 1;

        // Returns true if the paint was modified.
        bool modulatePaint(SkPaint*) const;
    };

    class ScopedRenderContext final {
    public:
        ScopedRenderContext(SkCanvas*, const RenderContext*);
        ~ScopedRenderContext();

        ScopedRenderContext(ScopedRenderContext&& that) { *this = std::move(that); }

        ScopedRenderContext& operator=(ScopedRenderContext&& that) {
            fCanvas       = that.fCanvas;
            fCtx          = std::move(that.fCtx);
            fRestoreCount = that.fRestoreCount;

            // scope ownership is being transferred
            that.fRestoreCount = -1;

            return *this;
        }

        operator const RenderContext* () const { return &fCtx; }

        // Add (cumulative) paint overrides to a render node sub-DAG.
        ScopedRenderContext&& modulateOpacity(float opacity);
        ScopedRenderContext&& modulateColorFilter(sk_sp<SkColorFilter>);

        // Force content isolation for a node sub-DAG by applying the RenderContext
        // overrides via a layer.
        ScopedRenderContext&& setIsolation(const SkRect& bounds, bool do_isolate);

    private:
        // stack-only
        void* operator new(size_t)        = delete;
        void* operator new(size_t, void*) = delete;

        // Scopes cannot be copied.
        ScopedRenderContext(const ScopedRenderContext&)            = delete;
        ScopedRenderContext& operator=(const ScopedRenderContext&) = delete;

        SkCanvas*     fCanvas;
        RenderContext fCtx;
        int           fRestoreCount;
    };

private:
    typedef Node INHERITED;
};

} // namespace sksg

#endif // SkSGRenderNode_DEFINED
