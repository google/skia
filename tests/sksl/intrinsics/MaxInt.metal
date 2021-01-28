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
    _out.sk_FragColor = ((((((max(intValues.x, 50) == 50 && all(max(intValues.xy, 50) == int2(50, 50))) && all(max(intValues.xyz, 50) == int3(50, 50, 75))) && all(max(intValues, 50) == int4(50, 50, 75, 225))) && max(intValues.x, intGreen.x) == 0) && all(max(intValues.xy, intGreen.xy) == int2(0, 100))) && all(max(intValues.xyz, intGreen.xyz) == int3(0, 100, 75))) && all(max(intValues, intGreen) == int4(0, 100, 75, 225)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
