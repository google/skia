#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 active = _uniforms.colorGreen;
    half4 centroid = _uniforms.colorGreen;
    half4 coherent = _uniforms.colorGreen;
    half4 common = _uniforms.colorGreen;
    half4 filter = _uniforms.colorGreen;
    half4 partition = _uniforms.colorGreen;
    half4 patch = _uniforms.colorGreen;
    half4 precise = _uniforms.colorGreen;
    half4 resource = _uniforms.colorGreen;
    half4 restrict = _uniforms.colorGreen;
    half4 shared = _uniforms.colorGreen;
    half4 smooth = _uniforms.colorGreen;
    half4 subroutine = _uniforms.colorGreen;
    _out.sk_FragColor = (((((((((((active * centroid) * coherent) * common) * filter) * partition) * patch) * precise) * resource) * restrict) * shared) * smooth) * subroutine;
    return _out;
}
