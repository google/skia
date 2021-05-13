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
    float3 v2 = float3(3.0) * float3x3(3.0);
    _out.sk_FragColor = float4(float(all((v2 >= float3(8.9999904632568359))) && all((v2 <= float3(9.0000095367431641))) ? 1 : 0), float(all((v2 >= float3(8.9998998641967773))) && all((v2 <= float3(9.0001001358032227))) ? 1 : 0), float(all((v2 >= float3(8.9989995956420898))) && all((v2 <= float3(9.0010004043579102))) ? 1 : 0), float(all((v2 >= float3(8.9899997711181641))) && all((v2 <= float3(9.0100002288818359))) ? 1 : 0));
    return _out;
}
