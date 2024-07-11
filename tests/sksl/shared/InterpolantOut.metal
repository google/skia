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
    float defaultVarying [[user(locn0)]];
    float linearVarying [[user(locn1) center_no_perspective]];
    float flatVarying [[user(locn2) flat]];
    float sk_PointSize [[point_size]];
};
vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _out;
    (void)_out;
    _out.defaultVarying = 1.0;
    _out.linearVarying = 2.0;
    _out.flatVarying = 3.0;
    return _out;
}
