#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 result;
    result.x = (sqrt(1.0) , _uniforms.colorGreen.x);
    result.y = (float2(2.0) , _uniforms.colorGreen.y);
    result.z = (float3(3.0) , _uniforms.colorGreen.z);
    result.w = (float2x2(4.0) , _uniforms.colorGreen.w);
    _out.sk_FragColor = result;
    return _out;
}
