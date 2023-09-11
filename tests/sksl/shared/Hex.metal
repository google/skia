#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int i1 = 0;
    i1++;
    int i2 = 4660;
    i2++;
    int i3 = 32766;
    i3++;
    int i4 = -32766;
    i4++;
    int i5 = 19132;
    i5++;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
