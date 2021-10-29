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

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
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
bool test_half_b() {
    bool ok = true;
    ok = ok && float3x3(2.0) + (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
    ok = ok && float3x3(2.0) - (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(float3(-2.0, -4.0, -4.0), float3(-4.0, -2.0, -4.0), float3(-4.0, -4.0, -2.0));
    ok = ok && float3x3(2.0) * 4.0 == float3x3(8.0);
    ok = ok && float3x3(2.0) / (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(0.5);
    ok = ok && (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) + float3x3(2.0) == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
    ok = ok && (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) - float3x3(2.0) == float3x3(float3(2.0, 4.0, 4.0), float3(4.0, 2.0, 4.0), float3(4.0, 4.0, 2.0));
    ok = ok && 4.0 * float3x3(2.0) == float3x3(8.0);
    ok = ok && (float2x2(1.0, 1.0, 1.0, 1.0) * 4.0) / float2x2(float2(2.0, 2.0), float2(2.0, 2.0)) == float2x2(float2(2.0, 2.0), float2(2.0, 2.0));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    _0_ok = _0_ok && float3x3(2.0) + (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
    _0_ok = _0_ok && float3x3(2.0) - (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(float3(-2.0, -4.0, -4.0), float3(-4.0, -2.0, -4.0), float3(-4.0, -4.0, -2.0));
    _0_ok = _0_ok && float3x3(2.0) * 4.0 == float3x3(8.0);
    _0_ok = _0_ok && float3x3(2.0) / (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) == float3x3(0.5);
    _0_ok = _0_ok && (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) + float3x3(2.0) == float3x3(float3(6.0, 4.0, 4.0), float3(4.0, 6.0, 4.0), float3(4.0, 4.0, 6.0));
    _0_ok = _0_ok && (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 4.0) - float3x3(2.0) == float3x3(float3(2.0, 4.0, 4.0), float3(4.0, 2.0, 4.0), float3(4.0, 4.0, 2.0));
    _0_ok = _0_ok && 4.0 * float3x3(2.0) == float3x3(8.0);
    _0_ok = _0_ok && (float2x2(1.0, 1.0, 1.0, 1.0) * 4.0) / float2x2(float2(2.0, 2.0), float2(2.0, 2.0)) == float2x2(float2(2.0, 2.0), float2(2.0, 2.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
