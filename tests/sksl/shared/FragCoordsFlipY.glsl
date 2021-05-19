#version 400
out vec4 sk_FragColor;
vec4 sk_RTAdjust;
void main() {
    sk_FragColor.xy = vec2(gl_FragCoord.x, 2.0 * sk_RTAdjust.w - gl_FragCoord.y);
}
