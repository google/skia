#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    int i = 5;
    ++i;
    ok = ok && i == 6;
    ok = ok && ++i == 7;
    ok = ok && --i == 6;
    --i;
    ok = ok && i == 5;
    float f = 0.5;
    ++f;
    ok = ok && f == 1.5;
    ok = ok && ++f == 2.5;
    ok = ok && --f == 1.5;
    --f;
    ok = ok && f == 0.5;
    ok = ok && !(_uniforms.colorGreen.x == 1.0h);
    uint val = uint(_uniforms.colorGreen.x);
    uint2 mask = uint2(val, ~val);
    int2 imask = int2(~mask);
    mask = ~mask & uint2(~imask);
    ok = ok && all(mask == uint2(0u));
    half one = _uniforms.colorGreen.x;
    half4x4 m = half4x4(one);
    _out.sk_FragColor = ok ? (-1.0h * m) * -_uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
