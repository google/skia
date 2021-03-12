
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    sk_FragColor = src * dst.w;
    sk_FragColor = dst * src.w;
}
