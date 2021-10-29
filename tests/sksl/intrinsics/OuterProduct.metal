#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float3x2 left, const float3x2 right);
thread bool operator!=(const float3x2 left, const float3x2 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);

thread bool operator==(const float4x2 left, const float4x2 right);
thread bool operator!=(const float4x2 left, const float4x2 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}

template <typename T, int C, int R>
matrix<T, C, R> outerProduct(const vec<T, R> a, const vec<T, C> b) {
    matrix<T, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a * b[c];
    }
    return result;
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
thread bool operator==(const float3x2 left, const float3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x2 left, const float3x2 right) {
    return !(left == right);
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
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float2 c12 = float2(1.0, 2.0);
    _out.sk_FragColor = ((((outerProduct(_uniforms.testMatrix2x2[0], _uniforms.testMatrix2x2[1]) == float2x2(float2(3.0, 6.0), float2(4.0, 8.0)) && outerProduct(_uniforms.testMatrix3x3[0], _uniforms.testMatrix3x3[1]) == float3x3(float3(4.0, 8.0, 12.0), float3(5.0, 10.0, 15.0), float3(6.0, 12.0, 18.0))) && outerProduct(_uniforms.testMatrix2x2[0], _uniforms.testMatrix3x3[1]) == float3x2(float2(4.0, 8.0), float2(5.0, 10.0), float2(6.0, 12.0))) && outerProduct(_uniforms.testInputs, float4(1.0, 0.0, 0.0, 2.0)) == float4x4(float4(-1.25, 0.0, 0.75, 2.25), float4(0.0, 0.0, 0.0, 0.0), float4(0.0, 0.0, 0.0, 0.0), float4(-2.5, 0.0, 1.5, 4.5))) && outerProduct(_uniforms.testInputs, c12) == float2x4(float4(-1.25, 0.0, 0.75, 2.25), float4(-2.5, 0.0, 1.5, 4.5))) && outerProduct(c12, _uniforms.testInputs) == float4x2(float2(-1.25, -2.5), float2(0.0, 0.0), float2(0.75, 1.5), float2(2.25, 4.5)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
