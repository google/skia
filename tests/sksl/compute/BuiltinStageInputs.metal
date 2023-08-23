#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
    uint3 sk_LocalInvocationID;
    uint sk_LocalInvocationIndex;
    uint3 sk_NumWorkgroups;
    uint3 sk_WorkgroupID;
};
struct outputs {
    uint outputBuffer[1];
};
struct Globals {
    device outputs* _anonInterface0;
};
uint helper_I(Inputs _in) {
    return ((_in.sk_NumWorkgroups.x + _in.sk_WorkgroupID.x) + _in.sk_LocalInvocationID.x) + _in.sk_GlobalInvocationID.x;
}
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], uint3 sk_LocalInvocationID [[thread_position_in_threadgroup]], uint sk_LocalInvocationIndex [[thread_index_in_threadgroup]], uint3 sk_NumWorkgroups [[threadgroups_per_grid]], uint3 sk_WorkgroupID [[threadgroup_position_in_grid]], device outputs& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID, sk_LocalInvocationID, sk_LocalInvocationIndex, sk_NumWorkgroups, sk_WorkgroupID };
    _globals._anonInterface0->outputBuffer[_in.sk_LocalInvocationIndex] = helper_I(_in);
    return;
}
