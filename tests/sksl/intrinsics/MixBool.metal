#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float4 colorBlack;
    float4 colorWhite;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};





fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int4 intGreen = int4(_uniforms.colorGreen * 100.0);
    int4 intRed = int4(_uniforms.colorRed * 100.0);
    _out.sk_FragColor = ((((((((((((((mix(intGreen.x, intRed.x, false) == intGreen.x && all(mix(intGreen.xy, intRed.xy, bool2(false)) == intGreen.xy)) && all(mix(intGreen.xyz, intRed.xyz, bool3(false)) == intGreen.xyz)) && all(mix(intGreen.xyzw, intRed.xyzw, bool4(false)) == intGreen.xyzw)) && mix(intGreen.x, intRed.x, true) == intRed.x) && all(mix(intGreen.xy, intRed.xy, bool2(true)) == intRed.xy)) && all(mix(intGreen.xyz, intRed.xyz, bool3(true)) == intRed.xyz)) && all(mix(intGreen.xyzw, intRed.xyzw, bool4(true)) == intRed.xyzw)) && mix(_uniforms.colorGreen.x, _uniforms.colorRed.x, false) == _uniforms.colorGreen.x) && all(mix(_uniforms.colorGreen.xy, _uniforms.colorRed.xy, bool2(false)) == _uniforms.colorGreen.xy)) && all(mix(_uniforms.colorGreen.xyz, _uniforms.colorRed.xyz, bool3(false)) == _uniforms.colorGreen.xyz)) && all(mix(_uniforms.colorGreen.xyzw, _uniforms.colorRed.xyzw, bool4(false)) == _uniforms.colorGreen.xyzw)) && mix(_uniforms.colorGreen.x, _uniforms.colorRed.x, true) == _uniforms.colorRed.x) && all(mix(_uniforms.colorGreen.xy, _uniforms.colorRed.xy, bool2(true)) == _uniforms.colorRed.xy)) && all(mix(_uniforms.colorGreen.xyz, _uniforms.colorRed.xyz, bool3(true)) == _uniforms.colorRed.xyz)) && all(mix(_uniforms.colorGreen.xyzw, _uniforms.colorRed.xyzw, bool4(true)) == _uniforms.colorRed.xyzw) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
