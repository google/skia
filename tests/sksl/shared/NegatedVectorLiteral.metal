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


bool test_int() {
    int one = 1;
    int two = 2;
    int4 result;
    result.x = int(all(int4(-1) == -int4(-int2(-1), int2(1))) ? 1 : 0);
    result.y = int(any(int4(1) != -int4(1)) ? 1 : 0);
    result.z = int(all(-int4(two) == int4(-two, int3(-two))) ? 1 : 0);
    result.w = int(all(-int2(-one, one + one) == -int2(one - two, two)) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_test_float;
    float _1_one = 1.0;
    float _2_two = 2.0;
    float4 _3_result;
    _3_result.x = float(all(float4(-1.0) == -float4(-float2(-1.0), float2(1.0))) ? 1 : 0);
    _3_result.y = float(any(float4(1.0) != -float4(1.0)) ? 1 : 0);
    _3_result.z = float(all(-float4(_2_two) == float4(-_2_two, float3(-_2_two))) ? 1 : 0);
    _3_result.w = float(all(-float2(-_1_one, _1_one + _1_one) == -float2(_1_one - _2_two, _2_two)) ? 1 : 0);
    _out.sk_FragColor = bool(((_3_result.x * _3_result.y) * _3_result.z) * _3_result.w) && test_int() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;

}
