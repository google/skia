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
    _out.sk_FragColor = ((((((((((((((((((((((((((((((select(intGreen.x, intRed.x, false) == intGreen.x && all(select(intGreen.xy, intRed.xy, bool2(false)) == intGreen.xy)) && all(select(intGreen.xyz, intRed.xyz, bool3(false)) == intGreen.xyz)) && all(select(intGreen, intRed, bool4(false)) == intGreen)) && select(intGreen.x, intRed.x, true) == intRed.x) && all(select(intGreen.xy, intRed.xy, bool2(true)) == intRed.xy)) && all(select(intGreen.xyz, intRed.xyz, bool3(true)) == intRed.xyz)) && all(select(intGreen, intRed, bool4(true)) == intRed)) && 0 == intGreen.x) && all(int2(0, 100) == intGreen.xy)) && all(int3(0, 100, 0) == intGreen.xyz)) && all(int4(0, 100, 0, 100) == intGreen)) && 100 == intRed.x) && all(int2(100, 0) == intRed.xy)) && all(int3(100, 0, 0) == intRed.xyz)) && all(int4(100, 0, 0, 100) == intRed)) && select(_uniforms.colorGreen.x, _uniforms.colorRed.x, false) == _uniforms.colorGreen.x) && all(select(_uniforms.colorGreen.xy, _uniforms.colorRed.xy, bool2(false)) == _uniforms.colorGreen.xy)) && all(select(_uniforms.colorGreen.xyz, _uniforms.colorRed.xyz, bool3(false)) == _uniforms.colorGreen.xyz)) && all(select(_uniforms.colorGreen, _uniforms.colorRed, bool4(false)) == _uniforms.colorGreen)) && select(_uniforms.colorGreen.x, _uniforms.colorRed.x, true) == _uniforms.colorRed.x) && all(select(_uniforms.colorGreen.xy, _uniforms.colorRed.xy, bool2(true)) == _uniforms.colorRed.xy)) && all(select(_uniforms.colorGreen.xyz, _uniforms.colorRed.xyz, bool3(true)) == _uniforms.colorRed.xyz)) && all(select(_uniforms.colorGreen, _uniforms.colorRed, bool4(true)) == _uniforms.colorRed)) && 0.0 == _uniforms.colorGreen.x) && all(float2(0.0, 1.0) == _uniforms.colorGreen.xy)) && all(float3(0.0, 1.0, 0.0) == _uniforms.colorGreen.xyz)) && all(float4(0.0, 1.0, 0.0, 1.0) == _uniforms.colorGreen)) && 1.0 == _uniforms.colorRed.x) && all(float2(1.0, 0.0) == _uniforms.colorRed.xy)) && all(float3(1.0, 0.0, 0.0) == _uniforms.colorRed.xyz)) && all(float4(1.0, 0.0, 0.0, 1.0) == _uniforms.colorRed) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
