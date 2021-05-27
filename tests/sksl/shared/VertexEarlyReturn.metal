#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float zoom;
};
struct Inputs {
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize [[point_size]];
};
vertex Outputs vertexMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _out;
    (void)_out;
    _out.sk_Position = float4(1.0);
    if (_uniforms.zoom == 1.0) return _out;
    _out.sk_Position *= _uniforms.zoom;
    return _out;
}
