#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
};
struct inputs {
    float in_data[1];
};
struct outputs {
    float out_data[1];
};
struct Globals {
    const device inputs* _anonInterface0;
    device outputs* _anonInterface1;
};
struct Threadgroups {
    array<float, 1024> shared_data;
};
void store_vIf(threadgroup Threadgroups& _threadgroups, uint i, float value) {
    _threadgroups.shared_data[i] = value;
}
kernel void computeMain(const device inputs& _anonInterface0 [[buffer(0)]], device outputs& _anonInterface1 [[buffer(1)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}};
    (void)_threadgroups;
    Inputs _in = { sk_ThreadPosition };
    uint id = _in.sk_ThreadPosition.x;
    uint rd_id;
    uint wr_id;
    uint mask;
    _threadgroups.shared_data[id * 2u] = _globals._anonInterface0->in_data[id * 2u];
    _threadgroups.shared_data[id * 2u + 1u] = _globals._anonInterface0->in_data[id * 2u + 1u];
    threadgroup_barrier(mem_flags::mem_threadgroup);
    const uint steps = 10u;
    for (uint step = 0u;step < steps; step++) {
        mask = (1u << step) - 1u;
        rd_id = ((id >> step) << step + 1u) + mask;
        wr_id = (rd_id + 1u) + (id & mask);
        store_vIf(_threadgroups, wr_id, _threadgroups.shared_data[wr_id] + _threadgroups.shared_data[rd_id]);
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    _globals._anonInterface1->out_data[id * 2u] = _threadgroups.shared_data[id * 2u];
    _globals._anonInterface1->out_data[id * 2u + 1u] = _threadgroups.shared_data[id * 2u + 1u];
    return;
}
