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
float userfunc_ff(float v) {
    return v + 1.0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float b = 2.0;
    float c = 3.0;
    b = 2.0;
    b = c + 77.0;
    b = sin(c + 77.0);
    userfunc_ff(c + 77.0);
    b = userfunc_ff(c + 77.0);
    b = (b = cos(c));
    for (int x = 0;x < 1; ++x) {
        continue;
    }
    float d = c;
    b = 3.0;
    d++;
    _out.sk_FragColor = half4(half(b == 2.0), half(b == 3.0), half(d == 5.0), half(d == 4.0));
    return _out;
}
