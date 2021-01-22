
layout (set = 0) uniform vec4 sk_RTAdjust;
void main() {
    gl_Position = vec4(1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
