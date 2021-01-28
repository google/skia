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
    _out.sk_FragColor = ((abs(int(_uniforms.testInputs.x)) == 1 && all(abs(int2(_uniforms.testInputs.xy)) == int2(1, 0))) && all(abs(int3(_uniforms.testInputs.xyz)) == int3(1, 0, 0))) && all(abs(int4(_uniforms.testInputs)) == int4(1, 0, 0, 2)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
