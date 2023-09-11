#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half2x2 testMatrix2x2;
    half3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(const half3x3 left, const half3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x3 left, const half3x3 right) {
    return !(left == right);
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
float3x3 float3x3_from_float2x2(float2x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(0.0, 0.0, 1.0));
}
float2x2 float2x2_from_float3x3(float3x3 x0) {
    return float2x2(float2(x0[0].xy), float2(x0[1].xy));
}

half4 half4_from_half2x2(half2x2 x) {
    return half4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    _0_ok = _0_ok && _uniforms.testMatrix2x2 == half2x2(half2(1.0h, 2.0h), half2(3.0h, 4.0h));
    _0_ok = _0_ok && _uniforms.testMatrix3x3 == half3x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h));
    _0_ok = _0_ok && _uniforms.testMatrix2x2 != half2x2(100.0h);
    _0_ok = _0_ok && _uniforms.testMatrix3x3 != half3x3(half3(9.0h, 8.0h, 7.0h), half3(6.0h, 5.0h, 4.0h), half3(3.0h, 2.0h, 1.0h));
    float _1_zero = float(_uniforms.colorGreen.x);
    float _2_one = float(_uniforms.colorGreen.y);
    float _3_two = 2.0 * _2_one;
    float _4_nine = 9.0 * _2_one;
    _0_ok = _0_ok && float2x2(float2(_2_one, _1_zero), float2(_1_zero, _2_one)) == float2x2(float2(1.0, 0.0), float2(0.0, 1.0));
    _0_ok = _0_ok && float2x2(float2(_2_one, _1_zero), float2(_2_one)) != float2x2(float2(1.0, 0.0), float2(0.0, 1.0));
    _0_ok = _0_ok && float2x2(_2_one) == float2x2(1.0);
    _0_ok = _0_ok && float2x2(_2_one) != float2x2(0.0);
    _0_ok = _0_ok && float2x2(-_2_one) == float2x2(-1.0);
    _0_ok = _0_ok && float2x2(_1_zero) == float2x2(-0.0);
    _0_ok = _0_ok && (-1.0 * float2x2(-_2_one)) == float2x2(1.0);
    _0_ok = _0_ok && (-1.0 * float2x2(_1_zero)) == float2x2(-0.0);
    _0_ok = _0_ok && float2x2(_2_one) == float2x2(float2(1.0, 0.0), float2(0.0, 1.0));
    _0_ok = _0_ok && float2x2(_3_two) != float2x2(float2(1.0, 0.0), float2(0.0, 1.0));
    _0_ok = _0_ok && float2x2(_2_one) == float2x2(1.0);
    _0_ok = _0_ok && float2x2(_2_one) != float2x2(0.0);
    _0_ok = _0_ok && float3x3(float3(_2_one, _1_zero, _1_zero), float3(_1_zero, _2_one, _1_zero), float3(_1_zero, _1_zero, _2_one)) == float3x3_from_float2x2(float2x2(1.0));
    _0_ok = _0_ok && float3x3(float3(_4_nine, _1_zero, _1_zero), float3(_1_zero, _4_nine, _1_zero), float3(_1_zero, _1_zero, _2_one)) == float3x3_from_float2x2(float2x2(9.0));
    _0_ok = _0_ok && float3x3(_2_one) == float3x3_from_float2x2(float2x2(1.0));
    _0_ok = _0_ok && float3x3(float3(_4_nine, 0.0, 0.0), float3(0.0, _4_nine, 0.0), float3(0.0, 0.0, _2_one)) == float3x3_from_float2x2(float2x2(9.0));
    _0_ok = _0_ok && float2x2_from_float3x3(float3x3(_2_one)) == float2x2(1.0);
    _0_ok = _0_ok && float2x2_from_float3x3(float3x3(_2_one)) == float2x2(1.0);
    _0_ok = _0_ok && float2x2(float2(_2_one, _1_zero), float2(_1_zero, _2_one)) == float2x2(1.0);
    _0_ok = _0_ok && float2x2(float2(_2_one, _1_zero), float2(_1_zero, _2_one)) == float2x2(1.0);
    _0_ok = _0_ok && float2x2(float2(_2_one, _1_zero), float2(_1_zero, _2_one)) == float2x2(1.0);
    _0_ok = _0_ok && all(float4(half4_from_half2x2(_uniforms.testMatrix2x2)) * float4(_2_one) == float4(1.0, 2.0, 3.0, 4.0));
    _0_ok = _0_ok && all(float4(half4_from_half2x2(_uniforms.testMatrix2x2)) * float4(_2_one) == float4(half4_from_half2x2(_uniforms.testMatrix2x2)));
    _0_ok = _0_ok && all(float4(half4_from_half2x2(_uniforms.testMatrix2x2)) * float4(_1_zero) == float4(0.0));
    float3x3 _5_m = float3x3(float3(_2_one, _3_two, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, _4_nine));
    _0_ok = _0_ok && all(_5_m[0] == float3(1.0, 2.0, 3.0));
    _0_ok = _0_ok && all(_5_m[1] == float3(4.0, 5.0, 6.0));
    _0_ok = _0_ok && all(_5_m[2] == float3(7.0, 8.0, 9.0));
    _0_ok = _0_ok && _5_m[0].x == 1.0;
    _0_ok = _0_ok && _5_m[0].y == 2.0;
    _0_ok = _0_ok && _5_m[0].z == 3.0;
    _0_ok = _0_ok && _5_m[1].x == 4.0;
    _0_ok = _0_ok && _5_m[1].y == 5.0;
    _0_ok = _0_ok && _5_m[1].z == 6.0;
    _0_ok = _0_ok && _5_m[2].x == 7.0;
    _0_ok = _0_ok && _5_m[2].y == 8.0;
    _0_ok = _0_ok && _5_m[2].z == 9.0;
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
