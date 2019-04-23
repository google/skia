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
    const auto visibility = ctx->presentationContext().fInherited.fVisibility.get()->type();
    return visibility != SkSVGVisibility::Type::kHidden;
}

void SkSVGNode::setAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    this->onSetAttribute(attr, v);
}

void SkSVGNode::setClipPath(const SkSVGClip& clip) {
    fPresentationAttributes.fClipPath.set(clip);
}

void SkSVGNode::setClipRule(const SkSVGFillRule& clipRule) {
    fPresentationAttributes.fClipRule.set(clipRule);
}

void SkSVGNode::setFill(const SkSVGPaint& svgPaint) {
    fPresentationAttributes.fFill.set(svgPaint);
}

void SkSVGNode::setFillOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fFillOpacity.set(
        SkSVGNumberType(SkTPin<SkScalar>(opacity.value(), 0, 1)));
}

void SkSVGNode::setFillRule(const SkSVGFillRule& fillRule) {
    fPresentationAttributes.fFillRule.set(fillRule);
}

void SkSVGNode::setOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fOpacity.set(
        SkSVGNumberType(SkTPin<SkScalar>(opacity.value(), 0, 1)));
}

void SkSVGNode::setStroke(const SkSVGPaint& svgPaint) {
    fPresentationAttributes.fStroke.set(svgPaint);
}

void SkSVGNode::setStrokeDashArray(const SkSVGDashArray& dashArray) {
    fPresentationAttributes.fStrokeDashArray.set(dashArray);
}

void SkSVGNode::setStrokeDashOffset(const SkSVGLength& dashOffset) {
    fPresentationAttributes.fStrokeDashOffset.set(dashOffset);
}

void SkSVGNode::setStrokeOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fStrokeOpacity.set(
        SkSVGNumberType(SkTPin<SkScalar>(opacity.value(), 0, 1)));
}

void SkSVGNode::setStrokeWidth(const SkSVGLength& strokeWidth) {
    fPresentationAttributes.fStrokeWidth.set(strokeWidth);
}

void SkSVGNode::setVisibility(const SkSVGVisibility& visibility) {
    fPresentationAttributes.fVisibility.set(visibility);
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
            fPresentationAttributes.fStrokeLineCap.set(*lineCap);
        }
        break;
    case SkSVGAttribute::kStrokeLineJoin:
        if (const SkSVGLineJoinValue* lineJoin = v.as<SkSVGLineJoinValue>()) {
            fPresentationAttributes.fStrokeLineJoin.set(*lineJoin);
        }
        break;
    case SkSVGAttribute::kStrokeMiterLimit:
        if (const SkSVGNumberValue* miterLimit = v.as<SkSVGNumberValue>()) {
            fPresentationAttributes.fStrokeMiterLimit.set(*miterLimit);
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
