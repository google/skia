
uniform float u_skRTHeight;
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = gl_FragCoord.y / u_skRTHeight;
}
