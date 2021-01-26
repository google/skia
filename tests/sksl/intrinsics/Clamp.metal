#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
    float c;
    int d;
    int e;
    int f;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};






fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = clamp(_in.a, _in.b, _in.c);
    _out.sk_FragColor.x = float(clamp(_in.d, _in.e, _in.f));
    return _out;
}
