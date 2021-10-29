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
    int4 expectedA = int4(50, 50, 75, 225);
    int4 expectedB = int4(0, 100, 75, 225);
    _out.sk_FragColor = ((((((((((((((max(intValues.x, 50) == expectedA.x && all(max(intValues.xy, 50) == expectedA.xy)) && all(max(intValues.xyz, 50) == expectedA.xyz)) && all(max(intValues, 50) == expectedA)) && 50 == expectedA.x) && all(int2(50, 50) == expectedA.xy)) && all(int3(50, 50, 75) == expectedA.xyz)) && all(int4(50, 50, 75, 225) == expectedA)) && max(intValues.x, intGreen.x) == expectedB.x) && all(max(intValues.xy, intGreen.xy) == expectedB.xy)) && all(max(intValues.xyz, intGreen.xyz) == expectedB.xyz)) && all(max(intValues, intGreen) == expectedB)) && 0 == expectedB.x) && all(int2(0, 100) == expectedB.xy)) && all(int3(0, 100, 75) == expectedB.xyz)) && all(int4(0, 100, 75, 225) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
