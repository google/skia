#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct StorageBuffer {
    array<float, 16> storage_data;
};
struct UniformBuffer {
    array<float, 16> uniform_data;
};
struct Globals {
    device StorageBuffer* _anonInterface0;
    constant UniformBuffer* _anonInterface1;
};
struct Threadgroups {
    array<float, 16> scratch;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], device StorageBuffer& _anonInterface0 [[buffer(0)]], constant UniformBuffer& _anonInterface1 [[buffer(1)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}};
    (void)_threadgroups;
    Inputs _in = { sk_GlobalInvocationID };
    uint id = _in.sk_GlobalInvocationID.x;
    _threadgroups.scratch[id] = _globals._anonInterface1->uniform_data[id];
    threadgroup_barrier(mem_flags::mem_threadgroup);
    _globals._anonInterface0->storage_data[id] = _threadgroups.scratch[id];
    return;
}
