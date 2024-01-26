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
int one_i() {
    return 1;
}
int two_i() {
    return 2;
}
int three_i() {
    return 3;
}
kernel void computeMain() {
    threadgroup Threadgroups _threadgroups{{}, {}, {}};
    (void)_threadgroups;
    Inputs _in = {  };
    _threadgroups.outX = IndirectDispatchArgs{one_i(), two_i(), three_i()}.x;
    _threadgroups.outY = IndirectDispatchArgs{one_i(), two_i(), three_i()}.y;
    _threadgroups.outZ = IndirectDispatchArgs{one_i(), two_i(), three_i()}.z;
    return;
}
