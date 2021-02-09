#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = _uniforms.unknownInput;
    float4 v = float4(float2(x), float(0), float(1));
    v = float4(float2(_uniforms.unknownInput), float(0), float(1));
    v = float3(_uniforms.unknownInput, float(0), float(1)).yxzy;
    v = float3(float2(_uniforms.unknownInput), float(0)).zxzy;
    _out.sk_FragColor = v;
    return _out;
}
