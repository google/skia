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
    result = bool4(bool2(v.x, bool(1)), true, false);
    result = bool4(bool2(v.y, bool(0)).yx, true, true);
    result = bool4(v.xyz, true);
    result = bool4(bool3(v.xy, bool(1)), true);
    result = bool4(bool3(v.xz, bool(0)).xzy, true);
    result = bool4(bool3(v.x, bool(1), bool(0)), false);
    result = bool4(bool3(v.yz, bool(1)).zxy, false);
    result = bool4(bool3(v.y, bool(0), bool(1)).yxz, false);
    result = bool4(bool2(v.z, bool(1)).yyx, false);
    result = v.xyzw;
    result = bool4(v.xyz, bool(1));
    result = bool4(v.xyw, bool(0)).xywz;
    result = bool4(v.xy, bool(1), bool(0));
    result = bool4(v.xzw, bool(1)).xwyz;
    result = bool4(v.xz, bool(0), bool(1)).xzyw;
    result = bool3(v.xw, bool(1)).xzzy;
    result = bool3(v.x, bool(1), bool(0)).xyzy;
    result = bool4(v.yzw, bool(1)).wxyz;
    result = bool4(v.yz, bool(0), bool(1)).zxyw;
    result = bool4(v.yw, bool(0), bool(1)).zxwy;
    result = bool2(v.y, bool(1)).yxyy;
    result = bool3(v.zw, bool(0)).zzxy;
    result = bool3(v.z, bool(0), bool(1)).yyxz;
    result = bool3(v.w, bool(0), bool(1)).yzzx;
    _out.sk_FragColor = any(result) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
