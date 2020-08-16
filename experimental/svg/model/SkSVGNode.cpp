/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGNode.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkTLazy.h"

SkSVGNode::SkSVGNode(SkSVGTag t) : fTag(t) { }

SkSVGNode::~SkSVGNode() { }

void SkSVGNode::render(const SkSVGRenderContext& ctx) const {
    SkSVGRenderContext localContext(ctx);

    if (this->onPrepareToRender(&localContext)) {
        this->onRender(localContext);
    }
}

bool SkSVGNode::asPaint(const SkSVGRenderContext& ctx, SkPaint* paint) const {
    SkSVGRenderContext localContext(ctx);

    return this->onPrepareToRender(&localContext) && this->onAsPaint(localContext, paint);
}

SkPath SkSVGNode::asPath(const SkSVGRenderContext& ctx) const {
    SkSVGRenderContext localContext(ctx);
    if (!this->onPrepareToRender(&localContext)) {
        return SkPath();
    }

    SkPath path = this->onAsPath(localContext);

    if (const auto* clipPath = localContext.clipPath()) {
        // There is a clip-path present on the current node.
        Op(path, *clipPath, kIntersect_SkPathOp, &path);
    }

    return path;
}

bool SkSVGNode::onPrepareToRender(SkSVGRenderContext* ctx) const {
    ctx->applyPresentationAttributes(fPresentationAttributes,
                                     this->hasChildren() ? 0 : SkSVGRenderContext::kLeaf);

    // visibility:hidden disables rendering
    const auto visibility = ctx->presentationContext().fInherited.fVisibility->type();
    return visibility != SkSVGVisibility::Type::kHidden;
}

void SkSVGNode::setAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    this->onSetAttribute(attr, v);
}

void SkSVGNode::setClipPath(const SkSVGClip& clip) {
    fPresentationAttributes.fClipPath.set(clip);
}

template <typename T>
void SetInheritedByDefault(SkTLazy<T>& presentation_attribute, const T& value) {
    if (value.type() != T::Type::kInherit) {
        presentation_attribute.set(value);
    } else {
        // kInherited values are semantically equivalent to
        // the absence of a local presentation attribute.
        presentation_attribute.reset();
    }
}

void SkSVGNode::setClipRule(const SkSVGFillRule& clipRule) {
    SetInheritedByDefault(fPresentationAttributes.fClipRule, clipRule);
}

void SkSVGNode::setColor(const SkSVGColorType& color) {
    // TODO: Color should be inherited by default
    fPresentationAttributes.fColor.set(color);
}

void SkSVGNode::setFill(const SkSVGPaint& svgPaint) {
    SetInheritedByDefault(fPresentationAttributes.fFill, svgPaint);
}

void SkSVGNode::setFillOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fFillOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setFillRule(const SkSVGFillRule& fillRule) {
    SetInheritedByDefault(fPresentationAttributes.fFillRule, fillRule);
}

void SkSVGNode::setOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setStroke(const SkSVGPaint& svgPaint) {
    SetInheritedByDefault(fPresentationAttributes.fStroke, svgPaint);
}

void SkSVGNode::setStrokeDashArray(const SkSVGDashArray& dashArray) {
    SetInheritedByDefault(fPresentationAttributes.fStrokeDashArray, dashArray);
}

void SkSVGNode::setStrokeDashOffset(const SkSVGLength& dashOffset) {
    fPresentationAttributes.fStrokeDashOffset.set(dashOffset);
}

void SkSVGNode::setStrokeOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fStrokeOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setStrokeLineCap(const SkSVGLineCap& lc) {
    SetInheritedByDefault(fPresentationAttributes.fStrokeLineCap, lc);
}

void SkSVGNode::setStrokeLineJoin(const SkSVGLineJoin& lj) {
    SetInheritedByDefault(fPresentationAttributes.fStrokeLineJoin, lj);
}

void SkSVGNode::setStrokeMiterLimit(const SkSVGNumberType& ml) {
    fPresentationAttributes.fStrokeMiterLimit.set(ml);
}

void SkSVGNode::setStrokeWidth(const SkSVGLength& strokeWidth) {
    fPresentationAttributes.fStrokeWidth.set(strokeWidth);
}

void SkSVGNode::setVisibility(const SkSVGVisibility& visibility) {
    SetInheritedByDefault(fPresentationAttributes.fVisibility, visibility);
}

void SkSVGNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kClipPath:
        if (const SkSVGClipValue* clip = v.as<SkSVGClipValue>()) {
            this->setClipPath(*clip);
        }
        break;
    case SkSVGAttribute::kClipRule:
        if (const SkSVGFillRuleValue* clipRule = v.as<SkSVGFillRuleValue>()) {
            this->setClipRule(*clipRule);
        }
        break;
    case SkSVGAttribute::kColor:
        if (const SkSVGColorValue* color = v.as<SkSVGColorValue>()) {
            this->setColor(*color);
        }
        break;
    case SkSVGAttribute::kFill:
        if (const SkSVGPaintValue* paint = v.as<SkSVGPaintValue>()) {
            this->setFill(*paint);
        }
        break;
    case SkSVGAttribute::kFillOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            this->setFillOpacity(*opacity);
        }
        break;
    case SkSVGAttribute::kFillRule:
        if (const SkSVGFillRuleValue* fillRule = v.as<SkSVGFillRuleValue>()) {
            this->setFillRule(*fillRule);
        }
        break;
    case SkSVGAttribute::kOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            this->setOpacity(*opacity);
        }
        break;
    case SkSVGAttribute::kStroke:
        if (const SkSVGPaintValue* paint = v.as<SkSVGPaintValue>()) {
            this->setStroke(*paint);
        }
        break;
    case SkSVGAttribute::kStrokeDashArray:
        if (const SkSVGDashArrayValue* dashArray = v.as<SkSVGDashArrayValue>()) {
            this->setStrokeDashArray(*dashArray);
        }
        break;
    case SkSVGAttribute::kStrokeDashOffset:
        if (const SkSVGLengthValue* dashOffset= v.as<SkSVGLengthValue>()) {
            this->setStrokeDashOffset(*dashOffset);
        }
        break;
    case SkSVGAttribute::kStrokeOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            this->setStrokeOpacity(*opacity);
        }
        break;
    case SkSVGAttribute::kStrokeLineCap:
        if (const SkSVGLineCapValue* lineCap = v.as<SkSVGLineCapValue>()) {
            this->setStrokeLineCap(*lineCap);
        }
        break;
    case SkSVGAttribute::kStrokeLineJoin:
        if (const SkSVGLineJoinValue* lineJoin = v.as<SkSVGLineJoinValue>()) {
            this->setStrokeLineJoin(*lineJoin);
        }
        break;
    case SkSVGAttribute::kStrokeMiterLimit:
        if (const SkSVGNumberValue* miterLimit = v.as<SkSVGNumberValue>()) {
            this->setStrokeMiterLimit(*miterLimit);
        }
        break;
    case SkSVGAttribute::kStrokeWidth:
        if (const SkSVGLengthValue* strokeWidth = v.as<SkSVGLengthValue>()) {
            this->setStrokeWidth(*strokeWidth);
        }
        break;
    case SkSVGAttribute::kVisibility:
        if (const SkSVGVisibilityValue* visibility = v.as<SkSVGVisibilityValue>()) {
            this->setVisibility(*visibility);
        }
        break;
    default:
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("attribute ID <%d> ignored for node <%d>\n", attr, fTag);
#endif
        break;
    }
}
