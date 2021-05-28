#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    int unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float value;
    switch (_uniforms.unknownInput) {
        case 0:
            value = 0.0;
        case 1:
            value = 1.0;
        default:
            value = 2.0;
    }
    _out.sk_FragColor = float4(value);
    return _out;
}
