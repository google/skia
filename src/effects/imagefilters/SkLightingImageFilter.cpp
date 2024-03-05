/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

struct SkISize;

namespace {
// The 3D points/vectors used for lighting don't have a great analog for the rest of the image
// filtering system, and don't have any representation for ParameterSpace and LayerSpace. The
// SVG spec is also vague on how to handle managing them. Using the principle of least-surprise,
// the X and Y coordinates are treated as ParameterSpace<SkPoint|Vector> and the Z will be
// scaled by the average of the X and Y scale factors when tranforming to layer space. For uniform
// scaling transforms, this has the desirable behavior of uniformly scaling the Z axis as well.
struct ZValue {
    ZValue() : fZ(0.f) {}
    ZValue(float z) : fZ(z) {}
    operator float() const { return fZ; }

    float fZ;
};
} // anonymous namespace

namespace skif {
template<>
class LayerSpace<ZValue> {
public:
    LayerSpace() = default;
    explicit LayerSpace(ZValue z) : fData(z) {}

    float val() const { return fData.fZ; }

    static LayerSpace<ZValue> Map(const Mapping& mapping, ParameterSpace<ZValue> z) {
        // See comment on ZValue for rationale.
        skif::LayerSpace<skif::Vector> z2d = mapping.paramToLayer(
                skif::ParameterSpace<skif::Vector>({ZValue(z), ZValue(z)}));
        return LayerSpace<ZValue>(SkScalarAve(z2d.x(), z2d.y()));
    }

private:
    ZValue fData;
};
} // namespace skif

namespace {

struct Light {
    enum class Type {
        kDistant,
        kPoint,
        kSpot,
        kLast = kSpot
    };

    Type fType;
    SkColor fLightColor; // All lights

    // Location and direction are decomposed into typed XY and Z for how they are transformed from
    // parameter space to layer space.
    skif::ParameterSpace<SkPoint> fLocationXY; // Spotlight and point lights only
    skif::ParameterSpace<ZValue>  fLocationZ;  //  ""

    skif::ParameterSpace<skif::Vector> fDirectionXY; // Spotlight and distant lights only
    skif::ParameterSpace<ZValue>       fDirectionZ;  //  ""

    // Spotlight only (and unchanged by layer matrix)
    float fFalloffExponent;
    float fCosCutoffAngle;

