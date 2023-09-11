#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
    uint sk_SampleMask [[sample_mask]];
};
half4 samplemaskin_as_color_h4(uint sk_SampleMaskIn) {
    return half4(half(sk_SampleMaskIn));
}
void clear_samplemask_v(thread Outputs& _out) {
    _out.sk_SampleMask = 0u;
}
void reset_samplemask_v(thread Outputs& _out, uint sk_SampleMaskIn) {
    _out.sk_SampleMask = sk_SampleMaskIn;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]], uint sk_SampleMaskIn [[sample_mask]]) {
    Outputs _out;
    (void)_out;
    clear_samplemask_v(_out);
    reset_samplemask_v(_out, sk_SampleMaskIn);
    _out.sk_SampleMask = 4294967295u;
    _out.sk_FragColor = samplemaskin_as_color_h4(sk_SampleMaskIn) * 0.00390625h;
    return _out;
}
