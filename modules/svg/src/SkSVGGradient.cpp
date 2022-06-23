/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTPin.h"
#include "modules/svg/include/SkSVGGradient.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGStop.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGGradient::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setGradientTransform(SkSVGAttributeParser::parse<SkSVGTransformType>(
                   "gradientTransform", name, value)) ||
           this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", name, value)) ||
           this->setSpreadMethod(
                   SkSVGAttributeParser::parse<SkSVGSpreadMethod>("spreadMethod", name, value)) ||
           this->setGradientUnits(SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>(
                   "gradientUnits", name, value));
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementHrefAttribute
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
        colors->push_back(this->resolveStopColor(ctx, stop));
        pos->push_back(SkTPin(ltx.resolve(stop.getOffset(), SkSVGLengthContext::LengthType::kOther),
                              0.f, 1.f));
    }

    SkASSERT(colors->count() == pos->count());

    if (pos->empty() && !fHref.iri().isEmpty()) {
        const auto ref = ctx.findNodeById(fHref);
        if (ref && (ref->tag() == SkSVGTag::kLinearGradient ||
                    ref->tag() == SkSVGTag::kRadialGradient)) {
            static_cast<const SkSVGGradient*>(ref.get())->collectColorStops(ctx, pos, colors);
        }
    }
}

SkColor4f SkSVGGradient::resolveStopColor(const SkSVGRenderContext& ctx,
                                          const SkSVGStop& stop) const {
    const auto& stopColor = stop.getStopColor();
    const auto& stopOpacity = stop.getStopOpacity();
    // Uninherited presentation attrs should have a concrete value at this point.
    if (!stopColor.isValue() || !stopOpacity.isValue()) {
        SkDebugf("unhandled: stop-color or stop-opacity has no value\n");
        return SkColors::kBlack;
    }

    const auto color = SkColor4f::FromColor(ctx.resolveSvgColor(*stopColor));

    return { color.fR, color.fG, color.fB, *stopOpacity };
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

    const auto obbt = ctx.transformForCurrentOBB(fGradientUnits);
    const auto localMatrix = SkMatrix::Translate(obbt.offset.x, obbt.offset.y)
                           * SkMatrix::Scale(obbt.scale.x, obbt.scale.y)
                           * fGradientTransform;

    paint->setShader(this->onMakeShader(ctx, colors.begin(), pos.begin(), colors.count(), tileMode,
                                        localMatrix));
    return true;
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementSpreadMethodAttribute
template <>
bool SkSVGAttributeParser::parse(SkSVGSpreadMethod* spread) {
    static const struct {
        SkSVGSpreadMethod::Type fType;
        const char*             fName;
    } gSpreadInfo[] = {
        { SkSVGSpreadMethod::Type::kPad    , "pad"     },
        { SkSVGSpreadMethod::Type::kReflect, "reflect" },
        { SkSVGSpreadMethod::Type::kRepeat , "repeat"  },
    };

    bool parsedValue = false;
    for (size_t i = 0; i < std::size(gSpreadInfo); ++i) {
        if (this->parseExpectedStringToken(gSpreadInfo[i].fName)) {
            *spread = SkSVGSpreadMethod(gSpreadInfo[i].fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}
