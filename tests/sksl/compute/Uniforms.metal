#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct constants {
    int x;
};
struct outputBuffer {
    int results[1];
};
struct Globals {
    constant constants* _anonInterface0;
    device outputBuffer* _anonInterface1;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], constant constants& _anonInterface0 [[buffer(0)]], device outputBuffer& _anonInterface1 [[buffer(1)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    _globals._anonInterface1->results[_in.sk_GlobalInvocationID.x] *= _globals._anonInterface0->x;
    return;
}
