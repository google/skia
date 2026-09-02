#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<half, access::read> t;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half, access::read> t [[texture(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{t};
    (void)_globals;
    Outputs _out;
    (void)_out;
    uint2 dims = uint2(_globals.t.get_width(), _globals.t.get_height());
    _out.sk_FragColor = half4(half2(dims), half2(dims));
    return _out;
}
