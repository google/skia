#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    a = 1;
    b = 2;
    c = 5;
    _out.sk_FragColor = ((a == 1 && b == 2) && c == 5) && d == 0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
