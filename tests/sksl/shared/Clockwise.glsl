
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(float((u_skRTFlip.y < 0.0 ? !gl_FrontFacing : gl_FrontFacing) ? 1 : -1));
}
