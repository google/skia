
out vec4 sk_FragColor;
void main() {
    bool sk_Clockwise = gl_FrontFacing;
    sk_FragColor = vec4(float(sk_Clockwise ? 1 : -1));
}
