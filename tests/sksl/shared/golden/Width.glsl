
uniform float u_skRTWidth;
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = gl_FragCoord.x / u_skRTWidth;
}
