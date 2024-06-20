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
struct FloatBuffer {
    float floatData[1];
};
struct Globals {
    device FloatBuffer* _anonInterface0;
};
void avoidInline_vf(thread Globals& _globals, thread float& f) {
    f = _globals._anonInterface0->floatData[0];
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], device FloatBuffer& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float _skTemp0;
    float f = 0.0;
    ((avoidInline_vf(_globals, _skTemp0)), (f = _skTemp0));
    _out.sk_FragColor = half4(half(f));
    return _out;
}
