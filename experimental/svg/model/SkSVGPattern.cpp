/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGPattern.h"

#include "SkPictureRecorder.h"
#include "SkShader.h"
#include "SkSVGRenderContext.h"
#include "SkSVGValue.h"

SkSVGPattern::SkSVGPattern() : INHERITED(SkSVGTag::kPattern) {}

void SkSVGPattern::setX(const SkSVGLength& x) {
    fAttributes.fX.set(x);
}

void SkSVGPattern::setY(const SkSVGLength& y) {
    fAttributes.fY.set(y);
}

void SkSVGPattern::setWidth(const SkSVGLength& w) {
    fAttributes.fWidth.set(w);
}

void SkSVGPattern::setHeight(const SkSVGLength& h) {
    fAttributes.fHeight.set(h);
}

void SkSVGPattern::setHref(const SkSVGStringType& href) {
    fHref = std::move(href);
}

void SkSVGPattern::setPatternTransform(const SkSVGTransformType& patternTransform) {
    fAttributes.fPatternTransform.set(patternTransform);
}

void SkSVGPattern::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kX:
        if (const auto* x = v.as<SkSVGLengthValue>()) {
            this->setX(*x);
        }
        break;
    case SkSVGAttribute::kY:
        if (const auto* y = v.as<SkSVGLengthValue>()) {
            this->setY(*y);
        }
        break;
    case SkSVGAttribute::kWidth:
        if (const auto* w = v.as<SkSVGLengthValue>()) {
            this->setWidth(*w);
        }
        break;
    case SkSVGAttribute::kHeight:
        if (const auto* h = v.as<SkSVGLengthValue>()) {
            this->setHeight(*h);
        }
        break;
    case SkSVGAttribute::kHref:
        if (const auto* href = v.as<SkSVGStringValue>()) {
            this->setHref(*href);
        }
        break;
    case SkSVGAttribute::kPatternTransform:
        if (const auto* t = v.as<SkSVGTransformValue>()) {
            this->setPatternTransform(*t);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

const SkSVGPattern* SkSVGPattern::hrefTarget(const SkSVGRenderContext& ctx) const {
    if (fHref.value().isEmpty()) {
        return nullptr;
    }

    const auto* href = ctx.findNodeById(fHref);
    if (!href || href->tag() != SkSVGTag::kPattern) {
        return nullptr;
    }

    return static_cast<const SkSVGPattern*>(href);
}

template <typename T>
bool inherit_if_needed(const SkTLazy<T>& src, SkTLazy<T>& dst) {
    if (!dst.isValid()) {
        dst = src;
        return true;
    }

    return false;
}

/* https://www.w3.org/TR/SVG/pservers.html#PatternElementHrefAttribute
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
            inherit_if_needed(currentNode->fAttributes.fX               , attrs->fX)      |
            inherit_if_needed(currentNode->fAttributes.fY               , attrs->fY)      |
            inherit_if_needed(currentNode->fAttributes.fWidth           , attrs->fWidth)  |
            inherit_if_needed(currentNode->fAttributes.fHeight          , attrs->fHeight) |
            inherit_if_needed(currentNode->fAttributes.fPatternTransform, attrs->fPatternTransform);

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
            attrs.fX.isValid()      ? *attrs.fX.get()      : SkSVGLength(0),
            attrs.fY.isValid()      ? *attrs.fY.get()      : SkSVGLength(0),
            attrs.fWidth.isValid()  ? *attrs.fWidth.get()  : SkSVGLength(0),
            attrs.fHeight.isValid() ? *attrs.fHeight.get() : SkSVGLength(0));

    if (tile.isEmpty()) {
        return false;
    }

    const SkMatrix* patternTransform = attrs.fPatternTransform.isValid()
            ? &attrs.fPatternTransform.get()->value()
            : nullptr;

    SkPictureRecorder recorder;
    SkSVGRenderContext recordingContext(ctx, recorder.beginRecording(tile));

    // Cannot call into INHERITED:: because SkSVGHiddenContainer skips rendering.
    contentNode->SkSVGContainer::onRender(recordingContext);

    paint->setShader(recorder.finishRecordingAsPicture()->makeShader(
                                                 SkTileMode::kRepeat,
                                                 SkTileMode::kRepeat,
                                                 patternTransform,
                                                 &tile));
    return true;
}
