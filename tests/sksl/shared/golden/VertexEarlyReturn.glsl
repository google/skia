
layout (set = 0) uniform float zoom;
void main() {
    gl_Position = vec4(1.0);
    if (zoom == 1.0) return;
    gl_Position *= zoom;
}
