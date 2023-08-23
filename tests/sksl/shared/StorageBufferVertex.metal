#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize [[point_size]];
};
struct storageBuffer {
    float2 vertices[1];
};
struct Globals {
    const device storageBuffer* _anonInterface0;
};
vertex Outputs vertexMain(Inputs _in [[stage_in]], const device storageBuffer& _anonInterface0 [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_Position = float4(_globals._anonInterface0->vertices[sk_VertexID], 1.0, 1.0);
    return _out;
}
