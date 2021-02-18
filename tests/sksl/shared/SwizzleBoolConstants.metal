#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 v = bool4(bool(_uniforms.colorGreen.y));
    bool4 result;
    result = bool4(v.x, true, true, true);
    result = bool4(v.xy, false, true);
    result = bool4(v.x, true, true, false);
    result = bool4(bool2(false, v.y), true, true);
    result = bool4(v.xyz, true);
    result = bool4(v.xy, true, true);
    result = bool4(bool3(v.xz.x, false, v.xz.y), true);
    result = bool4(v.x, true, false, false);
    result = bool4(bool3(true, v.yz.xy), false);
    result = bool4(bool3(false, v.y, true), false);
    result = bool4(bool3(true, true, v.z), false);
    result = v;
    result = bool4(v.xyz, true);
    result = bool4(v.xyw.xy, false, v.xyw.z);
    result = bool4(v.xy, true, false);
    result = bool4(v.xzw.x, true, v.xzw.yz);
    result = bool4(v.xz.x, false, v.xz.y, true);
    result = bool4(v.xw.x, true, true, v.xw.y);
    result = bool4(v.x, true, false, true);
    result = bool4(true, v.yzw.xyz);
    result = bool4(false, v.yz.xy, true);
    result = bool4(false, v.yw.x, true, v.yw.y);
    result = bool4(true, v.y, true, true);
    result = bool4(false, false, v.zw.xy);
    result = bool4(false, false, v.z, true);
    result = bool4(false, true, true, v.w);
    _out.sk_FragColor = any(result) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
