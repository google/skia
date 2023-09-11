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

thread bool operator==(const half3x2 left, const half3x2 right);
thread bool operator!=(const half3x2 left, const half3x2 right);

thread bool operator==(const half2x3 left, const half2x3 right);
thread bool operator!=(const half2x3 left, const half2x3 right);

thread bool operator==(const half4x3 left, const half4x3 right);
thread bool operator!=(const half4x3 left, const half4x3 right);

thread bool operator==(const half4x2 left, const half4x2 right);
thread bool operator!=(const half4x2 left, const half4x2 right);

thread bool operator==(const half2x4 left, const half2x4 right);
thread bool operator!=(const half2x4 left, const half2x4 right);

thread bool operator==(const float3x2 left, const float3x2 right);
thread bool operator!=(const float3x2 left, const float3x2 right);

thread bool operator==(const float2x3 left, const float2x3 right);
thread bool operator!=(const float2x3 left, const float2x3 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

thread bool operator==(const float4x2 left, const float4x2 right);
thread bool operator!=(const float4x2 left, const float4x2 right);

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);
thread bool operator==(const half3x2 left, const half3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x2 left, const half3x2 right) {
    return !(left == right);
}
thread half3x2 operator/(const half3x2 left, const half3x2 right) {
    return half3x2(left[0] / right[0], left[1] / right[1], left[2] / right[2]);
}
thread half3x2& operator/=(thread half3x2& left, thread const half3x2& right) {
    left = left / right;
    return left;
}
thread bool operator==(const half2x3 left, const half2x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x3 left, const half2x3 right) {
    return !(left == right);
}
thread half2x3 operator/(const half2x3 left, const half2x3 right) {
    return half2x3(left[0] / right[0], left[1] / right[1]);
}
thread half2x3& operator/=(thread half2x3& left, thread const half2x3& right) {
    left = left / right;
    return left;
}
thread bool operator==(const half4x3 left, const half4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x3 left, const half4x3 right) {
    return !(left == right);
}
thread bool operator==(const half4x2 left, const half4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x2 left, const half4x2 right) {
    return !(left == right);
}
thread half2x4 operator/(const half2x4 left, const half2x4 right) {
    return half2x4(left[0] / right[0], left[1] / right[1]);
}
thread half2x4& operator/=(thread half2x4& left, thread const half2x4& right) {
    left = left / right;
    return left;
}
thread bool operator==(const half2x4 left, const half2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x4 left, const half2x4 right) {
    return !(left == right);
}
thread half2x3& operator*=(thread half2x3& left, thread const half2x2& right) {
    left = left * right;
    return left;
}
thread bool operator==(const float3x2 left, const float3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x2 left, const float3x2 right) {
    return !(left == right);
}
thread float3x2 operator/(const float3x2 left, const float3x2 right) {
    return float3x2(left[0] / right[0], left[1] / right[1], left[2] / right[2]);
}
thread float3x2& operator/=(thread float3x2& left, thread const float3x2& right) {
    left = left / right;
    return left;
}
thread bool operator==(const float2x3 left, const float2x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x3 left, const float2x3 right) {
    return !(left == right);
}
thread float2x3 operator/(const float2x3 left, const float2x3 right) {
    return float2x3(left[0] / right[0], left[1] / right[1]);
}
thread float2x3& operator/=(thread float2x3& left, thread const float2x3& right) {
    left = left / right;
    return left;
}
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
thread bool operator==(const float4x2 left, const float4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x2 left, const float4x2 right) {
    return !(left == right);
}
thread float2x4 operator/(const float2x4 left, const float2x4 right) {
    return float2x4(left[0] / right[0], left[1] / right[1]);
}
thread float2x4& operator/=(thread float2x4& left, thread const float2x4& right) {
    left = left / right;
    return left;
}
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
    return !(left == right);
}
thread float2x3& operator*=(thread float2x3& left, thread const float2x2& right) {
    left = left * right;
    return left;
}
bool test_matrix_op_matrix_half_b() {
    bool ok = true;
    {
        const half3x2 splat_4 = half3x2(half2(4.0h, 4.0h), half2(4.0h, 4.0h), half2(4.0h, 4.0h));
        half3x2 m = half3x2(2.0h);
        m += splat_4;
        ok = ok && m == half3x2(half2(6.0h, 4.0h), half2(4.0h, 6.0h), half2(4.0h, 4.0h));
        m = half3x2(2.0h);
        m -= splat_4;
        ok = ok && m == half3x2(half2(-2.0h, -4.0h), half2(-4.0h, -2.0h), half2(-4.0h, -4.0h));
        m = half3x2(2.0h);
        m /= splat_4;
        ok = ok && m == half3x2(0.5h);
    }
    {
        const half2x3 splat_4 = half2x3(half3(4.0h, 4.0h, 4.0h), half3(4.0h, 4.0h, 4.0h));
        half2x3 m = splat_4;
        m += half2x3(2.0h);
        ok = ok && m == half2x3(half3(6.0h, 4.0h, 4.0h), half3(4.0h, 6.0h, 4.0h));
        m = splat_4;
        m -= half2x3(2.0h);
        ok = ok && m == half2x3(half3(2.0h, 4.0h, 4.0h), half3(4.0h, 2.0h, 4.0h));
        m = splat_4;
        m /= half2x3(half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h));
        ok = ok && m == half2x3(half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h));
    }
    {
        half4x3 m = half4x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h), half3(10.0h, 11.0h, 12.0h));
        m += half4x3(half3(16.0h, 15.0h, 14.0h), half3(13.0h, 12.0h, 11.0h), half3(10.0h, 9.0h, 8.0h), half3(7.0h, 6.0h, 5.0h));
        ok = ok && m == half4x3(half3(17.0h, 17.0h, 17.0h), half3(17.0h, 17.0h, 17.0h), half3(17.0h, 17.0h, 17.0h), half3(17.0h, 17.0h, 17.0h));
    }
    {
        half4x2 m = half4x2(half2(10.0h, 20.0h), half2(30.0h, 40.0h), half2(50.0h, 60.0h), half2(70.0h, 80.0h));
        m -= half4x2(half2(1.0h, 2.0h), half2(3.0h, 4.0h), half2(5.0h, 6.0h), half2(7.0h, 8.0h));
        ok = ok && m == half4x2(half2(9.0h, 18.0h), half2(27.0h, 36.0h), half2(45.0h, 54.0h), half2(63.0h, 72.0h));
    }
    {
        half2x4 m = half2x4(half4(10.0h, 20.0h, 30.0h, 40.0h), half4(10.0h, 20.0h, 30.0h, 40.0h));
        m /= half2x4(half4(10.0h, 10.0h, 10.0h, 10.0h), half4(5.0h, 5.0h, 5.0h, 5.0h));
        ok = ok && m == half2x4(half4(1.0h, 2.0h, 3.0h, 4.0h), half4(2.0h, 4.0h, 6.0h, 8.0h));
    }
    {
        half2x3 m = half2x3(half3(7.0h, 9.0h, 11.0h), half3(8.0h, 10.0h, 12.0h));
        m *= half2x2(half2(1.0h, 4.0h), half2(2.0h, 5.0h));
        ok = ok && m == half2x3(half3(39.0h, 49.0h, 59.0h), half3(54.0h, 68.0h, 82.0h));
    }
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    {
        const float3x2 _1_splat_4 = float3x2(float2(4.0, 4.0), float2(4.0, 4.0), float2(4.0, 4.0));
        float3x2 _2_m = float3x2(2.0);
        _2_m += _1_splat_4;
        _0_ok = _0_ok && _2_m == float3x2(float2(6.0, 4.0), float2(4.0, 6.0), float2(4.0, 4.0));
        _2_m = float3x2(2.0);
        _2_m -= _1_splat_4;
        _0_ok = _0_ok && _2_m == float3x2(float2(-2.0, -4.0), float2(-4.0, -2.0), float2(-4.0, -4.0));
        _2_m = float3x2(2.0);
        _2_m /= _1_splat_4;
        _0_ok = _0_ok && _2_m == float3x2(0.5);
    }
    {
        const float2x3 _3_splat_4 = float2x3(float3(4.0, 4.0, 4.0), float3(4.0, 4.0, 4.0));
        float2x3 _4_m = _3_splat_4;
        _4_m += float2x3(2.0);
        _0_ok = _0_ok && _4_m == float2x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0));
        _4_m = _3_splat_4;
        _4_m -= float2x3(2.0);
        _0_ok = _0_ok && _4_m == float2x3(float3(2.0, 4.0, 4.0), float3(4.0, 2.0, 4.0));
        _4_m = _3_splat_4;
        _4_m /= float2x3(float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0));
        _0_ok = _0_ok && _4_m == float2x3(float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0));
    }
    {
        float4x3 _5_m = float4x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0), float3(10.0, 11.0, 12.0));
        _5_m += float4x3(float3(16.0, 15.0, 14.0), float3(13.0, 12.0, 11.0), float3(10.0, 9.0, 8.0), float3(7.0, 6.0, 5.0));
        _0_ok = _0_ok && _5_m == float4x3(float3(17.0, 17.0, 17.0), float3(17.0, 17.0, 17.0), float3(17.0, 17.0, 17.0), float3(17.0, 17.0, 17.0));
    }
    {
        float4x2 _6_m = float4x2(float2(10.0, 20.0), float2(30.0, 40.0), float2(50.0, 60.0), float2(70.0, 80.0));
        _6_m -= float4x2(float2(1.0, 2.0), float2(3.0, 4.0), float2(5.0, 6.0), float2(7.0, 8.0));
        _0_ok = _0_ok && _6_m == float4x2(float2(9.0, 18.0), float2(27.0, 36.0), float2(45.0, 54.0), float2(63.0, 72.0));
    }
    {
        float2x4 _7_m = float2x4(float4(10.0, 20.0, 30.0, 40.0), float4(10.0, 20.0, 30.0, 40.0));
        _7_m /= float2x4(float4(10.0, 10.0, 10.0, 10.0), float4(5.0, 5.0, 5.0, 5.0));
        _0_ok = _0_ok && _7_m == float2x4(float4(1.0, 2.0, 3.0, 4.0), float4(2.0, 4.0, 6.0, 8.0));
    }
    {
        float2x3 _8_m = float2x3(float3(7.0, 9.0, 11.0), float3(8.0, 10.0, 12.0));
        _8_m *= float2x2(float2(1.0, 4.0), float2(2.0, 5.0));
        _0_ok = _0_ok && _8_m == float2x3(float3(39.0, 49.0, 59.0), float3(54.0, 68.0, 82.0));
    }
    _out.sk_FragColor = _0_ok && test_matrix_op_matrix_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
