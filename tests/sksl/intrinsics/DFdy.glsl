
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
uniform float a;
void main() {
    sk_FragColor.x = u_skRTFlip.y * dFdy(a);
}
