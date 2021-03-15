#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float a;
    float b;
    float4 c;
    float4 d;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _skTemp0;
    float _skTemp1;
    _out.sk_FragColor.x = (_skTemp0 = _uniforms.a, _skTemp1 = _uniforms.b, _skTemp0 - 2 * _skTemp1 * _skTemp0 * _skTemp1);
    _out.sk_FragColor = reflect(_uniforms.c, _uniforms.d);
    return _out;
}
