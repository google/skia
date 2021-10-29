#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x2 left, const float3x2 right);
thread bool operator!=(const float3x2 left, const float3x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
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
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x3 testMatrix2x3 = float2x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0));
    _out.sk_FragColor = (transpose(_uniforms.testMatrix2x2) == float2x2(float2(1.0, 3.0), float2(2.0, 4.0)) && transpose(testMatrix2x3) == float3x2(float2(1.0, 4.0), float2(2.0, 5.0), float2(3.0, 6.0))) && transpose(_uniforms.testMatrix3x3) == float3x3(float3(1.0, 4.0, 7.0), float3(2.0, 5.0, 8.0), float3(3.0, 6.0, 9.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
