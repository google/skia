#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    array<float, 3> rgb;
    float a;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    S s;
    s.rgb[0] = 0.0;
    s.rgb[1] = 1.0;
    s.rgb[2] = 0.0;
    s.a = 1.0;
    _out.sk_FragColor = float4(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
    return _out;
}
