#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
template <typename T, int C, int R>
thread bool operator==(const matrix<T, C, R> left, const matrix<T, C, R> right) {
    for (size_t index = 0; index < C; ++index) {
        if (any(left[index] != right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T, int C, int R>
thread bool operator!=(const matrix<T, C, R> left, const matrix<T, C, R> right) {
    return !(left == right);
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    _0_ok = _0_ok && _uniforms.testMatrix2x2 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _0_ok = _0_ok && _uniforms.testMatrix3x3 == float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    _0_ok = _0_ok && _uniforms.testMatrix2x2 != float2x2(100.0);
    _0_ok = _0_ok && _uniforms.testMatrix3x3 != float3x3(float3(9.0, 8.0, 7.0), float3(6.0, 5.0, 4.0), float3(3.0, 2.0, 1.0));
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
