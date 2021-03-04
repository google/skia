#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};



fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int4 intValues = int4(_uniforms.testInputs * 100.0);
    int4 intGreen = int4(_uniforms.colorGreen * 100.0);
    _out.sk_FragColor = ((((((min(intValues.x, 50) == -125 && all(min(intValues.xy, 50) == int2(-125, 0))) && all(min(intValues.xyz, 50) == int3(-125, 0, 50))) && all(min(intValues, 50) == int4(-125, 0, 50, 50))) && min(intValues.x, intGreen.x) == -125) && all(min(intValues.xy, intGreen.xy) == int2(-125, 0))) && all(min(intValues.xyz, intGreen.xyz) == int3(-125, 0, 0))) && all(min(intValues, intGreen) == int4(-125, 0, 0, 100)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
