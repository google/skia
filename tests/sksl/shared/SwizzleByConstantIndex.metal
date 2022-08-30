#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half4 constant_swizzle_h4(Uniforms _uniforms) {
    half4 v = _uniforms.testInputs;
    half x = v.x;
    half y = v.y;
    half z = v.z;
    half w = v.w;
    return half4(x, y, z, w);
}
half4 foldable_index_h4(Uniforms _uniforms) {
    half x = _uniforms.testInputs.x;
    half y = _uniforms.testInputs.y;
    half z = _uniforms.testInputs.z;
    half w = _uniforms.testInputs.w;
    return half4(x, y, z, w);
}
half4 foldable_h4() {
    half4 v = half4(0.0h, 1.0h, 2.0h, 3.0h);
    half x = v.x;
    half y = v.y;
    half z = v.z;
    half w = v.w;
    return half4(x, y, z, w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 a = constant_swizzle_h4(_uniforms);
    half4 b = foldable_index_h4(_uniforms);
    half4 c = foldable_h4();
    _out.sk_FragColor = (all(a == half4(-1.25h, 0.0h, 0.75h, 2.25h)) && all(b == half4(-1.25h, 0.0h, 0.75h, 2.25h))) && all(c == half4(0.0h, 1.0h, 2.0h, 3.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
