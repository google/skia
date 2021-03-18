/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGPattern.h"

#include "include/core/SkPictureRecorder.h"
#include "include/core/SkShader.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGPattern::SkSVGPattern() : INHERITED(SkSVGTag::kPattern) {}

bool SkSVGPattern::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", name, value)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", name, value)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", name, value)) ||
           this->setPatternTransform(SkSVGAttributeParser::parse<SkSVGTransformType>(
                   "patternTransform", name, value)) ||
           this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", name, value));
}

const SkSVGPattern* SkSVGPattern::hrefTarget(const SkSVGRenderContext& ctx) const {
    if (fHref.iri().isEmpty()) {
        return nullptr;
    }

    const auto href = ctx.findNodeById(fHref);
    if (!href || href->tag() != SkSVGTag::kPattern) {
        return nullptr;
    }

    return static_cast<const SkSVGPattern*>(href.get());
}

template <typename T>
bool inherit_if_needed(const SkTLazy<T>& src, SkTLazy<T>& dst) {
    if (!dst.isValid()) {
        dst = src;
        return true;
    }

    return false;
}

/* https://www.w3.org/TR/SVG11/pservers.html#PatternElementHrefAttribute
 *
 * Any attributes which are defined on the referenced element which are not defined on this element
 * are inherited by this element. If this element has no children, and the referenced element does
 * (possibly due to its own ‘xlink:href’ attribute), then this element inherits the children from
 * the referenced element. Inheritance can be indirect to an arbitrary level; thus, if the
 * referenced element inherits attributes or children due to its own ‘xlink:href’ attribute, then
 * the current element can inherit those attributes or children.
 */
const SkSVGPattern* SkSVGPattern::resolveHref(const SkSVGRenderContext& ctx,
                                              PatternAttributes* attrs) const {
    const SkSVGPattern *currentNode = this,
                       *contentNode = this;
    do {
        // Bitwise OR to avoid short-circuiting.
        const bool didInherit =
            inherit_if_needed(currentNode->fX               , attrs->fX)      |
            inherit_if_needed(currentNode->fY               , attrs->fY)      |
            inherit_if_needed(currentNode->fWidth           , attrs->fWidth)  |
            inherit_if_needed(currentNode->fHeight          , attrs->fHeight) |
            inherit_if_needed(currentNode->fPatternTransform, attrs->fPatternTransform);

        if (!contentNode->hasChildren()) {
            contentNode = currentNode;
        }

        if (contentNode->hasChildren() && !didInherit) {
            // All attributes have been resolved, and a valid content node has been found.
            // We can terminate the href chain early.
            break;
        }

        // TODO: reference loop mitigation.
        currentNode = currentNode->hrefTarget(ctx);
    } while (currentNode);

    return contentNode;
}

bool SkSVGPattern::onAsPaint(const SkSVGRenderContext& ctx, SkPaint* paint) const {
    PatternAttributes attrs;
    const auto* contentNode = this->resolveHref(ctx, &attrs);

    const auto tile = ctx.lengthContext().resolveRect(
            attrs.fX.isValid()      ? *attrs.fX      : SkSVGLength(0),
            attrs.fY.isValid()      ? *attrs.fY      : SkSVGLength(0),
            attrs.fWidth.isValid()  ? *attrs.fWidth  : SkSVGLength(0),
            attrs.fHeight.isValid() ? *attrs.fHeight : SkSVGLength(0));

    if (tile.isEmpty()) {
        return false;
    }

    const SkMatrix* patternTransform = attrs.fPatternTransform.isValid()
            ? attrs.fPatternTransform.get()
            : nullptr;

    SkPictureRecorder recorder;
    SkSVGRenderContext recordingContext(ctx, recorder.beginRecording(tile));

    // Cannot call into INHERITED:: because SkSVGHiddenContainer skips rendering.
    contentNode->SkSVGContainer::onRender(recordingContext);

    paint->setShader(recorder.finishRecordingAsPicture()->makeShader(
                                                 SkTileMode::kRepeat,
                                                 SkTileMode::kRepeat,
                                                 SkFilterMode::kLinear,
                                                 patternTransform,
                                                 &tile));
    return true;
}
