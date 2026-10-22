#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
    uint sk_SubgroupID;
    uint sk_SubgroupInvocationID;
    uint sk_SubgroupSize;
};
struct outputs {
    uint outputBuffer[1];
};
struct Globals {
    device outputs* _anonInterface0;
};
uint helper_I(Inputs _in) {
    return _in.sk_SubgroupSize + _in.sk_SubgroupID;
}
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], uint sk_SubgroupID [[simdgroup_index_in_threadgroup]], uint sk_SubgroupInvocationID [[thread_index_in_simdgroup]], uint sk_SubgroupSize [[threads_per_simdgroup]], device outputs& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID, sk_SubgroupID, sk_SubgroupInvocationID, sk_SubgroupSize };
    _globals._anonInterface0->outputBuffer[_in.sk_GlobalInvocationID.x] = helper_I(_in) + _in.sk_SubgroupInvocationID;
    return;
}
