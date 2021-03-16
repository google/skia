#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 pos  [[attribute(0)]];
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize [[point_size]];
};

vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _out;
    (void)_out;
    _out.sk_Position = _in.pos;
    return (_out.sk_Position.y = -_out.sk_Position.y, _out);
}
