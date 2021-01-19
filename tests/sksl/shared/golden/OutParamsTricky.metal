#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _skOut;
    _skOut.sk_FragColor.xz.xy = _skOut.sk_FragColor.zx;

    _skOut.sk_FragColor.yw = float2(3.0, 5.0);

    return _skOut;
}
