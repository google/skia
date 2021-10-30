#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 x = _uniforms.colorWhite;
    for (half r = -5.0h;r < 5.0h; r += 1.0h) {
        x.x = saturate(r);
        if (x.x == 0.0h) break;
    }
    for (half b = 5.0h;b >= 0.0h; b -= 1.0h) {
        x.z = b;
        if (x.w == 1.0h) continue;
        x.y = 0.0h;
    }
    _out.sk_FragColor = x;
    return _out;
}
