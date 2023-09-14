#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 a;
    half4 b;
    uint2 c;
    uint2 d;
    int3 e;
    int3 f;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 expectTTFF = bool4(true, true, false, false);
    bool4 expectFFTT = bool4(false, false, true, true);
    _out.sk_FragColor.x = half((_uniforms.a <= _uniforms.b).x);
    _out.sk_FragColor.y = half((_uniforms.c <= _uniforms.d).y);
    _out.sk_FragColor.z = half((_uniforms.e <= _uniforms.f).z);
    _out.sk_FragColor.w = half(any(expectTTFF) || any(expectFFTT));
    return _out;
}
