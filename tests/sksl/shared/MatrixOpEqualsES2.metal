#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

thread bool operator==(const half4x4 left, const half4x4 right);
thread bool operator!=(const half4x4 left, const half4x4 right);

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
thread bool operator==(const half3x3 left, const half3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x3 left, const half3x3 right) {
    return !(left == right);
}
thread half3x3 operator/(const half3x3 left, const half3x3 right) {
    return half3x3(left[0] / right[0], left[1] / right[1], left[2] / right[2]);
}
thread half3x3& operator/=(thread half3x3& left, thread const half3x3& right) {
    left = left / right;
    return left;
}
thread bool operator==(const half4x4 left, const half4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x4 left, const half4x4 right) {
    return !(left == right);
}
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread half2x2 operator/(const half2x2 left, const half2x2 right) {
    return half2x2(left[0] / right[0], left[1] / right[1]);
}
thread half2x2& operator/=(thread half2x2& left, thread const half2x2& right) {
    left = left / right;
    return left;
}
thread half2x2& operator*=(thread half2x2& left, thread const half2x2& right) {
    left = left * right;
    return left;
}
thread half3x3& operator*=(thread half3x3& left, thread const half3x3& right) {
    left = left * right;
    return left;
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
thread float3x3 operator/(const float3x3 left, const float3x3 right) {
    return float3x3(left[0] / right[0], left[1] / right[1], left[2] / right[2]);
}
thread float3x3& operator/=(thread float3x3& left, thread const float3x3& right) {
    left = left / right;
    return left;
}
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread float2x2 operator/(const float2x2 left, const float2x2 right) {
    return float2x2(left[0] / right[0], left[1] / right[1]);
}
thread float2x2& operator/=(thread float2x2& left, thread const float2x2& right) {
    left = left / right;
    return left;
}
thread float2x2& operator*=(thread float2x2& left, thread const float2x2& right) {
    left = left * right;
    return left;
}
thread float3x3& operator*=(thread float3x3& left, thread const float3x3& right) {
    left = left * right;
    return left;
}
bool test_matrix_op_matrix_half_b() {
    bool ok = true;
    {
        const half3x3 splat_4 = half3x3(half3(4.0h, 4.0h, 4.0h), half3(4.0h, 4.0h, 4.0h), half3(4.0h, 4.0h, 4.0h));
        const half3x3 splat_2 = half3x3(half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h));
        half3x3 m = half3x3(2.0h);
        m += splat_4;
        ok = ok && m == half3x3(half3(6.0h, 4.0h, 4.0h), half3(4.0h, 6.0h, 4.0h), half3(4.0h, 4.0h, 6.0h));
        m = half3x3(2.0h);
        m -= splat_4;
        ok = ok && m == half3x3(half3(-2.0h, -4.0h, -4.0h), half3(-4.0h, -2.0h, -4.0h), half3(-4.0h, -4.0h, -2.0h));
        m = half3x3(2.0h);
        m /= splat_4;
        ok = ok && m == half3x3(0.5h);
        m = splat_4;
        m += half3x3(2.0h);
        ok = ok && m == half3x3(half3(6.0h, 4.0h, 4.0h), half3(4.0h, 6.0h, 4.0h), half3(4.0h, 4.0h, 6.0h));
        m = splat_4;
        m -= half3x3(2.0h);
        ok = ok && m == half3x3(half3(2.0h, 4.0h, 4.0h), half3(4.0h, 2.0h, 4.0h), half3(4.0h, 4.0h, 2.0h));
        m = splat_4;
        m /= splat_2;
        ok = ok && m == half3x3(half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h));
    }
    {
        half4x4 m = half4x4(half4(1.0h, 2.0h, 3.0h, 4.0h), half4(5.0h, 6.0h, 7.0h, 8.0h), half4(9.0h, 10.0h, 11.0h, 12.0h), half4(13.0h, 14.0h, 15.0h, 16.0h));
        m += half4x4(half4(16.0h, 15.0h, 14.0h, 13.0h), half4(12.0h, 11.0h, 10.0h, 9.0h), half4(8.0h, 7.0h, 6.0h, 5.0h), half4(4.0h, 3.0h, 2.0h, 1.0h));
        ok = ok && m == half4x4(half4(17.0h, 17.0h, 17.0h, 17.0h), half4(17.0h, 17.0h, 17.0h, 17.0h), half4(17.0h, 17.0h, 17.0h, 17.0h), half4(17.0h, 17.0h, 17.0h, 17.0h));
    }
    {
        half2x2 m = half2x2(half2(10.0h, 20.0h), half2(30.0h, 40.0h));
        m -= half2x2(half2(1.0h, 2.0h), half2(3.0h, 4.0h));
        ok = ok && m == half2x2(half2(9.0h, 18.0h), half2(27.0h, 36.0h));
    }
    {
        half2x2 m = half2x2(half2(2.0h, 4.0h), half2(6.0h, 8.0h));
        m /= half2x2(half2(2.0h, 2.0h), half2(2.0h, 4.0h));
        ok = ok && m == half2x2(half2(1.0h, 2.0h), half2(3.0h, 2.0h));
    }
    {
        half2x2 m = half2x2(half2(1.0h, 2.0h), half2(7.0h, 4.0h));
        m *= half2x2(half2(3.0h, 5.0h), half2(3.0h, 2.0h));
        ok = ok && m == half2x2(half2(38.0h, 26.0h), half2(17.0h, 14.0h));
    }
    {
        half3x3 m = half3x3(half3(10.0h, 4.0h, 2.0h), half3(20.0h, 5.0h, 3.0h), half3(10.0h, 6.0h, 5.0h));
        m *= half3x3(half3(3.0h, 3.0h, 4.0h), half3(2.0h, 3.0h, 4.0h), half3(4.0h, 9.0h, 2.0h));
        ok = ok && m == half3x3(half3(130.0h, 51.0h, 35.0h), half3(120.0h, 47.0h, 33.0h), half3(240.0h, 73.0h, 45.0h));
    }
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    {
        const float3x3 _1_splat_4 = float3x3(float3(4.0, 4.0, 4.0), float3(4.0, 4.0, 4.0), float3(4.0, 4.0, 4.0));
        const float3x3 _2_splat_2 = float3x3(float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0));
        float3x3 _3_m = float3x3(2.0);
        _3_m += _1_splat_4;
        _0_ok = _0_ok && _3_m == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
        _3_m = float3x3(2.0);
        _3_m -= _1_splat_4;
        _0_ok = _0_ok && _3_m == float3x3(float3(-2.0, -4.0, -4.0), float3(-4.0, -2.0, -4.0), float3(-4.0, -4.0, -2.0));
        _3_m = float3x3(2.0);
        _3_m /= _1_splat_4;
        _0_ok = _0_ok && _3_m == float3x3(0.5);
        _3_m = _1_splat_4;
        _3_m += float3x3(2.0);
        _0_ok = _0_ok && _3_m == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
        _3_m = _1_splat_4;
        _3_m -= float3x3(2.0);
        _0_ok = _0_ok && _3_m == float3x3(float3(2.0, 4.0, 4.0), float3(4.0, 2.0, 4.0), float3(4.0, 4.0, 2.0));
        _3_m = _1_splat_4;
        _3_m /= _2_splat_2;
        _0_ok = _0_ok && _3_m == float3x3(float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0));
    }
    {
        float4x4 _4_m = float4x4(float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0), float4(9.0, 10.0, 11.0, 12.0), float4(13.0, 14.0, 15.0, 16.0));
        _4_m += float4x4(float4(16.0, 15.0, 14.0, 13.0), float4(12.0, 11.0, 10.0, 9.0), float4(8.0, 7.0, 6.0, 5.0), float4(4.0, 3.0, 2.0, 1.0));
        _0_ok = _0_ok && _4_m == float4x4(float4(17.0, 17.0, 17.0, 17.0), float4(17.0, 17.0, 17.0, 17.0), float4(17.0, 17.0, 17.0, 17.0), float4(17.0, 17.0, 17.0, 17.0));
    }
    {
        float2x2 _5_m = float2x2(float2(10.0, 20.0), float2(30.0, 40.0));
        _5_m -= float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
        _0_ok = _0_ok && _5_m == float2x2(float2(9.0, 18.0), float2(27.0, 36.0));
    }
    {
        float2x2 _6_m = float2x2(float2(2.0, 4.0), float2(6.0, 8.0));
        _6_m /= float2x2(float2(2.0, 2.0), float2(2.0, 4.0));
        _0_ok = _0_ok && _6_m == float2x2(float2(1.0, 2.0), float2(3.0, 2.0));
    }
    {
        float2x2 _7_m = float2x2(float2(1.0, 2.0), float2(7.0, 4.0));
        _7_m *= float2x2(float2(3.0, 5.0), float2(3.0, 2.0));
        _0_ok = _0_ok && _7_m == float2x2(float2(38.0, 26.0), float2(17.0, 14.0));
    }
    {
        float3x3 _8_m = float3x3(float3(10.0, 4.0, 2.0), float3(20.0, 5.0, 3.0), float3(10.0, 6.0, 5.0));
        _8_m *= float3x3(float3(3.0, 3.0, 4.0), float3(2.0, 3.0, 4.0), float3(4.0, 9.0, 2.0));
        _0_ok = _0_ok && _8_m == float3x3(float3(130.0, 51.0, 35.0), float3(120.0, 47.0, 33.0), float3(240.0, 73.0, 45.0));
    }
    _out.sk_FragColor = _0_ok && test_matrix_op_matrix_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
