
out vec4 sk_FragColor;
void main() {
    sk_FragColor.xz.xy = sk_FragColor.zx;

    sk_FragColor.yw = vec2(3.0, 5.0);

}
