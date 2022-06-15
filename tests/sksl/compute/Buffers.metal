#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device int* src;
};
struct Outputs {
    device int* dest;
};
kernel void computeMain(device int* src, device int* dest, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
Inputs _in = { sk_ThreadPosition, src };
Outputs _out = { dest };
_out.dest[_in.sk_ThreadPosition.x] = _in.src[_in.sk_ThreadPosition.x] * 2;
}
