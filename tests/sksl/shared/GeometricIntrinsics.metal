#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float scalar(float x, float y) {
    x = abs(x);
    x = abs(x - y);
    x = (x * y);
    x = sign(x);
    return x;
}
float2 vector(float2 x, float2 y) {
    x = float2(length(x));
    x = float2(distance(x, y));
    x = float2(dot(x, y));
    x = normalize(x);
    return x;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = scalar(1.0, 2.0);
    float2 y = vector(float2(1.0, 2.0), float2(3.0, 4.0));
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
