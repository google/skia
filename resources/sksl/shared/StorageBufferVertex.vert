layout(set=0, binding=0) readonly buffer storageBuffer
{
    float2[] vertices;
};

void main() {
    sk_Position = float4(vertices[sk_VertexID], 1.0, 1.0);
}
