#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
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
kernel void computeMain(constant constants& _anonInterface0 [[buffer(0)]], device outputBuffer& _anonInterface1 [[buffer(1)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition };
    _globals._anonInterface1->results[_in.sk_ThreadPosition.x] *= _globals._anonInterface0->x;
    return;
}
