#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool b = bool(_uniforms.unknownInput);
    bool4 b4 = bool4(b);
    b4 = bool4(bool2(b), false, true);
    b4 = bool4(false, b, true, false);
    b4 = bool4(false, b, false, b);
    _out.sk_FragColor = float4(b4);
    return _out;
}
