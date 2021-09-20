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
    switch (int(_uniforms.colorGreen.y)) {
        case 0:
            _out.sk_FragColor = _uniforms.colorRed;
            return _out;
        case 1:
            _out.sk_FragColor = _uniforms.colorGreen;
            return _out;
        default:
            _out.sk_FragColor = _uniforms.colorRed;
            return _out;
    }
    return _out;
}
