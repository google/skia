#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 a;
    float4 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _skOut;
    _skOut.sk_FragColor.x = float(packHalf2x16(_in.a));
    _skOut.sk_FragColor.x = float(packUnorm2x16(_in.a));
    _skOut.sk_FragColor.x = float(packSnorm2x16(_in.a));
    _skOut.sk_FragColor.x = float(packUnorm4x8(_in.b));
    _skOut.sk_FragColor.x = float(packSnorm4x8(_in.b));
    return _skOut;
}
