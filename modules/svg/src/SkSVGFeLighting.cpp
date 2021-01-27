/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint3.h"
#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeLightSource.h"
#include "modules/svg/include/SkSVGFeLighting.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

bool SkSVGFeLighting::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setSurfaceScale(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("surfaceScale", n, v)) ||
           this->setKernelUnitLength(SkSVGAttributeParser::parse<SkSVGFeLighting::KernelUnitLength>(
                   "kernelUnitLength", n, v));
}

template <>
bool SkSVGAttributeParser::parse<SkSVGFeLighting::KernelUnitLength>(
        SkSVGFeLighting::KernelUnitLength* kernelUnitLength) {
    std::vector<SkSVGNumberType> values;
    if (!this->parse(&values)) {
        return false;
    }

    kernelUnitLength->fDx = values[0];
    kernelUnitLength->fDy = values.size() > 1 ? values[1] : values[0];
    return true;
}

sk_sp<SkImageFilter> SkSVGFeLighting::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                        const SkSVGFilterContext& fctx) const {
    for (const auto& child : fChildren) {
        switch (child->tag()) {
            case SkSVGTag::kFePointLight:
                return this->makePointLight(ctx, fctx,
                                            static_cast<const SkSVGFePointLight*>(child.get()));
            default:
                // Ignore unknown children, such as <desc> elements
                break;
        }
    }

    SkDebugf("lighting filter effect needs exactly one light source");
    return nullptr;
}

SkColor SkSVGFeLighting::resolveLightingColor(const SkSVGRenderContext& ctx) const {
    const auto color = this->getLightingColor();
    if (!color.isValue()) {
        // Uninherited presentation attributes should have a concrete value by now.
        SkDebugf("unhandled: lighting-color has no value\n");
        return SK_ColorWHITE;
    }

    return ctx.resolveSvgColor(*color);
}

SkPoint3 SkSVGFeLighting::resolveXYZ(const SkSVGRenderContext& ctx,
                                     const SkSVGFilterContext& fctx,
                                     SkSVGNumberType x,
                                     SkSVGNumberType y,
                                     SkSVGNumberType z) const {
    if (fctx.primitiveUnits().type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        SkASSERT(ctx.node());
        const SkRect objBounds = ctx.node()->objectBoundingBox(ctx);
        const SkSVGLengthContext lctx({objBounds.width(), objBounds.height()});
        x = objBounds.left() + x * objBounds.width();
        y = objBounds.top() + y * objBounds.height();
        z = lctx.resolve(SkSVGLength(z * 100.f, SkSVGLength::Unit::kPercentage),
                         SkSVGLengthContext::LengthType::kOther);
    }
    return SkPoint3::Make(x, y, z);
}

bool SkSVGFeSpecularLighting::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setSpecularConstant(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("specularConstant", n, v)) ||
           this->setSpecularExponent(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("specularExponent", n, v));
}

sk_sp<SkImageFilter> SkSVGFeSpecularLighting::makePointLight(const SkSVGRenderContext& ctx,
                                                             const SkSVGFilterContext& fctx,
                                                             const SkSVGFePointLight* light) const {
    return SkImageFilters::PointLitSpecular(
            this->resolveXYZ(ctx, fctx, light->getX(), light->getY(), light->getZ()),
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            fSpecularConstant,
            fSpecularExponent,
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}
