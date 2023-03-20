#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
kernel void computeMain() {
    Inputs _in = {  };
    threadgroup_barrier(mem_flags::mem_threadgroup);
    threadgroup_barrier(mem_flags::mem_device);
    return;
}
