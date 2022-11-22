#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
kernel void computeMain(uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = {  };
    threadgroup_barrier(mem_flags::mem_threadgroup);
    threadgroup_barrier(mem_flags::mem_device);
    return;
}
