#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device float* in_data;
};
struct Outputs {
    device float* out_data;
};
struct Threadgroups {
    array<float, 1024> shared_data;
};
void store_vIf(threadgroup Threadgroups& _threadgroups, uint i, float value) {
    _threadgroups.shared_data[i] = value;
}
kernel void computeMain(device float* in_data, device float* out_data, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    threadgroup Threadgroups _threadgroups{{}};
    (void)_threadgroups;
    Inputs _in = { sk_ThreadPosition, in_data };
    Outputs _out = { out_data };
    uint id = _in.sk_ThreadPosition.x;
    uint rd_id;
    uint wr_id;
    uint mask;
    _threadgroups.shared_data[id * 2u] = _in.in_data[id * 2u];
    _threadgroups.shared_data[id * 2u + 1u] = _in.in_data[id * 2u + 1u];
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture);
    const uint steps = 10u;
    for (uint step = 0u;step < steps; step++) {
        mask = (1u << step) - 1u;
        rd_id = ((id >> step) << step + 1u) + mask;
        wr_id = (rd_id + 1u) + (id & mask);
        store_vIf(_threadgroups, wr_id, _threadgroups.shared_data[wr_id] + _threadgroups.shared_data[rd_id]);
        threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup | mem_flags::mem_texture);
    }
    _out.out_data[id * 2u] = _threadgroups.shared_data[id * 2u];
    _out.out_data[id * 2u + 1u] = _threadgroups.shared_data[id * 2u + 1u];
    return;
}
