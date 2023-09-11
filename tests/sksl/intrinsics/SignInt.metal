#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int _skTemp0;
    int2 _skTemp1;
    int3 _skTemp2;
    int4 _skTemp3;
    int4 expected = int4(-1, 0, 0, 1);
    _out.sk_FragColor = (((((((_skTemp0 = (int(_uniforms.testInputs.x)), select(select(int(0), int(-1), _skTemp0 < 0), int(1), _skTemp0 > 0)) == expected.x && all((_skTemp1 = (int2(_uniforms.testInputs.xy)), select(select(int2(0), int2(-1), _skTemp1 < 0), int2(1), _skTemp1 > 0)) == expected.xy)) && all((_skTemp2 = (int3(_uniforms.testInputs.xyz)), select(select(int3(0), int3(-1), _skTemp2 < 0), int3(1), _skTemp2 > 0)) == expected.xyz)) && all((_skTemp3 = (int4(_uniforms.testInputs)), select(select(int4(0), int4(-1), _skTemp3 < 0), int4(1), _skTemp3 > 0)) == expected)) && -1 == expected.x) && all(int2(-1, 0) == expected.xy)) && all(int3(-1, 0, 0) == expected.xyz)) && all(int4(-1, 0, 0, 1) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
