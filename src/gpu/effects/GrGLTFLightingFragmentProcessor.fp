/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@fields {
    GrCoordTransform fCoordTransform;
}

// layout(ctype=SkSize) and int2's don't seem fully supported in code gen
in half2 deviceSize;
uniform half2 deviceSizeUniform;
half2 previousSize;

// combined light position (xyz) and viewer distance (w)

in uniform half roughness;
in uniform half metallicness;
in uniform half4 lightAndViewer;

@clone {
    GrGLTFLightingFragmentProcessor::GrGLTFLightingFragmentProcessor(const GrGLTFLightingFragmentProcessor& src) :
        INHERITED(src.classID(), src.optimizationFlags()),
        fCoordTransform(src.fCoordTransform),
        fDeviceSize(src.fDeviceSize),
        fRoughness(src.fRoughness),
        fMetallicness(src.fMetallicness),
        fLightAndViewer(src.fLightAndViewer) { }

    std::unique_ptr<GrFragmentProcessor> GrGLTFLightingFragmentProcessor::clone() const {
        std::unique_ptr<GrGLTFLightingFragmentProcessor> fp(new GrGLTFLightingFragmentProcessor(*this));
        fp->addCoordTransform(&(fp->fCoordTransform));
        return fp;
    }
}

@constructorParams {
    SkMatrix transform
}

@initializers {
    fCoordTransform(transform)
}

@setData(pdman) {
// Must override set data even though deviceSize should be an "in uniform" since
// sksl generation outputs uniform1f instead of uniform2f
    if (previousSize != deviceSize) {
        pdman.set2f(deviceSizeUniform, deviceSize.fX, deviceSize.fY);
        previousSize = deviceSize;
    }
}

@make {
    static std::unique_ptr<GrGLTFLightingFragmentProcessor> Make(
            const SkISize& deviceSize, const SkPoint3& lightPos,
            const SkScalar& viewerDistance, const SkScalar& roughness,
            const SkScalar& metallicness) {
        std::unique_ptr<GrGLTFLightingFragmentProcessor> fp(
            new GrGLTFLightingFragmentProcessor(
                SkPoint::Make(deviceSize.fWidth, deviceSize.fHeight),
                roughness, metallicness,
                SkRect::MakeLTRB(lightPos.fX, lightPos.fY, lightPos.fZ, viewerDistance),
                SkMatrix::I()));

        fp->addCoordTransform(&(fp->fCoordTransform));
        return fp;
    }
}

float PI = 3.1415927;

void main() {
    // viewer position, lighting position, normals are all in 3D device space
    // so in some cases we pick a reasonable Z value in pixel size away from the screen
    float3 objPos = float3(sk_FragCoord.x, sk_FragCoord.y, 0);
    float3 viewerPos = float3(deviceSizeUniform / 2, lightAndViewer.w);

    float3 lightPos = lightAndViewer.xyz;


    float3 v = normalize(viewerPos - objPos);
    float3 l = normalize(lightPos - objPos);
    float3 n = float3(0, 0, 1);
    float3 h = normalize(v + l);

    // gltf input parameters (should be uniforms or textures or shader stages)
    float3 baseColor = float3(0.5, 0.2, 0.7);

    float3 dielectricSpecular = float3(0.04, 0.04, 0.04);
    float3 black = float3(0, 0, 0);

    // See https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#appendix-b-brdf-implementation
    float3 cDiff = (1 - metallicness) * baseColor * (1 - dielectricSpecular.x);
    float3 f0 = mix(dielectricSpecular, baseColor, metallicness);
    float alpha = roughness * roughness;

    // Schlick's simplied approximation
    float3 fresnel = f0; //f0 + (1 - f0) * pow(clamp(1 - dot(v, h), 0, 1), 5);


    // Diffuse component (Lambertian)
    float3 fDiffuse = (1 - fresnel) * cDiff / PI;

    // Geometric occlusion
    float k = alpha * sqrt(2 / PI);
    float lh = max(0.0, dot(l, h));
    float nh = max(0.0, dot(n, h));

    float go = lh / (lh * (1 - k) + k) * nh / (nh * (1 - k) + k);


    // Microfacet distribution
    float a2 = alpha * alpha;
    float mdDenom = nh * nh * (a2 - 1) + 1;
    float d = a2 / (PI * mdDenom * mdDenom);

    // Specular component
    float nl = max(0.0, dot(n, l));
    float nv = max(0.0, dot(n, v));
    float3 fSpecular = fresnel * go * d / (4 * nl * nv);

    sk_OutColor = float4(nl * (fDiffuse + fSpecular), 1);
    //sk_OutColor = float4(0.5*l+0.5, 1);
}
