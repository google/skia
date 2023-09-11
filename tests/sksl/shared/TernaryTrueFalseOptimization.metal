#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    ok = ok && _uniforms.colorGreen.y == 1.0h;
    ok = ok && _uniforms.colorGreen.x != 1.0h;
    ok = ok && all(_uniforms.colorGreen.yx == _uniforms.colorRed.xy);
    ok = ok && all(_uniforms.colorGreen.yx == _uniforms.colorRed.xy);
    ok = ok && (all(_uniforms.colorGreen.yx == _uniforms.colorRed.xy) || _uniforms.colorGreen.w != _uniforms.colorRed.w);
    ok = ok && (any(_uniforms.colorGreen.yx != _uniforms.colorRed.xy) && _uniforms.colorGreen.w == _uniforms.colorRed.w);
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
