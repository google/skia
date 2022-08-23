// TODO(b/243079493, b/243078879, b/243077639): Move this test to shared/ once storage buffers are
// supported in other backends
layout(set=0, binding=0) readonly buffer storageBuffer
{
    float2[] vertices;
};

void main() {
    sk_Position = float4(vertices[sk_VertexID], 1.0, 1.0);
}
