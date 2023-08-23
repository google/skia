#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    atomic_uint structMemberAtomic;
    array<atomic_uint, 2> structMemberAtomicArray;
};
struct NestedS {
    S nestedStructWithAtomicMember;
};
struct Inputs {
};
struct ssbo {
    atomic_uint ssboAtomic;
    array<atomic_uint, 2> ssboAtomicArray;
    S ssboStructWithAtomicMember;
    array<S, 2> ssboStructWithAtomicMemberArray;
    NestedS ssboNestedStructWithAtomicMember;
};
struct Globals {
    device ssbo* _anonInterface0;
};
struct Threadgroups {
    atomic_uint wgAtomic;
    array<atomic_uint, 2> wgAtomicArray;
    NestedS wgNestedStructWithAtomicMember;
};
kernel void computeMain(device ssbo& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}, {}, {}};
    (void)_threadgroups;
    Inputs _in = {  };
    atomic_fetch_add_explicit(&_threadgroups.wgAtomicArray[1], atomic_load_explicit(&_threadgroups.wgAtomic, memory_order_relaxed), memory_order_relaxed);
    atomic_fetch_add_explicit(&_threadgroups.wgAtomicArray[0], atomic_load_explicit(&_threadgroups.wgAtomicArray[1], memory_order_relaxed), memory_order_relaxed);
    atomic_fetch_add_explicit(&_threadgroups.wgNestedStructWithAtomicMember.nestedStructWithAtomicMember.structMemberAtomic, 1u, memory_order_relaxed);
    return;
}
