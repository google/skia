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

thread bool operator==(const float4x2 left, const float4x2 right);
thread bool operator!=(const float4x2 left, const float4x2 right);

thread bool operator==(const float2x3 left, const float2x3 right);
thread bool operator!=(const float2x3 left, const float2x3 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);

thread bool operator==(const float3x4 left, const float3x4 right);
thread bool operator!=(const float3x4 left, const float3x4 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
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
thread bool operator==(const float4x2 left, const float4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x2 left, const float4x2 right) {
    return !(left == right);
}
thread bool operator==(const float2x3 left, const float2x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x3 left, const float2x3 right) {
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
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
    return !(left == right);
}
thread bool operator==(const float3x4 left, const float3x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x4 left, const float3x4 right) {
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float2x3 testMatrix2x3 = float2x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0));
    const float2x4 testMatrix2x4 = float2x4(float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0));
    const float3x2 testMatrix3x2 = float3x2(float2(1.0, 2.0), float2(3.0, 4.0), float2(5.0, 6.0));
    const float3x4 testMatrix3x4 = float3x4(float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0), float4(9.0, 10.0, 11.0, 12.0));
    const float4x2 testMatrix4x2 = float4x2(float2(1.0, 2.0), float2(3.0, 4.0), float2(5.0, 6.0), float2(7.0, 8.0));
    const float4x3 testMatrix4x3 = float4x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0), float3(10.0, 11.0, 12.0));
    const float4x4 testMatrix4x4 = float4x4(float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0), float4(9.0, 10.0, 11.0, 12.0), float4(13.0, 14.0, 15.0, 16.0));
    _out.sk_FragColor = (((((((transpose(_uniforms.testMatrix2x2) == float2x2(float2(1.0, 3.0), float2(2.0, 4.0)) && transpose(testMatrix2x3) == float3x2(float2(1.0, 4.0), float2(2.0, 5.0), float2(3.0, 6.0))) && transpose(testMatrix2x4) == float4x2(float2(1.0, 5.0), float2(2.0, 6.0), float2(3.0, 7.0), float2(4.0, 8.0))) && transpose(testMatrix3x2) == float2x3(float3(1.0, 3.0, 5.0), float3(2.0, 4.0, 6.0))) && transpose(_uniforms.testMatrix3x3) == float3x3(float3(1.0, 4.0, 7.0), float3(2.0, 5.0, 8.0), float3(3.0, 6.0, 9.0))) && transpose(testMatrix3x4) == float4x3(float3(1.0, 5.0, 9.0), float3(2.0, 6.0, 10.0), float3(3.0, 7.0, 11.0), float3(4.0, 8.0, 12.0))) && transpose(testMatrix4x2) == float2x4(float4(1.0, 3.0, 5.0, 7.0), float4(2.0, 4.0, 6.0, 8.0))) && transpose(testMatrix4x3) == float3x4(float4(1.0, 4.0, 7.0, 10.0), float4(2.0, 5.0, 8.0, 11.0), float4(3.0, 6.0, 9.0, 12.0))) && transpose(testMatrix4x4) == float4x4(float4(1.0, 5.0, 9.0, 13.0), float4(2.0, 6.0, 10.0, 14.0), float4(3.0, 7.0, 11.0, 15.0), float4(4.0, 8.0, 12.0, 16.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
