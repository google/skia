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
    int id [[user(locn1)]];
    float sk_PointSize [[point_size]];
};
vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _out;
    (void)_out;
    _out.id = sk_InstanceID;
    return _out;
}
