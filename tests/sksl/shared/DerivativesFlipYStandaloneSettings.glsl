
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
void main() {
    (sk_FragColor.x = dFdx(1.0) , sk_FragColor.y = (u_skRTFlip.y * dFdy)(1.0));
}
