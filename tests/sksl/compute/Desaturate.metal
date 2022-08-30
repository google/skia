#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
};
struct Globals {
    texture2d<half, access::read> src;
    texture2d<half, access::write> dest;
};
half4 desaturate_h4h4(half4 color) {
    color.xyz = half3(dot(color.xyz, half3(0.2199999988079071h, 0.67000001668930054h, 0.10999999940395355h)));
    return color;
}
kernel void computeMain(texture2d<half, access::read> src [[texture(0)]], texture2d<half, access::write> dest [[texture(1)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{src, dest};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition };
    if (_in.sk_ThreadPosition.x < _globals.src.get_width() && _in.sk_ThreadPosition.y < _globals.src.get_height()) {
        _globals.dest.write(desaturate_h4h4(_globals.src.read(_in.sk_ThreadPosition.xy)), _in.sk_ThreadPosition.xy);
    }
    return;
}
