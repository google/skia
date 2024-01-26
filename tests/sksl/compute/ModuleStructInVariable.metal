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
    IndirectDispatchArgs args = IndirectDispatchArgs{1, 2, 3};
    _threadgroups.outX = args.x;
    _threadgroups.outY = args.y;
    _threadgroups.outZ = args.z;
    return;
}
