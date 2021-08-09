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
struct Globals {
    float3x3 a;
    float3x3 b;
    float4x4 c;
    float4x4 d;
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

template <int C, int R>
matrix<float, C, R> matrixCompMult(matrix<float, C, R> a, matrix<float, C, R> b) {
    matrix<float, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a[c] * b[c];
    }
    return result;
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
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
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float2x2 h22 = float2x2(float2(0.0, 5.0), float2(10.0, 15.0));
    float4x4 h44 = float4x4(float4(0.5, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0), float4(0.0, 0.0, 5.5, 0.0), float4(0.0, 0.0, 0.0, 8.0));
    float4x3 f43 = float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0));
    _out.sk_FragColor.xyz = matrixCompMult(_globals.a, _globals.b)[0];
    _out.sk_FragColor = matrixCompMult(_globals.c, _globals.d)[0];
    _out.sk_FragColor = (h22 == float2x2(float2(0.0, 5.0), float2(10.0, 15.0)) && h44 == float4x4(float4(0.5, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0), float4(0.0, 0.0, 5.5, 0.0), float4(0.0, 0.0, 0.0, 8.0))) && f43 == float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
