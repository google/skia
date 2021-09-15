#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int x = 0;
    int y = 0;
    int z = 0;
    x = 1;
    z = 1;
    _out.sk_FragColor.xyz = float3(float(x), float(y), float(z));
    return _out;
}
