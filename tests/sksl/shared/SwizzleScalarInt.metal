#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int i = int(_uniforms.unknownInput);
    int4 i4 = int4(i);
    i4 = int4(int2(i), 0, 1);
    i4 = int4(0, i, 1, 0);
    i4 = int4(0, i, 0, i);
    _out.sk_FragColor = half4(i4);
    return _out;
}
