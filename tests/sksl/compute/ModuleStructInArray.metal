#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct IndirectDispatchArgs {
    int x;
    int y;
    int z;
};
struct Inputs {
};
struct Threadgroups {
    int outX;
    int outY;
    int outZ;
};
kernel void computeMain() {
    threadgroup Threadgroups _threadgroups{{}, {}, {}};
    (void)_threadgroups;
    Inputs _in = {  };
    array<IndirectDispatchArgs, 3> args;
    _threadgroups.outX = args[0].x;
    _threadgroups.outY = args[1].y;
    _threadgroups.outZ = args[2].z;
    return;
}
