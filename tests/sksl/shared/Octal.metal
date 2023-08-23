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
    int i1 = 1;
    int i2 = 342391;
    int i3 = 2000000000;
    int i4 = -2000000000;
    _out.sk_FragColor = ((i1 == 1 && i2 == 342391) && i3 == 2000000000) && i4 == -2000000000 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
