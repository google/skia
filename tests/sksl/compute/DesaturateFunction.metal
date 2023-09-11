#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct Globals {
    texture2d<half, access::read> src;
    texture2d<half, access::write> dest;
};
void desaturate_vTT(Inputs _in, texture2d<half, access::read> src, texture2d<half, access::write> dest) {
    half4 color = src.read(_in.sk_GlobalInvocationID.xy);
    color.xyz = half3(dot(color.xyz, half3(0.22h, 0.67h, 0.11h)));
    dest.write(color, _in.sk_GlobalInvocationID.xy);
}
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], texture2d<half, access::read> src [[texture(0)]], texture2d<half, access::write> dest [[texture(1)]]) {
    Globals _globals{src, dest};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    if (_in.sk_GlobalInvocationID.x < _globals.src.get_width() && _in.sk_GlobalInvocationID.y < _globals.src.get_height()) {
        desaturate_vTT(_in, _globals.src, _globals.dest);
    }
    return;
}
