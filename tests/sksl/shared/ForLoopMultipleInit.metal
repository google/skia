#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 result = float4(0.0);
    for (float a = 0.0;
    float b = 0.0;
    a < 10.0 && b < 10.0; (++a , ++b)) {
        result.x = result.x + a;
        result.y = result.y + b;
    }
    for (int c = 0;
    c < 10; ++c) {
        result.z = result.z + 1.0;
    }
    for (array<float, 2> d = array<float, 2>{0.0, 10.0};
    array<float, 4> e = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    float f = 9.0;
    d[0] < d[1]; ++d[0]) {
        result.w = e[0] * f;
    }
    for (; ; ) break;
    _out.sk_FragColor = result;
    return _out;
}
