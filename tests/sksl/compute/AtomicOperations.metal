#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_LocalInvocationID;
};
struct ssbo {
    atomic_uint globalCounter;
};
struct Globals {
    device ssbo* _anonInterface0;
};
struct Threadgroups {
    atomic_uint localCounter;
};
kernel void computeMain(uint3 sk_LocalInvocationID [[thread_position_in_threadgroup]], device ssbo& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}};
    (void)_threadgroups;
    Inputs _in = { sk_LocalInvocationID };
    if (_in.sk_LocalInvocationID.x == 0u) {
        atomic_store_explicit(&_threadgroups.localCounter, 0u, memory_order_relaxed);
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    atomic_fetch_add_explicit(&_threadgroups.localCounter, 1u, memory_order_relaxed);
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (_in.sk_LocalInvocationID.x == 0u) {
        atomic_fetch_add_explicit(&_globals._anonInterface0->globalCounter, atomic_load_explicit(&_threadgroups.localCounter, memory_order_relaxed), memory_order_relaxed);
    }
    return;
}
