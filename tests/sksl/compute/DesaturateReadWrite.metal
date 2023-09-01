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
    texture2d<half, access::read> texIn;
    texture2d<half, access::write> texOut;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], texture2d<half, access::read> texIn [[texture(0)]], texture2d<half, access::write> texOut [[texture(1)]]) {
    Globals _globals{texIn, texOut};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    if (_in.sk_GlobalInvocationID.x < _globals.texIn.get_width() && _in.sk_GlobalInvocationID.y < _globals.texIn.get_height()) {
        half4 _0_color = _globals.texIn.read(_in.sk_GlobalInvocationID.xy);
        _0_color.xyz = half3(dot(_0_color.xyz, half3(0.22h, 0.67h, 0.11h)));
        half4 gray = _0_color;
        _globals.texOut.write(gray, _in.sk_GlobalInvocationID.xy);
    }
    return;
}