    static Light Point(SkColor color, const SkPoint3& location) {
        return {Type::kPoint,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                /*directionXY=*/{},
                /*directionZ=*/{},
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Distant(SkColor color, const SkPoint3& direction) {
        return {Type::kDistant,
                color,
                /*locationXY=*/{},
                /*locationZ=*/{},
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Spot(SkColor color, const SkPoint3& location, const SkPoint3& direction,
                      float falloffExponent, float cosCutoffAngle) {
        return {Type::kSpot,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                falloffExponent,
                cosCutoffAngle};
    }
};

struct Material {
    enum class Type {
        kDiffuse,
        kSpecular,
        kLast = kSpecular
    };

    Type fType;
    // The base scale factor applied to alpha image to go from [0-1] to [0-depth] before computing
    // surface normals.
    skif::ParameterSpace<ZValue> fSurfaceDepth;

    // Non-geometric
    float fK; // Reflectance coefficient
    float fShininess; // Specular only

    static Material Diffuse(float k, float surfaceDepth) {
        return {Type::kDiffuse, skif::ParameterSpace<ZValue>(surfaceDepth), k, 0.f};
    }

    static Material Specular(float k, float shininess, float surfaceDepth) {
        return {Type::kSpecular, skif::ParameterSpace<ZValue>(surfaceDepth), k, shininess};
    }
};

class SkLightingImageFilter final : public SkImageFilter_Base {
public:
    SkLightingImageFilter(const Light& light, const Material& material, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fLight(light)
            , fMaterial(material) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterLightingImageFilterFlattenables();
    SK_FLATTENABLE_HOOKS(SkLightingImageFilter)
    static Light LegacyDeserializeLight(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacyDiffuseCreateProc(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacySpecularCreateProc(SkReadBuffer& buffer);

    bool onAffectsTransparentBlack() const override { return true; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::LayerSpace<SkIRect> requiredInput(const skif::LayerSpace<SkIRect>& desiredOutput) const {
        // We request 1px of padding so that the visible normal map can do a regular Sobel kernel
        // eval. The Sobel kernel is always applied in layer pixels
        skif::LayerSpace<SkIRect> requiredInput = desiredOutput;
        requiredInput.outset(skif::LayerSpace<SkISize>({1, 1}));
        return requiredInput;
    }

    Light fLight;
    Material fMaterial;
};

// Creates a shader that performs a Sobel filter on the alpha channel of the input image, using
// 'edgeBounds' to decide how to modify the kernel weights.
sk_sp<SkShader> make_normal_shader(sk_sp<SkShader> alphaMap,
                                   const skif::LayerSpace<SkIRect>& edgeBounds,
                                   skif::LayerSpace<ZValue> surfaceDepth) {
    const SkRuntimeEffect* normalEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kNormal);

    SkRuntimeShaderBuilder builder(sk_ref_sp(normalEffect));
    builder.child("alphaMap") = std::move(alphaMap);
    builder.uniform("edgeBounds") = SkRect::Make(SkIRect(edgeBounds)).makeInset(0.5f, 0.5f);
    builder.uniform("negSurfaceDepth") = -surfaceDepth.val();

    return builder.makeShader();
}

sk_sp<SkShader> make_lighting_shader(sk_sp<SkShader> normalMap,
                                     Light::Type lightType,
                                     SkColor lightColor,
                                     skif::LayerSpace<SkPoint> locationXY,
                                     skif::LayerSpace<ZValue> locationZ,
                                     skif::LayerSpace<skif::Vector> directionXY,
                                     skif::LayerSpace<ZValue> directionZ,
                                     float falloffExponent,
                                     float cosCutoffAngle,
                                     Material::Type matType,
                                     skif::LayerSpace<ZValue> surfaceDepth,
                                     float k,
                                     float shininess) {

    const SkRuntimeEffect* lightingEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLighting);

    SkRuntimeShaderBuilder builder(sk_ref_sp(lightingEffect));
    builder.child("normalMap") = std::move(normalMap);

    builder.uniform("materialAndLightType") =
            SkV4{surfaceDepth.val(),
                 shininess,
                 matType == Material::Type::kDiffuse ? 0.f : 1.f,
                 lightType == Light::Type::kPoint ?
                         0.f : (lightType == Light::Type::kDistant ? -1.f : 1.f)};
    builder.uniform("lightPosAndSpotFalloff") =
            SkV4{locationXY.x(), locationXY.y(), locationZ.val(), falloffExponent};

    // Pre-normalize the light direction, but this can be (0,0,0) for point lights, which won't use
    // the uniform anyways. Avoid a division by 0 to keep ASAN happy or in the event that a spot/dir
    // light have bad user input.
    SkV3 dir{directionXY.x(), directionXY.y(), directionZ.val()};
    float invDirLen = dir.length();
    invDirLen = invDirLen ? 1.0f / invDirLen : 0.f;
    builder.uniform("lightDirAndSpotCutoff") =
            SkV4{invDirLen*dir.x, invDirLen*dir.y, invDirLen*dir.z, cosCutoffAngle};

    // Historically, the Skia lighting image filter did not apply any color space transformation to
    // the light's color. The SVG spec for the lighting effects does not stipulate how to interpret
    // the color for a light. Overall, it does not have a principled physically based approach, but
    // the closest way to interpret it, is:
    //  - the material's K is a uniformly distributed reflectance coefficient
    //  - lighting *should* be calculated in a linear color space, which is the default for SVG
    //    filters. Chromium manages these color transformations using SkImageFilters::ColorFilter
    //    so it's not necessarily reflected in the Context's color space.
    //  - it's unspecified in the SVG spec if the light color should be transformed to linear or
    //    interpreted as linear already. Regardless, if there was any transformation that needed to
    //    occur, Blink took care of it in the past so adding color space management to the light
    //    color would be a breaking change.
    //  - so for now, leave the color un-modified and apply K up front since no color space
    //    transforms need to be performed on the original light color.
    const float colorScale = k / 255.f;
    builder.uniform("lightColor") = SkV3{SkColorGetR(lightColor) * colorScale,
                                         SkColorGetG(lightColor) * colorScale,
                                         SkColorGetB(lightColor) * colorScale};

    return builder.makeShader();
}

sk_sp<SkImageFilter> make_lighting(const Light& light,
                                   const Material& material,
                                   sk_sp<SkImageFilter> input,
                                   const SkImageFilters::CropRect& cropRect) {
    // According to the spec, ks and kd can be any non-negative number:
    // http://www.w3.org/TR/SVG/filters.html#feSpecularLightingElement
    if (!SkScalarIsFinite(material.fK) || material.fK < 0.f ||
        !SkScalarIsFinite(material.fShininess) ||
        !SkScalarIsFinite(ZValue(material.fSurfaceDepth))) {
        return nullptr;
    }

    // Ensure light values are finite, and the cosine should be between -1 and 1
    if (!SkPoint(light.fLocationXY).isFinite() ||
        !SkScalarIsFinite(ZValue(light.fLocationZ)) ||
        !skif::Vector(light.fDirectionXY).isFinite() ||
        !SkScalarIsFinite(ZValue(light.fDirectionZ)) ||
        !SkScalarIsFinite(light.fFalloffExponent) ||
        !SkScalarIsFinite(light.fCosCutoffAngle) ||
        light.fCosCutoffAngle < -1.f || light.fCosCutoffAngle > 1.f) {
        return nullptr;
    }

    // If a crop rect is provided, it clamps both the input (to better match the SVG's normal
    // boundary condition spec) and the output (because otherwise it has infinite bounds).
    sk_sp<SkImageFilter> filter = std::move(input);
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, std::move(filter));
    }
    filter = sk_sp<SkImageFilter>(
            new SkLightingImageFilter(light, material, std::move(filter)));
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, std::move(filter));
    }
    return filter;
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

void SkRegisterLightingImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkLightingImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDiffuseLightingImageFilter",
                            SkLightingImageFilter::LegacyDiffuseCreateProc);
    SkFlattenable::Register("SkSpecularLightingImageFilter",
                            SkLightingImageFilter::LegacySpecularCreateProc);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light;
    light.fType = buffer.read32LE(Light::Type::kLast);
    light.fLightColor = buffer.readColor();

    SkPoint3 lightPos, lightDir;
    buffer.readPoint3(&lightPos);
    light.fLocationXY = skif::ParameterSpace<SkPoint>({lightPos.fX, lightPos.fY});
    light.fLocationZ = skif::ParameterSpace<ZValue>(lightPos.fZ);

    buffer.readPoint3(&lightDir);
    light.fDirectionXY = skif::ParameterSpace<skif::Vector>({lightDir.fX, lightDir.fY});
    light.fDirectionZ = skif::ParameterSpace<ZValue>(lightDir.fZ);

    light.fFalloffExponent = buffer.readScalar();
    light.fCosCutoffAngle = buffer.readScalar();

    Material material;
    material.fType = buffer.read32LE(Material::Type::kLast);
    material.fSurfaceDepth = skif::ParameterSpace<ZValue>(buffer.readScalar());
    material.fK = buffer.readScalar();
    material.fShininess = buffer.readScalar();

    if (!buffer.isValid()) {
        return nullptr;
    }

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

Light SkLightingImageFilter::LegacyDeserializeLight(SkReadBuffer& buffer) {
    // Light::Type has the same order as the legacy SkImageFilterLight::LightType enum
    Light::Type lightType = buffer.read32LE(Light::Type::kLast);
    if (!buffer.isValid()) {
        return {};
    }

    // Legacy lights stored just the RGB, but as floats (notably *not* normalized to [0-1])
    SkColor lightColor = SkColorSetARGB(/*a (ignored)=*/255,
                                        /*r=*/ (U8CPU) buffer.readScalar(),
                                        /*g=*/ (U8CPU) buffer.readScalar(),
                                        /*b=*/ (U8CPU) buffer.readScalar());
    // Legacy lights only serialized fields specific to that type
    switch (lightType) {
        case Light::Type::kDistant: {
            SkPoint3 dir = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Distant(lightColor, dir);
        }
        case Light::Type::kPoint: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Point(lightColor, loc);
        }
        case Light::Type::kSpot: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            SkPoint3 target = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            float falloffExponent = buffer.readScalar();
            float cosOuterConeAngle = buffer.readScalar();
            buffer.readScalar(); // skip cosInnerConeAngle, derived from outer cone angle
            buffer.readScalar(); // skip coneScale, which is a constant
            buffer.readScalar(); // skip S, which is normalize(target - loc)
            buffer.readScalar(); //  ""
            buffer.readScalar(); //  ""
            return Light::Spot(lightColor, loc, target - loc, falloffExponent, cosOuterConeAngle);
        }
    }

