/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTPin.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"
#include "src/core/SkTLazy.h"

SkSVGNode::SkSVGNode(SkSVGTag t) : fTag(t) { }

SkSVGNode::~SkSVGNode() { }

void SkSVGNode::render(const SkSVGRenderContext& ctx) const {
    SkSVGRenderContext localContext(ctx, this);

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

SkRect SkSVGNode::objectBoundingBox(const SkSVGRenderContext& ctx) const {
    return this->onObjectBoundingBox(ctx);
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

void SkSVGNode::setColor(const SkSVGColorType& color) {
    // TODO: Color should be inherited by default
    fPresentationAttributes.fColor.set(color);
}

void SkSVGNode::setFillOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fFillOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setStrokeDashOffset(const SkSVGLength& dashOffset) {
    fPresentationAttributes.fStrokeDashOffset.set(dashOffset);
}

void SkSVGNode::setStrokeOpacity(const SkSVGNumberType& opacity) {
    fPresentationAttributes.fStrokeOpacity.set(SkSVGNumberType(SkTPin<SkScalar>(opacity, 0, 1)));
}

void SkSVGNode::setStrokeMiterLimit(const SkSVGNumberType& ml) {
    fPresentationAttributes.fStrokeMiterLimit.set(ml);
}

void SkSVGNode::setStrokeWidth(const SkSVGLength& strokeWidth) {
    fPresentationAttributes.fStrokeWidth.set(strokeWidth);
}

void SkSVGNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kColor:
        if (const SkSVGColorValue* color = v.as<SkSVGColorValue>()) {
            this->setColor(*color);
        }
        break;
    case SkSVGAttribute::kFillOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            this->setFillOpacity(*opacity);
        }
        break;
    case SkSVGAttribute::kFilter:
        if (const SkSVGFilterValue* filter = v.as<SkSVGFilterValue>()) {
            this->setFilter(*filter);
        }
        break;
    case SkSVGAttribute::kOpacity:
        if (const SkSVGNumberValue* opacity = v.as<SkSVGNumberValue>()) {
            this->setOpacity(*opacity);
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
    default:
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("attribute ID <%d> ignored for node <%d>\n", attr, fTag);
#endif
        break;
    }
}

bool SkSVGNode::parseAndSetAttribute(const char* n, const char* v) {
    return this->setClipPath(SkSVGAttributeParser::parse<SkSVGClip>     ("clip-path"       , n, v))
        || this->setClipRule(SkSVGAttributeParser::parse<SkSVGFillRule> ("clip-rule"       , n, v))
        || this->setFill    (SkSVGAttributeParser::parse<SkSVGPaint>    ("fill"            , n, v))
        || this->setFillRule(SkSVGAttributeParser::parse<SkSVGFillRule> ("fill-rule"       , n, v))
        || this->setFontFamily
                          (SkSVGAttributeParser::parse<SkSVGFontFamily> ("font-family"     , n, v))
        || this->setFontSize(SkSVGAttributeParser::parse<SkSVGFontSize> ("font-size"       , n, v))
        || this->setFontStyle
                            (SkSVGAttributeParser::parse<SkSVGFontStyle>("font-style"      , n, v))
        || this->setFontWeight
                           (SkSVGAttributeParser::parse<SkSVGFontWeight>("font-weight"     , n, v))
        || this->setStroke  (SkSVGAttributeParser::parse<SkSVGPaint>    ("stroke"          , n, v))
        || this->setStrokeDashArray
                            (SkSVGAttributeParser::parse<SkSVGDashArray>("stroke-dasharray", n, v))
        || this->setStrokeLineCap
                            (SkSVGAttributeParser::parse<SkSVGLineCap>  ("stroke-linecap"  , n ,v))
        || this->setStrokeLineJoin
                            (SkSVGAttributeParser::parse<SkSVGLineJoin> ("stroke-linejoin" , n ,v))
        || this->setTextAnchor
                           (SkSVGAttributeParser::parse<SkSVGTextAnchor>("text-anchor"     , n, v))
        || this->setVisibility
                           (SkSVGAttributeParser::parse<SkSVGVisibility>("visibility"      , n, v));
}
