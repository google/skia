#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    int a;
    uint b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int b1 = (popcount(_uniforms.a)) + int(popcount(_uniforms.b));
    int2 b2 = (popcount(int2(_uniforms.a))) + int2(popcount(uint2(_uniforms.b)));
    int3 b3 = (popcount(int3(_uniforms.a))) + int3(popcount(uint3(_uniforms.b)));
    int4 b4 = (popcount(int4(_uniforms.a))) + int4(popcount(uint4(_uniforms.b)));
    _out.sk_FragColor = half4(((int4(b1) + b2.xyxy) + int4(b3, 1)) + b4);
    return _out;
}
