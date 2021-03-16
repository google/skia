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
    int4 expectedA = int4(-125, 0, 50, 50);
    int4 expectedB = int4(-125, 0, 0, 100);
    _out.sk_FragColor = ((((((min(intValues.x, 50) == expectedA.x && all(min(intValues.xy, 50) == expectedA.xy)) && all(min(intValues.xyz, 50) == expectedA.xyz)) && all(min(intValues, 50) == expectedA)) && min(intValues.x, intGreen.x) == expectedB.x) && all(min(intValues.xy, intGreen.xy) == expectedB.xy)) && all(min(intValues.xyz, intGreen.xyz) == expectedB.xyz)) && all(min(intValues, intGreen) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
