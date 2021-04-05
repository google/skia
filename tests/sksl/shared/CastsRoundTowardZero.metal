#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    ok = ok && all(float4(int4(0, 0, 1, 2)) == float4(int4(float4(0.0099999997764825821, 0.99000000953674316, 1.4900000095367432, 2.75))));
    ok = ok && all(float4(int4(0, 0, -1, -2)) == float4(int4(float4(-0.0099999997764825821, -0.99000000953674316, -1.4900000095367432, -2.75))));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
