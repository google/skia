#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
    float4 c;
    float4 d;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = atan(_in.a);
    _out.sk_FragColor.x = atan2(_in.a, _in.b);
    _out.sk_FragColor = atan(_in.c);
    _out.sk_FragColor = atan2(_in.c, _in.d);
    return _out;
}
