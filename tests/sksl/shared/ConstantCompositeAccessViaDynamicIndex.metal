#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const array<half, 2> globalArray = array<half, 2>{1.0h, 1.0h};
constant const half2 globalVector = half2(1.0h);
constant const half2x2 globalMatrix = half2x2(half2(1.0h, 1.0h), half2(1.0h, 1.0h));
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    int zero;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    const array<half, 2> localArray = array<half, 2>{0.0h, 1.0h};
    const half2 localVector = half2(1.0h);
    const half2x2 localMatrix = half2x2(half2(0.0h, 1.0h), half2(2.0h, 3.0h));
    _out.sk_FragColor = half4(globalArray[_globals.zero] * localArray[_globals.zero], globalVector[_globals.zero] * localVector[_globals.zero], globalMatrix[_globals.zero] * localMatrix[_globals.zero]);
    return _out;
}
