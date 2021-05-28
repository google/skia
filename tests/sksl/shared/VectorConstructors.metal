#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(float2 v1, float2 v2, float2 v3, float3 v4, int2 v5, int2 v6, float2 v7, float2 v8, float4 v9, int2 v10, bool4 v11, float2 v12, float2 v13, float2 v14, bool2 v15, bool2 v16, bool3 v17) {
    return (((((((((((((((v1.x + v2.x) + v3.x) + v4.x) + float(v5.x)) + float(v6.x)) + v7.x) + v8.x) + v9.x) + float(v10.x)) + float(v11.x)) + v12.x) + v13.x) + v14.x) + float(v15.x)) + float(v16.x)) + float(v17.x) == 17.0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2 v1 = float2(1.0);
    float2 v2 = float2(1.0, 2.0);
    float2 v3 = float2(1.0);
    float3 v4 = float3(float2(1.0), 1.0);
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
    bool3 v17 = bool3(true, bool2(true));
    _out.sk_FragColor = check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
