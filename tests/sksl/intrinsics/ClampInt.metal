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
    _out.sk_FragColor = ((((((clamp(intValues.x, -100, 100) == int4(-100, 0, 75, 100).x && all(clamp(intValues.xy, -100, 100) == int4(-100, 0, 75, 100).xy)) && all(clamp(intValues.xyz, -100, 100) == int4(-100, 0, 75, 100).xyz)) && all(clamp(intValues, -100, 100) == int4(-100, 0, 75, 100))) && clamp(intValues.x, int4(-100, -200, -200, 100).x, int4(100, 200, 50, 300).x) == int4(-100, 0, 50, 225).x) && all(clamp(intValues.xy, int4(-100, -200, -200, 100).xy, int4(100, 200, 50, 300).xy) == int4(-100, 0, 50, 225).xy)) && all(clamp(intValues.xyz, int4(-100, -200, -200, 100).xyz, int4(100, 200, 50, 300).xyz) == int4(-100, 0, 50, 225).xyz)) && all(clamp(intValues, int4(-100, -200, -200, 100), int4(100, 200, 50, 300)) == int4(-100, 0, 50, 225)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
