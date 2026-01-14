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
    int _skTemp0;
    uint _skTemp1;
    int2 _skTemp2;
    uint2 _skTemp3;
    int3 _skTemp4;
    uint3 _skTemp5;
    int4 _skTemp6;
    uint4 _skTemp7;
    int b1 = (_skTemp0 = (_uniforms.a), select((ctz(_skTemp0)), int(-1), _skTemp0 == int(0))) + (_skTemp1 = (_uniforms.b), select(int(ctz(_skTemp1)), int(-1), _skTemp1 == uint(0)));
    int2 b2 = (_skTemp2 = (int2(_uniforms.a)), select((ctz(_skTemp2)), int2(-1), _skTemp2 == int2(0))) + (_skTemp3 = (uint2(_uniforms.b)), select(int2(ctz(_skTemp3)), int2(-1), _skTemp3 == uint2(0)));
    int3 b3 = (_skTemp4 = (int3(_uniforms.a)), select((ctz(_skTemp4)), int3(-1), _skTemp4 == int3(0))) + (_skTemp5 = (uint3(_uniforms.b)), select(int3(ctz(_skTemp5)), int3(-1), _skTemp5 == uint3(0)));
    int4 b4 = (_skTemp6 = (int4(_uniforms.a)), select((ctz(_skTemp6)), int4(-1), _skTemp6 == int4(0))) + (_skTemp7 = (uint4(_uniforms.b)), select(int4(ctz(_skTemp7)), int4(-1), _skTemp7 == uint4(0)));
    _out.sk_FragColor = half4(((int4(b1) + b2.xyxy) + int4(b3, 1)) + b4);
    return _out;
}
