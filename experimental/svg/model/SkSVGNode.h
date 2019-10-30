/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGNode_DEFINED
#define SkSVGNode_DEFINED

#include "experimental/svg/model/SkSVGAttribute.h"
#include "include/core/SkRefCnt.h"

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkPath;
class SkSVGRenderContext;
class SkSVGValue;

enum class SkSVGTag {
    kCircle,
    kClipPath,
    kDefs,
    kEllipse,
    kG,
    kLine,
    kLinearGradient,
    kPath,
    kPattern,
    kPolygon,
    kPolyline,
    kRadialGradient,
    kRect,
    kStop,
    kSvg,
    kUse
};

class SkSVGNode : public SkRefCnt {
public:
    virtual ~SkSVGNode();

    SkSVGTag tag() const { return fTag; }

    virtual void appendChild(sk_sp<SkSVGNode>) = 0;

    void render(const SkSVGRenderContext&) const;
    bool asPaint(const SkSVGRenderContext&, SkPaint*) const;
    SkPath asPath(const SkSVGRenderContext&) const;

    void setAttribute(SkSVGAttribute, const SkSVGValue&);

    void setClipPath(const SkSVGClip&);
    void setClipRule(const SkSVGFillRule&);
    void setFill(const SkSVGPaint&);
    void setFillOpacity(const SkSVGNumberType&);
    void setFillRule(const SkSVGFillRule&);
    void setOpacity(const SkSVGNumberType&);
    void setStroke(const SkSVGPaint&);
    void setStrokeDashArray(const SkSVGDashArray&);
    void setStrokeDashOffset(const SkSVGLength&);
    void setStrokeOpacity(const SkSVGNumberType&);
    void setStrokeWidth(const SkSVGLength&);
    void setVisibility(const SkSVGVisibility&);

protected:
    SkSVGNode(SkSVGTag);

    // Called before onRender(), to apply local attributes to the context.  Unlike onRender(),
    // onPrepareToRender() bubbles up the inheritance chain: overriders should always call
    // INHERITED::onPrepareToRender(), unless they intend to short-circuit rendering
    // (return false).
    // Implementations are expected to return true if rendering is to continue, or false if
    // the node/subtree rendering is disabled.
    virtual bool onPrepareToRender(SkSVGRenderContext*) const;

    virtual void onRender(const SkSVGRenderContext&) const = 0;

    virtual bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const { return false; }

    virtual SkPath onAsPath(const SkSVGRenderContext&) const = 0;

    virtual void onSetAttribute(SkSVGAttribute, const SkSVGValue&);

    virtual bool hasChildren() const { return false; }

private:
    SkSVGTag                    fTag;

    // FIXME: this should be sparse
    SkSVGPresentationAttributes fPresentationAttributes;

    typedef SkRefCnt INHERITED;
};

#endif // SkSVGNode_DEFINED
