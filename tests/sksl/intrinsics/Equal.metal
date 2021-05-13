#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 a;
    float4 b;
    uint2 c;
    uint2 d;
    int3 e;
    int3 f;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 expectTTFF = bool4(true, true, false, false);
    bool4 expectFFTT = bool4(false, false, true, true);
    _out.sk_FragColor.x = float((_uniforms.a == _uniforms.b).x ? 1 : 0);
    _out.sk_FragColor.y = float((_uniforms.c == _uniforms.d).y ? 1 : 0);
    _out.sk_FragColor.z = float((_uniforms.e == _uniforms.f).z ? 1 : 0);
    _out.sk_FragColor.w = float(any(expectTTFF) || any(expectFFTT) ? 1 : 0);
    return _out;
}
