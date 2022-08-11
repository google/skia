#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
};
struct inputBlock {
    uint offset;
    int src[1];
};
struct outputBlock {
    int dest[1];
};
struct Globals {
    const device inputBlock* _anonInterface0;
    device outputBlock* _anonInterface1;
};
kernel void computeMain(const device inputBlock& _anonInterface0 [[buffer(0)]], device outputBlock& _anonInterface1 [[buffer(1)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition };
    _globals._anonInterface1->dest[_in.sk_ThreadPosition.x] = _globals._anonInterface0->src[_in.sk_ThreadPosition.x] + _globals._anonInterface0->src[_in.sk_ThreadPosition.x + _globals._anonInterface0->offset];
    return;
}
