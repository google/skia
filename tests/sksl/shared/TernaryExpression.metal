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
    int check = 0;
    check += int(_uniforms.colorGreen.y == 1.0h ? 0 : 1);
    check += int(_uniforms.colorGreen.x == 1.0h);
    check += int(all(_uniforms.colorGreen.yx == _uniforms.colorRed.xy) ? 0 : 1);
    check += int(any(_uniforms.colorGreen.yx != _uniforms.colorRed.xy));
    _out.sk_FragColor = check == 0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
