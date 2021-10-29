#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half a;
    half b;
    half c;
    half4 d;
    half4 e;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = (refract(float2(_uniforms.a, 0), float2(_uniforms.b, 0), _uniforms.c).x);
    _out.sk_FragColor = refract(_uniforms.d, _uniforms.e, _uniforms.c);
    _out.sk_FragColor.xy = half2(0.5h, -0.86602538824081421h);
    _out.sk_FragColor.xyz = half3(0.5h, 0.0h, -0.86602538824081421h);
    _out.sk_FragColor = half4(0.5h, 0.0h, 0.0h, -0.86602538824081421h);
    return _out;
}
