#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    array<half, 3> rgb;
    half a;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    S s;
    s.rgb[0] = 0.0h;
    s.rgb[1] = 1.0h;
    s.rgb[2] = 0.0h;
    s.a = 1.0h;
    _out.sk_FragColor = half4(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
    return _out;
}
