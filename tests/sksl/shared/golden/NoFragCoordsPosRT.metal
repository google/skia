#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 sk_RTAdjust;
};
struct Inputs {
    float4 pos;
};
struct Outputs {
    float4 sk_Position [[position]];
    float sk_PointSize [[point_size]];
};


vertex Outputs vertexMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _skOut;
    _skOut.sk_Position = _in.pos;
    _skOut.sk_Position = float4(_skOut.sk_Position.xy * _uniforms.sk_RTAdjust.xz + _skOut.sk_Position.ww * _uniforms.sk_RTAdjust.yw, 0.0, _skOut.sk_Position.w);
    return (_skOut.sk_Position.y = -_skOut.sk_Position.y, _skOut);
}
