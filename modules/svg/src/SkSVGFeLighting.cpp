/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeLighting.h"

#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFeLightSource.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGRenderContext.h"

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
            case SkSVGTag::kFeDistantLight:
                return this->makeDistantLight(
                        ctx, fctx, static_cast<const SkSVGFeDistantLight*>(child.get()));
            case SkSVGTag::kFePointLight:
                return this->makePointLight(
                        ctx, fctx, static_cast<const SkSVGFePointLight*>(child.get()));
            case SkSVGTag::kFeSpotLight:
                return this->makeSpotLight(
                        ctx, fctx, static_cast<const SkSVGFeSpotLight*>(child.get()));
            default:
                // Ignore unknown children, such as <desc> elements
                break;
        }
    }

    SkDEBUGF("lighting filter effect needs exactly one light source\n");
    return nullptr;
}

SkColor SkSVGFeLighting::resolveLightingColor(const SkSVGRenderContext& ctx) const {
    const auto color = this->getLightingColor();
    if (!color.isValue()) {
        // Uninherited presentation attributes should have a concrete value by now.
        SkDEBUGF("unhandled: lighting-color has no value\n");
        return SK_ColorWHITE;
    }

    return ctx.resolveSvgColor(*color);
}

SkPoint3 SkSVGFeLighting::resolveXYZ(const SkSVGRenderContext& ctx,
                                     const SkSVGFilterContext& fctx,
                                     SkSVGNumberType x,
                                     SkSVGNumberType y,
                                     SkSVGNumberType z) const {
    const auto obbt = ctx.transformForCurrentOBB(fctx.primitiveUnits());
    const auto xy = SkV2{x,y} * obbt.scale + obbt.offset;
    z = SkSVGLengthContext({obbt.scale.x, obbt.scale.y})
            .resolve(SkSVGLength(z * 100.f, SkSVGLength::Unit::kPercentage),
                     SkSVGLengthContext::LengthType::kOther);
    return SkPoint3::Make(xy.x, xy.y, z);
}

bool SkSVGFeSpecularLighting::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setSpecularConstant(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("specularConstant", n, v)) ||
           this->setSpecularExponent(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("specularExponent", n, v));
}

sk_sp<SkImageFilter> SkSVGFeSpecularLighting::makeDistantLight(
        const SkSVGRenderContext& ctx,
        const SkSVGFilterContext& fctx,
        const SkSVGFeDistantLight* light) const {
    const SkPoint3 dir = light->computeDirection();
    return SkImageFilters::DistantLitSpecular(
            this->resolveXYZ(ctx, fctx, dir.fX, dir.fY, dir.fZ),
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            fSpecularConstant,
            fSpecularExponent,
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
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

sk_sp<SkImageFilter> SkSVGFeSpecularLighting::makeSpotLight(const SkSVGRenderContext& ctx,
                                                            const SkSVGFilterContext& fctx,
                                                            const SkSVGFeSpotLight* light) const {
    const auto& limitingConeAngle = light->getLimitingConeAngle();
    const float cutoffAngle = limitingConeAngle.isValid() ? *limitingConeAngle : 180.f;

    return SkImageFilters::SpotLitSpecular(
            this->resolveXYZ(ctx, fctx, light->getX(), light->getY(), light->getZ()),
            this->resolveXYZ(
                    ctx, fctx, light->getPointsAtX(), light->getPointsAtY(), light->getPointsAtZ()),
            light->getSpecularExponent(),
            cutoffAngle,
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            fSpecularConstant,
            fSpecularExponent,
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

bool SkSVGFeDiffuseLighting::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setDiffuseConstant(
                   SkSVGAttributeParser::parse<SkSVGNumberType>("diffuseConstant", n, v));
}

sk_sp<SkImageFilter> SkSVGFeDiffuseLighting::makeDistantLight(
        const SkSVGRenderContext& ctx,
        const SkSVGFilterContext& fctx,
        const SkSVGFeDistantLight* light) const {
    const SkPoint3 dir = light->computeDirection();
    return SkImageFilters::DistantLitDiffuse(
            this->resolveXYZ(ctx, fctx, dir.fX, dir.fY, dir.fZ),
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            this->getDiffuseConstant(),
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

sk_sp<SkImageFilter> SkSVGFeDiffuseLighting::makePointLight(const SkSVGRenderContext& ctx,
                                                            const SkSVGFilterContext& fctx,
                                                            const SkSVGFePointLight* light) const {
    return SkImageFilters::PointLitDiffuse(
            this->resolveXYZ(ctx, fctx, light->getX(), light->getY(), light->getZ()),
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            this->getDiffuseConstant(),
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}

sk_sp<SkImageFilter> SkSVGFeDiffuseLighting::makeSpotLight(const SkSVGRenderContext& ctx,
                                                           const SkSVGFilterContext& fctx,
                                                           const SkSVGFeSpotLight* light) const {
    const auto& limitingConeAngle = light->getLimitingConeAngle();
    const float cutoffAngle = limitingConeAngle.isValid() ? *limitingConeAngle : 180.f;

    return SkImageFilters::SpotLitDiffuse(
            this->resolveXYZ(ctx, fctx, light->getX(), light->getY(), light->getZ()),
            this->resolveXYZ(
                    ctx, fctx, light->getPointsAtX(), light->getPointsAtY(), light->getPointsAtZ()),
            light->getSpecularExponent(),
            cutoffAngle,
            this->resolveLightingColor(ctx),
            this->getSurfaceScale(),
            this->getDiffuseConstant(),
            fctx.resolveInput(ctx, this->getIn(), this->resolveColorspace(ctx, fctx)),
            this->resolveFilterSubregion(ctx, fctx));
}
