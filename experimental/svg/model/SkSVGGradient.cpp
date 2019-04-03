/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGGradient.h"
#include "SkSVGRenderContext.h"
#include "SkSVGStop.h"
#include "SkSVGValue.h"

void SkSVGGradient::setHref(const SkSVGStringType& href) {
    fHref = std::move(href);
}

void SkSVGGradient::setGradientTransform(const SkSVGTransformType& t) {
    fGradientTransform = t;
}

void SkSVGGradient::setSpreadMethod(const SkSVGSpreadMethod& spread) {
    fSpreadMethod = spread;
}

void SkSVGGradient::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kGradientTransform:
        if (const auto* t = v.as<SkSVGTransformValue>()) {
            this->setGradientTransform(*t);
        }
        break;
    case SkSVGAttribute::kHref:
        if (const auto* href = v.as<SkSVGStringValue>()) {
            this->setHref(*href);
        }
        break;
    case SkSVGAttribute::kSpreadMethod:
        if (const auto* spread = v.as<SkSVGSpreadMethodValue>()) {
            this->setSpreadMethod(*spread);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
    }
}

// https://www.w3.org/TR/SVG/pservers.html#LinearGradientElementHrefAttribute
void SkSVGGradient::collectColorStops(const SkSVGRenderContext& ctx,
                                      StopPositionArray* pos,
                                      StopColorArray* colors) const {
    // Used to resolve percentage offsets.
    const SkSVGLengthContext ltx(SkSize::Make(1, 1));

    for (const auto& child : fChildren) {
        if (child->tag() != SkSVGTag::kStop) {
            continue;
        }

        const auto& stop = static_cast<const SkSVGStop&>(*child);
        colors->push_back(SkColorSetA(stop.stopColor(),
                                      SkScalarRoundToInt(stop.stopOpacity() * 255)));
        pos->push_back(SkTPin(ltx.resolve(stop.offset(), SkSVGLengthContext::LengthType::kOther),
                              0.f, 1.f));
    }

    SkASSERT(colors->count() == pos->count());

    if (pos->empty() && !fHref.value().isEmpty()) {
        const auto* ref = ctx.findNodeById(fHref);
        if (ref && (ref->tag() == SkSVGTag::kLinearGradient ||
                    ref->tag() == SkSVGTag::kRadialGradient)) {
            static_cast<const SkSVGGradient*>(ref)->collectColorStops(ctx, pos, colors);
        }
    }
}

bool SkSVGGradient::onAsPaint(const SkSVGRenderContext& ctx, SkPaint* paint) const {
    StopColorArray colors;
    StopPositionArray pos;

    this->collectColorStops(ctx, &pos, &colors);

    // TODO:
    //       * stop (lazy?) sorting
    //       * href loop detection
    //       * href attribute inheritance (not just color stops)
    //       * objectBoundingBox units support

    static_assert(static_cast<SkTileMode>(SkSVGSpreadMethod::Type::kPad) ==
                  SkTileMode::kClamp, "SkSVGSpreadMethod::Type is out of sync");
    static_assert(static_cast<SkTileMode>(SkSVGSpreadMethod::Type::kRepeat) ==
                  SkTileMode::kRepeat, "SkSVGSpreadMethod::Type is out of sync");
    static_assert(static_cast<SkTileMode>(SkSVGSpreadMethod::Type::kReflect) ==
                  SkTileMode::kMirror, "SkSVGSpreadMethod::Type is out of sync");
    const auto tileMode = static_cast<SkTileMode>(fSpreadMethod.type());

    paint->setShader(this->onMakeShader(ctx, colors.begin(), pos.begin(), colors.count(), tileMode,
                                        fGradientTransform.value()));
    return true;
}
