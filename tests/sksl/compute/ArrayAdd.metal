#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device int* src1;
    device int* src2;
};
struct Outputs {
    device int* dest;
};
kernel void computeMain(device int* src1, device int* src2, device int* dest, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, src1, src2 };
    Outputs _out = { dest };
    _out.dest[_in.sk_ThreadPosition.x] = _in.src1[_in.sk_ThreadPosition.x] + _in.src2[_in.sk_ThreadPosition.x];
    return;
}
