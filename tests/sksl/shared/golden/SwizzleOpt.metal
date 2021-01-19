#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float3 colRGB;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float fn(float v) {
    switch (int(v)) {
        case 1:
            return 2.0;
        default:
            return 3.0;
    }
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _skOut;
    float v = sqrt(1.0);
    _skOut.sk_FragColor = float4(v);
    _skOut.sk_FragColor = float4(0.0, float3(v));
    _skOut.sk_FragColor = float4(0.0, 0.0, float2(v));
    _skOut.sk_FragColor = float4(1.0, 1.0, float2(v));
    _skOut.sk_FragColor = float4(float2(v), 1.0, 1.0);
    _skOut.sk_FragColor = float4(v);
    _skOut.sk_FragColor = float4(float2(v), 1.0, 1.0);
    _skOut.sk_FragColor = float4(v);
    _skOut.sk_FragColor = float3(fn(v), 123.0, 456.0).yyzz;
    _skOut.sk_FragColor = float3(fn(v), 123.0, 456.0).yyzz;
    _skOut.sk_FragColor = float4(123.0, 456.0, 456.0, fn(v));
    _skOut.sk_FragColor = float4(123.0, 456.0, 456.0, fn(v));
    _skOut.sk_FragColor = float3(fn(v), 123.0, 456.0).yxxz;
    _skOut.sk_FragColor = float3(fn(v), 123.0, 456.0).yxxz;
    _skOut.sk_FragColor = float4(1.0, 1.0, 2.0, 3.0);
    _skOut.sk_FragColor = float4(_uniforms.colRGB, 1.0);
    _skOut.sk_FragColor = float4(_uniforms.colRGB.x, 1.0, _uniforms.colRGB.yz);
    _skOut.sk_FragColor.xyzw = _skOut.sk_FragColor;
    _skOut.sk_FragColor.wzyx = _skOut.sk_FragColor;
    _skOut.sk_FragColor.xyzw.xw = _skOut.sk_FragColor.yz;
    _skOut.sk_FragColor.wzyx.yzw = float3(_skOut.sk_FragColor.ww, 1.0);
    return _skOut;
}
