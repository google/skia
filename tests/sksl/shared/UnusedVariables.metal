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
    float b = 2.0;
    float c = 3.0;
    for (int x = 0;x < 1; ++x) {
        continue;
    }
    float d = c;
    b++;
    d++;
    _out.sk_FragColor = float4(float(b == 2.0), float(b == 3.0), float(d == 5.0), float(d == 4.0));
    return _out;
}
