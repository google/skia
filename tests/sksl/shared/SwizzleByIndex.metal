#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorBlack;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half4 non_constant_swizzle_h4(Uniforms _uniforms) {
    half4 v = _uniforms.testInputs;
    int4 i = int4(_uniforms.colorBlack);
    half x = v[i.x];
    half y = v[i.y];
    half z = v[i.z];
    half w = v[i.w];
    return half4(x, y, z, w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = all(non_constant_swizzle_h4(_uniforms) == half4(-1.25h, -1.25h, -1.25h, 0.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
