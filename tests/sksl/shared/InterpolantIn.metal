#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    float defaultVarying [[user(locn0)]];
    float linearVarying [[user(locn1) center_no_perspective]];
    float flatVarying [[user(locn2) flat]];
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(half(_in.defaultVarying), half(_in.linearVarying), half(_in.flatVarying), 1.0h);
    return _out;
}
