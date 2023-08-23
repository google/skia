#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(float2 v1, float2 v2, float2 v3, float3 v4, int2 v5, int2 v6, float2 v7, float2 v8, float4 v9, int2 v10, bool4 v11, float2 v12, float2 v13, float2 v14, bool2 v15, bool2 v16, bool3 v17, int4 v18) {
    return ((((((((((((((((half(v1.x) + half(v2.x)) + half(v3.x)) + half(v4.x)) + half(v5.x)) + half(v6.x)) + half(v7.x)) + half(v8.x)) + half(v9.x)) + half(v10.x)) + half(v11.x)) + half(v12.x)) + half(v13.x)) + half(v14.x)) + half(v15.x)) + half(v16.x)) + half(v17.x)) + half(v18.x) == 18.0h;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2 v1 = float2(1.0);
    float2 v2 = float2(1.0, 2.0);
    float2 v3 = float2(1.0);
    float3 v4 = float3(1.0);
    int2 v5 = int2(1);
    int2 v6 = int2(1, 2);
    float2 v7 = float2(1.0, 2.0);
    float2 v8 = float2(v5);
    float4 v9 = float4(float(v6.x), _uniforms.unknownInput, 3.0, 4.0);
    int2 v10 = int2(3, int(v1.x));
    bool4 v11 = bool4(true, false, true, false);
    float2 v12 = float2(1.0, 0.0);
    float2 v13 = float2(0.0);
    float2 v14 = float2(0.0);
    bool2 v15 = bool2(true);
    bool2 v16 = bool2(true);
    bool3 v17 = bool3(true);
    int4 v18 = int4(1);
    _out.sk_FragColor = check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
