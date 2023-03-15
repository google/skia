
layout (binding = 0, set = 0) readonly buffer storageBuffer {
    vec2[] vertices;
};
void main() {
    gl_Position = vec4(vertices[gl_VertexID], 1.0, 1.0);
}
