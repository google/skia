/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRenderContext_DEFINED
#define SkSVGRenderContext_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGIDMapper.h"
#include "src/core/SkTLazy.h"

class SkCanvas;
class SkSVGLength;

class SkSVGLengthContext {
public:
    SkSVGLengthContext(const SkSize& viewport, SkScalar dpi = 90)
        : fViewport(viewport), fDPI(dpi) {}

    enum class LengthType {
        kHorizontal,
        kVertical,
        kOther,
    };

    const SkSize& viewPort() const { return fViewport; }
    void setViewPort(const SkSize& viewport) { fViewport = viewport; }

    SkScalar resolve(const SkSVGLength&, LengthType) const;
    SkRect   resolveRect(const SkSVGLength& x, const SkSVGLength& y,
                         const SkSVGLength& w, const SkSVGLength& h) const;

private:
    SkSize   fViewport;
    SkScalar fDPI;
};

struct SkSVGPresentationContext {
    SkSVGPresentationContext();
    SkSVGPresentationContext(const SkSVGPresentationContext&)            = default;
    SkSVGPresentationContext& operator=(const SkSVGPresentationContext&) = default;

    // Inherited presentation attributes, computed for the current node.
    SkSVGPresentationAttributes fInherited;

    // Cached paints, reflecting the current presentation attributes.
    SkPaint fFillPaint;
    SkPaint fStrokePaint;
};

class SkSVGRenderContext {
public:
    SkSVGRenderContext(SkCanvas*, const sk_sp<SkFontMgr>&, const SkSVGIDMapper&,
                       const SkSVGLengthContext&, const SkSVGPresentationContext&,
                       const SkSVGNode*);
    SkSVGRenderContext(const SkSVGRenderContext&);
    SkSVGRenderContext(const SkSVGRenderContext&, SkCanvas*);
    SkSVGRenderContext(const SkSVGRenderContext&, const SkSVGNode*);
    ~SkSVGRenderContext();

    const SkSVGLengthContext& lengthContext() const { return *fLengthContext; }
    SkSVGLengthContext* writableLengthContext() { return fLengthContext.writable(); }

    const SkSVGPresentationContext& presentationContext() const { return *fPresentationContext; }

    SkCanvas* canvas() const { return fCanvas; }
    void saveOnce();

    enum ApplyFlags {
        kLeaf = 1 << 0, // the target node doesn't have descendants
    };
    void applyPresentationAttributes(const SkSVGPresentationAttributes&, uint32_t flags);

    // Scoped wrapper that temporarily clears the original node reference.
    class BorrowedNode {
    public:
        explicit BorrowedNode(sk_sp<SkSVGNode>* node)
            : fOwner(node) {
            if (fOwner) {
                fBorrowed = std::move(*fOwner);
                *fOwner = nullptr;
            }
        }

        ~BorrowedNode() {
            if (fOwner) {
                *fOwner = std::move(fBorrowed);
            }
        }

        const SkSVGNode* get() const { return fBorrowed.get(); }
        const SkSVGNode* operator->() const { return fBorrowed.get(); }
        const SkSVGNode& operator*() const { return *fBorrowed; }

        operator bool() const { return !!fBorrowed; }

    private:
        // noncopyable
        BorrowedNode(const BorrowedNode&)      = delete;
        BorrowedNode& operator=(BorrowedNode&) = delete;

        sk_sp<SkSVGNode>* fOwner;
        sk_sp<SkSVGNode>  fBorrowed;
    };

    // Note: the id->node association is cleared for the lifetime of the returned value
    // (effectively breaks reference cycles, assuming appropriate return value scoping).
    BorrowedNode findNodeById(const SkString&) const;

    const SkPaint* fillPaint() const;
    const SkPaint* strokePaint() const;

    SkSVGColorType resolveSvgColor(const SkSVGColor&) const;

    // The local computed clip path (not inherited).
    const SkPath* clipPath() const { return fClipPath.getMaybeNull(); }

    // The node being rendered (may be null).
    const SkSVGNode* node() const { return fNode; }

    sk_sp<SkFontMgr> fontMgr() const {
        return fFontMgr ? fFontMgr : SkFontMgr::RefDefault();
    }

    SkRect resolveOBBRect(const SkSVGLength& x, const SkSVGLength& y,
                          const SkSVGLength& w, const SkSVGLength& h,
                          SkSVGObjectBoundingBoxUnits) const;

    const SkSVGIDMapper& idMapper() const { return fIDMapper; }

private:
    // Stack-only
    void* operator new(size_t)                               = delete;
    void* operator new(size_t, void*)                        = delete;
    SkSVGRenderContext& operator=(const SkSVGRenderContext&) = delete;

    void applyOpacity(SkScalar opacity, uint32_t flags, bool hasFilter);
    void applyFilter(const SkSVGFuncIRI&);
    void applyClip(const SkSVGFuncIRI&);
    void applyMask(const SkSVGFuncIRI&);
    void updatePaintsWithCurrentColor(const SkSVGPresentationAttributes&);

    const sk_sp<SkFontMgr>&                       fFontMgr;
    const SkSVGIDMapper&                          fIDMapper;
    SkTCopyOnFirstWrite<SkSVGLengthContext>       fLengthContext;
    SkTCopyOnFirstWrite<SkSVGPresentationContext> fPresentationContext;
    SkCanvas*                                     fCanvas;
    // The save count on 'fCanvas' at construction time.
    // A restoreToCount() will be issued on destruction.
    int                                           fCanvasSaveCount;

    // clipPath, if present for the current context (not inherited).
    SkTLazy<SkPath>                               fClipPath;

    const SkSVGNode* fNode;
};

#endif // SkSVGRenderContext_DEFINED
