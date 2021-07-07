
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
void main() {
    bool sk_Clockwise = gl_FrontFacing;
    if (u_skRTFlip.y < 0.0) {
        sk_Clockwise = !sk_Clockwise;
    }
    sk_FragColor = vec4(float(sk_Clockwise ? 1 : -1));
}
