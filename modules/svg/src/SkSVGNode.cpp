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

SkSVGNode::SkSVGNode(SkSVGTag t) : fTag(t) {
    // Uninherited presentation attributes need a non-null default value.
    fPresentationAttributes.fStopColor.set(SkSVGColor(SK_ColorBLACK));
    fPresentationAttributes.fStopOpacity.set(SkSVGNumberType(1.0f));
    fPresentationAttributes.fFloodColor.set(SkSVGColor(SK_ColorBLACK));
    fPresentationAttributes.fFloodOpacity.set(SkSVGNumberType(1.0f));
}

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

bool SkSVGNode::parseAndSetAttribute(const char* n, const char* v) {
#define PARSE_AND_SET(svgName, attrName)                                                        \
    this->set##attrName(                                                                        \
            SkSVGAttributeParser::parseProperty<decltype(fPresentationAttributes.f##attrName)>( \
                    svgName, n, v))

    return PARSE_AND_SET(   "clip-path"        , ClipPath)
           || PARSE_AND_SET("clip-rule"        , ClipRule)
           || PARSE_AND_SET("color"            , Color)
           || PARSE_AND_SET("fill"             , Fill)
           || PARSE_AND_SET("fill-opacity"     , FillOpacity)
           || PARSE_AND_SET("fill-rule"        , FillRule)
           || PARSE_AND_SET("filter"           , Filter)
           || PARSE_AND_SET("flood-color"      , FloodColor)
           || PARSE_AND_SET("flood-opacity"    , FloodOpacity)
           || PARSE_AND_SET("font-family"      , FontFamily)
           || PARSE_AND_SET("font-size"        , FontSize)
           || PARSE_AND_SET("font-style"       , FontStyle)
           || PARSE_AND_SET("font-weight"      , FontWeight)
           || PARSE_AND_SET("opacity"          , Opacity)
           || PARSE_AND_SET("stop-color"       , StopColor)
           || PARSE_AND_SET("stop-opacity"     , StopOpacity)
           || PARSE_AND_SET("stroke"           , Stroke)
           || PARSE_AND_SET("stroke-dasharray" , StrokeDashArray)
           || PARSE_AND_SET("stroke-dashoffset", StrokeDashOffset)
           || PARSE_AND_SET("stroke-linecap"   , StrokeLineCap)
           || PARSE_AND_SET("stroke-linejoin"  , StrokeLineJoin)
           || PARSE_AND_SET("stroke-miterlimit", StrokeMiterLimit)
           || PARSE_AND_SET("stroke-opacity"   , StrokeOpacity)
           || PARSE_AND_SET("stroke-width"     , StrokeWidth)
           || PARSE_AND_SET("text-anchor"      , TextAnchor)
           || PARSE_AND_SET("visibility"       , Visibility);

#undef PARSE_AND_SET
}
