#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct GlobalCounts {
    atomic_uint firstHalfCount;
    atomic_uint secondHalfCount;
};
struct Inputs {
    uint3 sk_LocalInvocationID;
};
struct ssbo {
    GlobalCounts globalCounts;
};
struct Globals {
    device ssbo* _anonInterface0;
};
struct Threadgroups {
    array<atomic_uint, 2> localCounts;
};
kernel void computeMain(uint3 sk_LocalInvocationID [[thread_position_in_threadgroup]], device ssbo& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}};
    (void)_threadgroups;
    Inputs _in = { sk_LocalInvocationID };
    if (_in.sk_LocalInvocationID.x == 0u) {
        atomic_store_explicit(&_threadgroups.localCounts[0], 0u, memory_order_relaxed);
        atomic_store_explicit(&_threadgroups.localCounts[1], 0u, memory_order_relaxed);
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    uint idx = uint(_in.sk_LocalInvocationID.x < 128u ? 0 : 1);
    atomic_fetch_add_explicit(&_threadgroups.localCounts[idx], 1u, memory_order_relaxed);
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (_in.sk_LocalInvocationID.x == 0u) {
        atomic_fetch_add_explicit(&_globals._anonInterface0->globalCounts.firstHalfCount, atomic_load_explicit(&_threadgroups.localCounts[0], memory_order_relaxed), memory_order_relaxed);
        atomic_fetch_add_explicit(&_globals._anonInterface0->globalCounts.secondHalfCount, atomic_load_explicit(&_threadgroups.localCounts[1], memory_order_relaxed), memory_order_relaxed);
    }
    return;
}
