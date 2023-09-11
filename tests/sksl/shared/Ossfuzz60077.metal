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
};
void d_vi(int ) {
    int b = 4;
}
void c_vi(int i) {
    d_vi(i);
}
void b_vi(int i) {
    c_vi(i);
}
void a_vi(int i) {
    b_vi(i);
    b_vi(i);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int i;
    a_vi(i);
    _out.sk_FragColor = half4(0.0h);
    return _out;
}
