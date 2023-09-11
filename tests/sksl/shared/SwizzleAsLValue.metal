#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 scalar;
    array<float4, 1> array;
    scalar = float4(_uniforms.colorGreen) * 0.5;
    scalar.w = 2.0;
    scalar.y = scalar.y * 4.0;
    scalar.yzw = scalar.yzw * float3x3(0.5);
    scalar.zywx = scalar.zywx + float4(0.25, 0.0, 0.0, 0.75);
    scalar.x = scalar.x + (scalar.w <= 1.0 ? scalar.z : 0.0);
    array[0] = float4(_uniforms.colorGreen) * 0.5;
    array[0].w = 2.0;
    array[0].y = array[0].y * 4.0;
    array[0].yzw = array[0].yzw * float3x3(0.5);
    array[0].zywx = array[0].zywx + float4(0.25, 0.0, 0.0, 0.75);
    array[0].x = array[0].x + (array[0].w <= 1.0 ? array[0].z : 0.0);
    _out.sk_FragColor = all(scalar == float4(1.0, 1.0, 0.25, 1.0)) && all(array[0] == float4(1.0, 1.0, 0.25, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