    SkUNREACHABLE; // Validation by read32LE() should avoid this
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacyDiffuseCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale.
    float surfaceScale = buffer.readScalar();
    float kd = buffer.readScalar();
    Material material = Material::Diffuse(kd, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacySpecularCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale.
    float surfaceScale = buffer.readScalar();
    float ks = buffer.readScalar();
    float shininess = buffer.readScalar();
    Material material = Material::Specular(ks, shininess, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

void SkLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);

    // Light
    buffer.writeInt((int) fLight.fType);
    buffer.writeColor(fLight.fLightColor);

    buffer.writePoint(SkPoint(fLight.fLocationXY));
    buffer.writeScalar(ZValue(fLight.fLocationZ));

    skif::Vector dirXY{fLight.fDirectionXY};
    buffer.writePoint(SkPoint{dirXY.fX, dirXY.fY});
    buffer.writeScalar(ZValue(fLight.fDirectionZ));

    buffer.writeScalar(fLight.fFalloffExponent);
    buffer.writeScalar(fLight.fCosCutoffAngle);

    // Material
    buffer.writeInt((int) fMaterial.fType);
    buffer.writeScalar(ZValue(fMaterial.fSurfaceDepth));
    buffer.writeScalar(fMaterial.fK);
    buffer.writeScalar(fMaterial.fShininess);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkLightingImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    auto mapZToLayer = [&ctx](skif::ParameterSpace<ZValue> z) {
        return skif::LayerSpace<ZValue>::Map(ctx.mapping(), z);
    };

    // Map lighting and material parameters into layer space
    skif::LayerSpace<ZValue> surfaceDepth = mapZToLayer(fMaterial.fSurfaceDepth);
    skif::LayerSpace<SkPoint> lightLocationXY = ctx.mapping().paramToLayer(fLight.fLocationXY);
    skif::LayerSpace<ZValue> lightLocationZ = mapZToLayer(fLight.fLocationZ);
    skif::LayerSpace<skif::Vector> lightDirXY = ctx.mapping().paramToLayer(fLight.fDirectionXY);
    skif::LayerSpace<ZValue> lightDirZ = mapZToLayer(fLight.fDirectionZ);

    // The normal map is determined by a 3x3 kernel, so we request a 1px outset of what should be
    // filled by the lighting equation. Ideally this means there are no boundary conditions visible.
    // If the required input is incomplete, the lighting filter handles the boundaries in two ways:
    // - When the actual child output's edge matches the desired output's edge, it uses clamped
    //   tiling at the desired output. This approximates the modified Sobel kernel's specified in
    //   https://drafts.fxtf.org/filter-effects/#feDiffuseLightingElement. NOTE: It's identical to
    //   the interior kernel and near equal on the 4 edges (only weights are biased differently).
    //   The four corners' convolution sums with clamped tiling are not equal, but should not be
    //   objectionable since the normals produced are reasonable and still further processed by the
    //   lighting equation. The increased complexity is not worth it for just 4 pixels of output.
    // - However, when the desired output is far larger than the produced image, we process the
    //   child output with the default decal tiling that the Skia image filter pipeline relies on.
    //   This creates a visual bevel at the image boundary but avoids producing streaked normals if
    //   the clamped tiling was used in all scenarios.
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(ctx.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, ctx.withNewDesiredOutput(requiredInput));

    skif::LayerSpace<SkIRect> clampRect = requiredInput; // effectively no clamping of normals
    if (!childOutput.layerBounds().contains(requiredInput)) {
        // Adjust clampRect edges to desiredOutput if the actual child output matched the lighting
        // output size (typical SVG case). Otherwise leave coordinates alone to use decal tiling
        // automatically for the pixels outside the child image but inside the desired output.
        auto edgeClamp = [](int actualEdgeValue, int requestedEdgeValue, int outputEdge) {
            return actualEdgeValue == outputEdge ? outputEdge : requestedEdgeValue;
        };
        auto inputRect = childOutput.layerBounds();
        auto clampTo = ctx.desiredOutput();
        clampRect = skif::LayerSpace<SkIRect>({
                edgeClamp(inputRect.left(),   requiredInput.left(),   clampTo.left()),
                edgeClamp(inputRect.top(),    requiredInput.top(),    clampTo.top()),
                edgeClamp(inputRect.right(),  requiredInput.right(),  clampTo.right()),
                edgeClamp(inputRect.bottom(), requiredInput.bottom(), clampTo.bottom())});
    }

    skif::FilterResult::Builder builder{ctx};
    builder.add(childOutput, /*sampleBounds=*/clampRect, ShaderFlags::kSampledRepeatedly);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> input) {
        // TODO: Once shaders are deferred in FilterResult, it will likely make sense to have an
        // internal normal map filter that uses this shader, and then have the lighting effects as
        // a separate filter. It's common for multiple lights to use the same input (producing the
        // same normal map) before being merged together. With a separate normal image filter, its
        // output would be automatically cached, and the lighting equation shader would be deferred
        // to the merge's draw operation, making for a maximum of 2 renderpasses instead of N+1.
        sk_sp<SkShader> normals = make_normal_shader(std::move(input[0]), clampRect, surfaceDepth);
        return make_lighting_shader(std::move(normals),
                                    // Light in layer space
                                    fLight.fType,
                                    fLight.fLightColor,
                                    lightLocationXY,
                                    lightLocationZ,
                                    lightDirXY,
                                    lightDirZ,
                                    fLight.fFalloffExponent,
                                    fLight.fCosCutoffAngle,
                                    // Material in layer space
                                    fMaterial.fType,
                                    surfaceDepth,
                                    fMaterial.fK,
                                    fMaterial.fShininess);
    });
}

skif::LayerSpace<SkIRect> SkLightingImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkLightingImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // The lighting equation is defined on the entire plane, even if the input image that defines
    // the normal map is bounded. It just is evaluated at a constant normal vector, which can still
    // produce non-constant color since the direction to the eye and light change per pixel.
    return skif::LayerSpace<SkIRect>::Unbounded();
}

SkRect SkLightingImageFilter::computeFastBounds(const SkRect& src) const {
    return SkRectPriv::MakeLargeS32();
}
