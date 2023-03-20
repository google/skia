#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct Globals {
    texture2d<half, access::read_write> tex;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], texture2d<half, access::read_write> tex [[texture(0)]]) {
    Globals _globals{tex};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    if (_in.sk_GlobalInvocationID.x < _globals.tex.get_width() && _in.sk_GlobalInvocationID.y < _globals.tex.get_height()) {
        half4 _0_color = _globals.tex.read(_in.sk_GlobalInvocationID.xy);
        _0_color.xyz = half3(dot(_0_color.xyz, half3(0.22h, 0.67h, 0.11h)));
        _globals.tex.write(_0_color, _in.sk_GlobalInvocationID.xy);
    }
    return;
}
