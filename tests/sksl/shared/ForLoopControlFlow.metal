#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 x = _uniforms.colorWhite;
    for (float r = -5.0;r < 5.0; r += 1.0) {
        x.x = saturate(r);
        if (x.x == 0.0) break;
    }
    for (float b = 5.0;b >= 0.0; b -= 1.0) {
        x.z = b;
        if (x.w == 1.0) continue;
        x.y = 0.0;
    }
    _out.sk_FragColor = x;
    return _out;
}
